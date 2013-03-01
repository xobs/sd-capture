#include <QFile>
#include "networkconnection.h"
#include "packet.h"

#define REREAD_OVERLAP 80

NetworkConnection::NetworkConnection(QObject *parent) :
	QObject(parent),
	dataSocket(this),
	currentCommand(-1),
	currentSubCommand(0),
	currentSyncPoint(0),
	ignoreBuffer(0),
	goodPackets(0),
	skipPackets(0),
	bufferDraining(false),
	commandRunning(false),
	didSkipPackets(false)
{
}

bool NetworkConnection::connectToHost(QString &hostname, quint16 dataPort)
{
	dataSocket.connectToHost(hostname, dataPort);
	connect(&dataSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));
	return true;
}

bool NetworkConnection::setLogfile(QString &filename)
{
	if (logFile.isOpen())
		logFile.close();

	logFile.setFileName(filename);
	return logFile.open(QIODevice::WriteOnly);
}

void NetworkConnection::runScript(QList<NetCommand> &script)
{
	networkScript = script;

	currentCommand = -1;
	currentSubCommand = -1;
	currentSyncPoint = 0;

	sendNextCommand();
}

void NetworkConnection::receiveData()
{
	while(dataSocket.bytesAvailable()) {
		Packet p(dataSocket, this);
		processDataPacket(p);
		p.write(logFile);
	}
}

void NetworkConnection::processDataPacket(Packet &packet)
{
	qDebug() << "Got data packet:" << packet;

	// If the buffer starts draining, hold off on new commands
	if (packet.packetType() == PACKET_BUFFER_DRAIN) {
		if (packet.bufferDrainEvent() == PKT_BUFFER_DRAIN_START) {
			bufferDraining = true;
			packetsDrained = 0;
		}
		else if (packet.bufferDrainEvent() == PKT_BUFFER_DRAIN_STOP) {
			bufferDraining = false;
			qDebug() << "Drained" << packetsDrained << "packets out of NAND,"
					 << goodPackets << "total packets drained so far";
		}
	}

	// If a command is running, also hold off on new packets
	if (packet.packetType() == PACKET_COMMAND) {
		if (packet.commandState() == CMD_START)
			commandRunning = true;
		else if (packet.commandState() == CMD_STOP)
			commandRunning = false;
	}

	// If we get a command out of the NAND cycle, increase the number of good packets
	if (packet.packetType() == PACKET_NAND_CYCLE) {
		packetsDrained++;
		goodPackets++;
	}

	// If we hit an overflow, re-read the last 100 packets
	if (packet.packetType() == PACKET_ERROR
	 && packet.errorSubsystem() == SUBSYS_FPGA
	 && packet.errorCode() == FPGA_ERR_OVERFLOW) {
		goodPackets -= REREAD_OVERLAP;
		skipPackets = goodPackets;
	}

	// If the previous command just finished, send the next command
	if (!bufferDraining && !commandRunning) {

		// If skipPackets is true, then we encountered an overflow at some
		// point during the last command.
		// Run the "ib" command to ignore the first few bytes, then queue
		// up to re-run the command.
		// Otherwise,
		if (skipPackets) {
			QString command;
			command.sprintf("ib %d\n", skipPackets);
			ignoreBuffer = skipPackets;
			// Replay the last command
			skipPackets = 0;
			didSkipPackets = true;
			runCommand(command);
		}

		// If we just skipped a packet, re-run the last command.
		else if (didSkipPackets) {
			didSkipPackets = false;
			currentCommand = currentSyncPoint-1;
			sendNextCommand();
		}

		// Otherwise, run the next command.
		else {
			sendNextCommand();
		}
	}

	emit gotPacket(packet);
}

void NetworkConnection::sendNextCommand()
{
	currentSubCommand++;
	if (currentCommand < 0 || currentSubCommand >= networkScript.at(currentCommand).commands().length()) {
		currentCommand++;
		currentSubCommand = 0;
	}

	// Finish running commands, if we've hit the end
	if (currentCommand >= networkScript.length()) {
		emit scriptComplete();
		return;
	}

	// If we have an explicit command starting with "ib", start a new
	// sync point
	QString command = networkScript.at(currentCommand).commands().at(currentSubCommand) + "\n";
	if (networkScript.at(currentCommand).commands().at(currentSubCommand).startsWith(QString("ib"))) {
		currentSyncPoint = currentCommand+1;
		skipPackets = 0;
		goodPackets = 0;
	}

	runCommand(command);
}

void NetworkConnection::runCommand(QString &cmd)
{
	emit sendingCommand(cmd, currentCommand);
	dataSocket.write(cmd.toUtf8());
}

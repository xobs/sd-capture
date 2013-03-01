#include <QFile>
#include <QDialog>
#include <QApplication>
#include <QErrorMessage>
#include "networkconnection.h"
#include "packet.h"
#include "mainwindow.h"

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
	connect(&dataSocket, SIGNAL(connected()),
			(MainWindow *)parent, SLOT(networkConnected()));
	connect(&dataSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			(MainWindow *)parent, SLOT(networkConnectionError(QAbstractSocket::SocketError)));
}

bool NetworkConnection::connectToHost(QString &hostname, quint16 dataPort)
{
	dataSocket.connectToHost(hostname, dataPort);
	connect(&dataSocket, SIGNAL(readyRead()), this, SLOT(receiveData()));
	return true;
}

bool NetworkConnection::setLogfile(QString &filename)
{
	bool result;
	if (logFile.isOpen())
		logFile.close();

	logFile.setFileName(filename);
	result = logFile.open(QIODevice::WriteOnly);

	// If we were unable to open the logfile, display an error
	if (!result) {
		QErrorMessage errorMessage(NULL);
		errorMessage.showMessage(QString("Unable to open logfile: %1").arg(logFile.errorString()));
		errorMessage.exec();
		qApp->quit();
		return false;
	}

	// We opened the file, so drain the packet buffer
	for (int i=0; i<packetBuffer.count(); i++) {
		((Packet)packetBuffer.at(i)).write(logFile);
	}
	packetBuffer.clear();

	return result;
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
		if (logFile.isOpen())
			p.write(logFile);
		else
			packetBuffer.append(p);
	}
}

void NetworkConnection::processDataPacket(Packet &packet)
{
	// If the buffer starts draining, hold off on new commands
	if (packet.packetType() == PACKET_BUFFER_DRAIN) {
		if (packet.bufferDrainEvent() == PKT_BUFFER_DRAIN_START) {
			bufferDraining = true;
			packetsDrained = 0;
		}
		else if (packet.bufferDrainEvent() == PKT_BUFFER_DRAIN_STOP) {
			bufferDraining = false;
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

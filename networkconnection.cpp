#include <QFile>
#include "networkconnection.h"
#include "packet.h"

NetworkConnection::NetworkConnection(QObject *parent) :
	QObject(parent),
	dataSocket(this),
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

void NetworkConnection::runScript(QStringList &script)
{
	qDebug() << "Running script: " << script;
	networkScript = script;

	currentCommand = -1;
	sendNextCommand();
}

void NetworkConnection::receiveData()
{
	while(dataSocket.bytesAvailable()) {
		Packet p(dataSocket, this);
		processDataPacket(p);
	}
	/*
	QByteArray byteArray(dataSocket.bytesAvailable(), 0);
	int currentOffset;
	int dataSize;
	const char *data;

	dataSocket.read(byteArray.data(), byteArray.size());
	if (-1 == logFile.write(byteArray))
		qDebug() << "Unable to log data";


	currentOffset = 0;
	data = byteArray.data();
	dataSize = byteArray.size();

	int packetsInFrame = 0;
	qDebug() << "Frame was" << dataSize << "bytes large";
	while (dataSize > 0) {
		QByteArray subPacket(data+currentOffset, dataSize);
		Packet p(subPacket, this);
		qDebug() << "Packet was " << p.packetSize() << "bytes";
		processDataPacket(p);

		currentOffset += p.packetSize();
		dataSize -= p.packetSize();
		packetsInFrame++;
		qDebug() << packetsInFrame << " packets in this frame," << dataSize << "bytes left";
		if (dataSize > 0 && dataSize < sizeof(pkt_header)) {
			qWarning() << "Fewer bytes left than pkt_header size!" << dataSize << "vs" << sizeof(pkt_header);
			dataSize = 0;
		}
	}
	qDebug() << "Finished reading frame";
	*/

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
			qDebug() << "Drained" << packetsDrained << "packets out of NAND";
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
		skipPackets = goodPackets - 100;
	}

	// If the previous command just finished, send the next command
	if (!bufferDraining && !commandRunning) {
		if (skipPackets) {
			QString command;
			command.sprintf("ib %d\n", skipPackets);
			currentCommand--;
			runCommand(command);
			skipPackets = 0;
			didSkipPackets = true;
		}
		else {
			if (didSkipPackets)
				didSkipPackets = false;
			else
				goodPackets = 0;
			sendNextCommand();
		}
	}

	emit gotPacket(packet);
}

void NetworkConnection::sendNextCommand()
{
	currentCommand++;
	// Finish running commands, if we've hit the end
	if (currentCommand >= networkScript.length())
		return;
	QString command = networkScript[currentCommand] + "\n";
	runCommand(command);
}

void NetworkConnection::runCommand(QString &cmd)
{
	emit sendingCommand(cmd);
	dataSocket.write(cmd.toUtf8());
}

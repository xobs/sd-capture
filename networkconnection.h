#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QObject>
#include <QFile>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QStringList>

class Packet;

class NetworkConnection : public QObject
{
	Q_OBJECT
public:
	explicit NetworkConnection(QObject *parent = 0);
	bool connectToHost(QString &hostname, quint16 dataPort = 17283);
	bool setLogfile(QString &filename);

private:
	QFile logFile;
	QTcpSocket dataSocket;
	QStringList networkScript;
	int currentCommand;
	int goodPackets;
	int skipPackets; // Used to re-run commands
	bool bufferDraining;
	bool commandRunning;
	bool didSkipPackets;
	int packetsDrained;
	void sendNextCommand();
	void processDataPacket(Packet &p);

signals:
	void gotPacket(Packet &packet);
	void sendingCommand(QString &command);

public slots:
	void receiveData();
	void runScript(QStringList &script);
	void runCommand(QString &cmd);
};

#endif // NETWORKCONNECTION_H

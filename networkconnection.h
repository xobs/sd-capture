#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H

#include <QObject>
#include <QFile>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QStringList>
#include "netcommand.h"
#include "packet.h"

class Packet;

class NetworkConnection : public QObject
{
	Q_OBJECT
public:
	explicit NetworkConnection(QObject *parent = 0);
	bool connectToHost(QString &hostname, quint16 dataPort = 17283);
	bool setLogfile(QString &filename);
	QString socketErrorString() const;

private:
	QFile logFile;
	QTcpSocket dataSocket;
	QList<NetCommand> networkScript;

	int currentCommand;
	int currentSubCommand;
	int currentSyncPoint;
	bool haveRunScript; // True if we've ever run a script in this session

	int ignoreBuffer;
	int goodPackets;
	int skipPackets; // Used to re-run commands
	bool bufferDraining;
	bool commandRunning;
	bool didSkipPackets;
	int packetsDrained;
	void sendNextCommand();
	void processDataPacket(Packet &p);

	QByteArray networkBuffer;
	QList<Packet> packetBuffer; // Buffer packets while the logfile is opened

signals:
	void gotPacket(Packet &packet);
	void sendingCommand(const QString &command, int index);
	void scriptComplete();

public slots:
	void receiveData();
	void runScript(QList<NetCommand> &script);
	void runCommand(QString &cmd);
};

#endif // NETWORKCONNECTION_H

#ifndef PACKET_H
#define PACKET_H

#include <QObject>
#include <QIODevice>
#include <QDateTime>
#include <stdint.h>
#include <QFile>

#include "packet-struct.h"

#define MAX_BUFFER_SIZE 4096

QList<QString> &PacketTypes();

class Packet : public QObject
{
    Q_OBJECT
public:
    explicit Packet(QObject *parent = 0);
	Packet(QIODevice &source, QObject *parent = 0);
	Packet(QByteArray &data, QObject *parent = 0);

    Packet() {}
    Packet(const Packet &other, QObject *parent = 0);
    Packet &operator=(const Packet &other);
    bool operator<(const Packet &other) const;
	void decodePacket();

	/* Used for writing data out */
	qint64 write(QIODevice &device);

	uint32_t nanoSeconds() const;
	time_t seconds() const;
	uint32_t packetSize() const;

	enum pkt_type packetType() const;
	const QString &packetTypeStr() const;
	int index();
	int setIndex(int index);

    /* NAND Command/Data functions */
	uint8_t nandControl() const;
	uint8_t nandData() const;
	uint16_t nandUnknown() const;

    /* Error functions */
	const QString &errorStr() const;
	uint32_t errorCode() const;
	uint32_t errorArgument() const;
	uint32_t errorSubsystem() const;

    /* SD Command or Data functions */
	uint8_t sdRegister() const;
	uint8_t sdValue() const;

	/* SD response */
	uint8_t sdResponse() const;

	/* Command state */
	uint8_t commandState() const;
	const QString &commandName() const;
	uint32_t commandArg() const;

	/* Hello state */
	uint8_t helloVersion() const;

	/* Buffer drain started/stopped */
	uint8_t bufferDrainEvent() const;

	/* Reset event */
	uint8_t resetVersion() const;

	/* Get CSD */
	const QString &csd() const;

	/* Get CID */
	const QString &cid() const;

private:
    struct pkt packet;
	int packetIndex;
	QString errorString;
	QString commandNameString;
	QString cardCSD;
	QString cardCID;

signals:
    
public slots:
    
};

QDebug operator<<(QDebug dbg, const Packet &p);

#endif // PACKET_H

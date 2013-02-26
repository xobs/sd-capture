#include <arpa/inet.h>
#include <QDebug>
#include <QTextStream>
#include "packet.h"

static QList<QString> packetTypes;

enum control_pins {
    NAND_ALE = 2,
    NAND_CLE = 1,
    NAND_WE = 4,
    NAND_RE = 8,
    NAND_CS = 16,
    NAND_RB = 32,
};

static uint8_t order[] = {
    4,  // Known
    5,  // Known
    6,  // Known
    7,  // Known

    3,  // Known
    2,  // Known
    1,  // Known
    0,  // Known
};

static uint8_t nand_unscramble_byte(uint8_t byte) {
    return (
          ( (!!(byte&(1<<order[0]))) << 0)
        | ( (!!(byte&(1<<order[1]))) << 1)
        | ( (!!(byte&(1<<order[2]))) << 2)
        | ( (!!(byte&(1<<order[3]))) << 3)
        | ( (!!(byte&(1<<order[4]))) << 4)
        | ( (!!(byte&(1<<order[5]))) << 5)
        | ( (!!(byte&(1<<order[6]))) << 6)
        | ( (!!(byte&(1<<order[7]))) << 7)
    );
}

QList<QString> &PacketTypes()
{
    if (packetTypes.length() <= 0) {
        packetTypes.append("PACKET_UNKNOWN");
        packetTypes.append("PACKET_ERROR");
        packetTypes.append("PACKET_NAND_CYCLE");
        packetTypes.append("PACKET_SD_DATA");
        packetTypes.append("PACKET_SD_CMD_ARG");
        packetTypes.append("PACKET_SD_RESPONSE");
        packetTypes.append("PACKET_SD_CID");
        packetTypes.append("PACKET_SD_CSD");
        packetTypes.append("PACKET_BUFFER_OFFSET");
        packetTypes.append("PACKET_BUFFER_CONTENTS");
        packetTypes.append("PACKET_COMMAND");
        packetTypes.append("PACKET_RESET");
		packetTypes.append("PACKET_BUFFER_DRAIN");
		packetTypes.append("PACKET_HELLO");
	}
    return packetTypes;
}

static qint64 streamReadData(QIODevice &source, char *data, qint64 bytesTotal) {
	qint64 bytesRead = 0;
	qint64 bytesLeft = bytesTotal;
	int result;

	while (bytesLeft > 0) {
		result = source.read(data, bytesLeft);
		if (result < 0) {
			qDebug() << "Source had error:" << source.errorString();
			return result;
		}

		if (result == 0) {
			if (!source.waitForReadyRead(-1)) {
				qDebug() << "Source is done, closing";
				break;
			}
		}

		if (result > bytesTotal) {
			qDebug() << "Very bad.  Wanted " << bytesTotal << ", but got " << result;
		}
		bytesRead += result;
		data += result;
		if (bytesLeft - result < 0) {
			qDebug() << "That was a really strange read";
		}
		bytesLeft -= result;
	}
	if (bytesLeft < 0) {
		qDebug() << "How could bytesLeft (" << bytesLeft << ") be less than zero?";
	}
	return bytesRead;
}


Packet::Packet(QObject *parent) :
    QObject(parent)
{
	memset(&packet, 0, sizeof(packet));
}

Packet::Packet(QIODevice &source, QObject *parent) :
	QObject(parent)
{
	qint64 bytesRead;

	memset(&packet, 0, sizeof(packet));
	bytesRead = streamReadData(source, (char *)(&packet.header), sizeof(packet.header));
	if (bytesRead != sizeof(packet.header)) {
		qDebug() << "Read an unexpected number of bytes:" << bytesRead << "vs" << sizeof(packet.header);
	}
    packet.header.nsec = ntohl(packet.header.nsec);
    packet.header.sec = ntohl(packet.header.sec);
    packet.header.size = ntohs(packet.header.size);
	if (packet.header.size > sizeof(packet)) {
		qDebug() << "Header size is VERY wrong:" << packet.header.size;
	}

	bytesRead = streamReadData(source, (char *)(&packet.data), packet.header.size - sizeof(packet.header));
	decodePacket();
}

Packet::Packet(QByteArray &data, QObject *parent) :
	QObject(parent)
{
	memset(&packet, 0, sizeof(packet));
	size_t size = data.length();
	if (size > sizeof(packet))
		size = sizeof(packet);
	memcpy(&packet, data, size);
	packet.header.nsec = ntohl(packet.header.nsec);
	packet.header.sec = ntohl(packet.header.sec);
	packet.header.size = ntohs(packet.header.size);
	decodePacket();
}

Packet::Packet(const Packet &other, QObject *parent) :
	QObject(parent)
{
    memcpy(&packet, &other.packet, sizeof(packet));
	decodePacket();
	setIndex(other.packetIndex);
}

Packet &Packet::operator=(const Packet &other)
{
    memcpy(&packet, &other.packet, sizeof(packet));
	decodePacket();
    return *this;
}

bool Packet::operator<(const Packet &other) const
{
	if (seconds() < other.seconds())
        return true;
	if (seconds() > other.seconds())
        return false;
	if (seconds() == other.seconds() && nanoSeconds() < other.nanoSeconds())
        return true;
    return false;
}

void Packet::decodePacket() {
	if (packetType() == PACKET_ERROR) {
		char message[sizeof(packet.data.error.message)+1];
		memset(message, 0, sizeof(message));
		strncpy(message, (const char *)packet.data.error.message, sizeof(message));
		errorString = message;
		errorString.remove("\n");
	}
	else
		errorString = "";

	if (packetType() == PACKET_COMMAND) {
		char cmd[sizeof(packet.data.command.cmd)+1];
		memset(cmd, 0, sizeof(cmd));
		memcpy(cmd, packet.data.command.cmd, sizeof(packet.data.command.cmd));
		commandNameString = cmd;
		commandNameString.remove("\n");
	}
	else
		commandNameString = "";

	if (packetType() == PACKET_SD_CID) {
		cardCID.sprintf("%02x %02x %02x %02x %02x %02x %02x %02x",
						packet.data.cid.cid[0],
						packet.data.cid.cid[1],
						packet.data.cid.cid[2],
						packet.data.cid.cid[3],
						packet.data.cid.cid[4],
						packet.data.cid.cid[5],
						packet.data.cid.cid[6],
						packet.data.cid.cid[7]
						);
	}
	else
		cardCID = "";

	if (packetType() == PACKET_SD_CSD) {
		cardCSD.sprintf("%02x %02x %02x %02x %02x %02x %02x %02x",
						packet.data.csd.csd[0],
						packet.data.csd.csd[1],
						packet.data.csd.csd[2],
						packet.data.csd.csd[3],
						packet.data.csd.csd[4],
						packet.data.csd.csd[5],
						packet.data.csd.csd[6],
						packet.data.csd.csd[7]
						);
	}
	else
		cardCSD = "";

}

uint32_t Packet::nanoSeconds() const {
    return packet.header.nsec;
}

time_t Packet::seconds() const {
    return packet.header.sec;
}

uint32_t Packet::packetSize() const {
	return packet.header.size;
}

enum pkt_type Packet::packetType() const {
    return (enum pkt_type)packet.header.type;
}

const QString &Packet::packetTypeStr() const {
	if (packet.header.type > PacketTypes().count())
		return PacketTypes()[0];
    return PacketTypes()[packet.header.type];
}

int Packet::setIndex(int index)
{
    int o = index;
	this->packetIndex = index;
    return o;
}

int Packet::index()
{
	return packetIndex;
}

uint8_t Packet::nandControl() const {
	if (packetType() != PACKET_NAND_CYCLE)
        return 0;
	return packet.data.nand_cycle.control;
}

uint8_t Packet::nandData() const {
	if (packetType() != PACKET_NAND_CYCLE)
        return 0;
	return nand_unscramble_byte(packet.data.nand_cycle.data);
}
uint16_t Packet::nandUnknown() const {
	if (packetType() != PACKET_NAND_CYCLE)
		return 0;
	return ntohs(packet.data.nand_cycle.unknown);
}

const QString &Packet::errorStr() const {
	return errorString;
}

uint32_t Packet::errorCode() const {
	if (packetType() != PACKET_ERROR)
		return 0;
	return packet.data.error.code;
}

uint32_t Packet::errorArgument() const {
	if (packetType() != PACKET_ERROR)
		return 0;
	return ntohs(packet.data.error.arg);
}

uint32_t Packet::errorSubsystem() const {
	if (packetType() != PACKET_ERROR)
		return 0;
	return packet.data.error.subsystem;
}

uint8_t Packet::sdRegister() const {
	if (packetType() != PACKET_SD_CMD_ARG)
        return 0;
    return packet.data.sd_cmd_arg.reg;
}

uint8_t Packet::sdValue() const {
	if (packetType() != PACKET_SD_CMD_ARG)
        return 0;
    return packet.data.sd_cmd_arg.val;
}

uint8_t Packet::commandState() const {
	if (packetType() != PACKET_COMMAND)
		return 0;
	return packet.data.command.start_stop;
}

uint32_t Packet::commandArg() const {
	if (packetType() != PACKET_COMMAND)
		return 0;
	return ntohl(packet.data.command.arg);
}

const QString &Packet::commandName() const {
	return commandNameString;
}

uint8_t Packet::helloVersion() const {
	if (packetType() != PACKET_HELLO)
		return 0;
	return packet.data.hello.version;
}

uint8_t Packet::sdResponse() const {
	if (packetType() != PACKET_SD_RESPONSE)
		return 0;
	return packet.data.response.byte;
}

uint8_t Packet::bufferDrainEvent() const {
	if (packetType() != PACKET_BUFFER_DRAIN)
		return 0;
	return packet.data.buffer_drain.start_stop;
}

uint8_t Packet::resetVersion() const {
	if (packetType() != PACKET_RESET)
		return 0;
	return packet.data.reset.version;
}

const QString &Packet::csd() const {
	return cardCSD;
}

const QString &Packet::cid() const {
	return cardCID;
}

QDebug operator<<(QDebug dbg, const Packet &p)
{
	QString timestamp;
	QString message;
	timestamp.sprintf("<%d.%09d> - ", (int)p.seconds(), (int)p.nanoSeconds());
	message = p.packetTypeStr() + " " + timestamp;

	switch(p.packetType()) {
		case PACKET_HELLO:
			QTextStream(&message) << "Hello (v" << p.helloVersion() << ")";
			break;

		case PACKET_SD_CMD_ARG:
			if (p.sdRegister() == 0) {
				QTextStream(&message) << "CMD" << (p.sdValue()&0x3f);
			}
			else {
				QTextStream(&message) << "R" << p.sdRegister() <<"=" << p.sdValue();
			}
			break;

		case PACKET_COMMAND:
			if (p.commandState() == CMD_START)
				QTextStream(&message) << "Start " << p.commandName() << "(" << p.commandArg() << ")";
			else if (p.commandState() == CMD_STOP)
				QTextStream(&message) << "Finish " << p.commandName() << "(" << p.commandArg() << ")";
			else
				QTextStream(&message) << "Unknown state for " << p.commandName() << "(" << p.commandArg() << ")";
			break;

		case PACKET_SD_RESPONSE:
			QTextStream(&message) << "First byte of card response: " << QString::number(p.sdResponse(), 16);
			break;

		case PACKET_NAND_CYCLE: {
                QString ctrlString;
                uint8_t ctrl = p.nandControl();
                QTextStream(&message)
                    << QString("%1").arg(p.nandData(), 2, 16, QChar('0'))
                    << " " << (ctrl&NAND_ALE?"A":" ")
                    << " " << (ctrl&NAND_CLE?"C":" ")
                    << " " << (ctrl&NAND_WE?"W":" ")
                    << " " << (ctrl&NAND_RE?"R":" ")
                    << " " << (ctrl&NAND_CS?"S":" ")
                    << " " << (ctrl&NAND_RB?"B":" ")
                    << " (" << QString("%1").arg(p.nandUnknown(), 4, 16, QChar('0')) << ")";
            }
			break;

		case PACKET_SD_DATA:
			QTextStream(&message) << "512 bytes of data";
			break;

		case PACKET_ERROR:
			QTextStream(&message) << "Subsystem " << p.errorSubsystem() << ", Code " << p.errorCode() << ", Arg " << p.errorArgument() << ", Message: " << p.errorStr();
			break;

		case PACKET_RESET:
			QTextStream(&message) << "Reset (version v" << p.resetVersion() << ")";
			break;

		case PACKET_BUFFER_DRAIN:
			if (p.bufferDrainEvent() == PKT_BUFFER_DRAIN_START)
				QTextStream(&message) << "Buffer started draining";
			else if (p.bufferDrainEvent() == PKT_BUFFER_DRAIN_STOP)
				QTextStream(&message) << "Buffer finished draining";
			else
				QTextStream(&message) << "Buffer drain did something unknown: " << p.bufferDrainEvent();
			break;

		case PACKET_SD_CID:
			QTextStream(&message) << "Card CID: " << p.cid();
			break;

		case PACKET_SD_CSD:
			QTextStream(&message) << "Card CSD: " << p.csd();
			break;

		case PACKET_UNKNOWN:
		default:
			QTextStream(&message) << "Unknown packet";
			break;
	}

	dbg.nospace() << message;
	return dbg.space();
}

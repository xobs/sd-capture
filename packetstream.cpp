#include <QDebug>
#include "packetstream.h"


static uint8_t transform(uint8_t op) {
    return (op>>4&0x0f) | (op<<4&0xf0);
}

static int handle_nand(Packet &nand) {
	if (nand.packetType() != PACKET_NAND_CYCLE)
        return -1;

    /* Bits [0..4] are ALE, CLE, WE, RE, and CS (in order) */
	int ALE=!!((nand.nandControl())&1);
	int CLE=!!((nand.nandControl())&2);
	int WE=!!((nand.nandControl())&4);
	int RE=!!((nand.nandControl())&8);
	int CE=!!((nand.nandControl())&16);
    const char *operation, *type;

    if (CLE && !ALE) {
        type = "Cmd";
    }
    else if (!CLE && ALE) {
        type = "Address";
    }
    else if (!CLE && !ALE) {
        type = "Input";
        //type = NULL;
    }
    else {
        type = "Unknown";
    }

    if (!RE)
        operation = "Read";
    else
        operation = "Write";


    if (type && operation) {
		qDebug("%25s @ %3ld.%08d  @ %02x  - %s %s", (const char *)nand.packetTypeStr().toUtf8(), nand.seconds(), nand.nanoSeconds(), transform(nand.nandData()), operation, type);
    }
    return 0;
}

static int handle_error(Packet &error) {
	if (error.packetType() != PACKET_ERROR)
        return -1;
	qDebug("%25s @ %3ld.%08d  - %08x %s", (const char *)error.packetTypeStr().toUtf8(), error.seconds(), error.nanoSeconds(),
		   error.errorCode(), (const char *)error.errorStr().toUtf8());
    return 0;
}

static int handle_sd_cmd_arg(Packet &pkt) {
	if (pkt.sdRegister() == 0) {
		qDebug("%25s @ %3ld.%08d  - CMD%d", (const char *)pkt.packetTypeStr().toUtf8(), pkt.seconds(), pkt.nanoSeconds(), pkt.sdValue()&0x3f);
    }
    else {
		qDebug("%25s @ %3ld.%08d  - R%d %02x", (const char *)pkt.packetTypeStr().toUtf8(), pkt.seconds(), pkt.nanoSeconds(), pkt.sdRegister(), pkt.sdValue());
    }
    return 0;
}

static int handle_other(Packet &pkt) {
	qDebug("%25s @ %3ld.%08d", (const char *)pkt.packetTypeStr().toUtf8(), pkt.seconds(), pkt.nanoSeconds());
    return 0;
}

PacketStream::PacketStream(QObject *parent) :
    QObject(parent)
{
}

int PacketStream::LoadFromFile(QFile &source)
{
    bool gotReset = false;
    int index = 0;
    long last_sec=-1, last_nsec = -1;
    int duplicate_packets=0, total_packets=0;
    packets.clear();
    while (!source.atEnd()) {
        Packet next = Packet(source);
        total_packets++;
		if (last_sec == next.seconds() && last_nsec == next.nanoSeconds()) {
            duplicate_packets++;
            continue;
        }
		last_sec = next.seconds();
		last_nsec = next.nanoSeconds();
        /*
        if (!gotReset && next.PacketType() != PACKET_RESET) {
            qDebug("Throwing away a packet %s before reset with time %ld", (const char *)next.PacketTypeStr().toAscii(), next.Seconds());
            continue;
        }
        */
		if (next.seconds() > 10000) {
			qDebug("Throwing away a packet with time %ld", next.seconds());
            continue;
        }
        gotReset = true;

		next.setIndex(index++);
        packets.append(next);
    }

    qSort(packets.begin(), packets.end());

    for (int i=0; i<packets.count(); i++) {
        Packet &next = packets[i];
        if (i>0 && !(packets[i-1] < packets[i]))
            qDebug("Sorting algorithm messed up");
		if (next.packetType() == PACKET_NAND_CYCLE)
            handle_nand(next);
		else if(next.packetType() == PACKET_ERROR)
            handle_error(next);
		else if (next.packetType() == PACKET_SD_CMD_ARG)
            handle_sd_cmd_arg(next);
        else
            handle_other(next);
    }

    qDebug("Got %d packets in total with %d (%d%%) duplicates...", packets.count(), duplicate_packets, 100*duplicate_packets/total_packets);
    return 0;
}

const QList<Packet> &PacketStream::Packets()
{
    return packets;
}

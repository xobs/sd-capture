#ifndef PACKETSTREAM_H
#define PACKETSTREAM_H

#include <QObject>
#include <QFile>
#include "packet.h"

class PacketStream : public QObject
{
    Q_OBJECT
public:
    explicit PacketStream(QObject *parent = 0);
    int LoadFromFile(QFile &source);
    const QList<Packet> &Packets();

private:
    QList<Packet> packets;

signals:
    
public slots:
    
};

#endif // PACKETSTREAM_H

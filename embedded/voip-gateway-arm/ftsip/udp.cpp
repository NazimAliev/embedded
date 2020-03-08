/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "udp.h"
#include "console.h"

Udp::Udp(QString _localAddr, int _localPort, QString _remoteAddr, int _remotePort, QObject *parent) :
    QObject(parent)
{
    localAddr = _localAddr;
    localPort = _localPort;
    remoteAddr = _remoteAddr;
    remotePort = _remotePort;
    //qDebug() << "Udp::Udp" << localAddr << localPort << remoteAddr << remotePort;

    udpSocket = new QUdpSocket(this);
    udpLocalHost.setAddress(localAddr);
    udpRemoteHost.setAddress(remoteAddr);
    bool ok = udpSocket->bind(udpLocalHost,localPort);
    if(!ok)
    {
        log(QString("bind failed addr: [%1:%2]").arg(localAddr).arg(localPort), LOG_ERR);
    }
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(slotReadDatagram()));
}

void Udp::slotReadDatagram()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress host;
        quint16 port;
        datagram.resize(udpSocket->pendingDatagramSize());

        // store remote ip and port for using in outgoing udp
        // attention! this values remove setup default value - shoud be the same
        udpSocket->readDatagram(datagram.data(), datagram.size(), &host, &port);

        if(host.toString() != udpRemoteHost.toString() || port != remotePort)
            log(QString("remote udp host [%1:%2] doesn't match expected [%3:%4]").
                arg(host.toString()).arg(port).arg(udpRemoteHost.toString()).arg(remotePort), LOG_WARN);

        QString str(datagram);
        str.remove('\n');
        str.remove('\r');
        Q_EMIT(signalHasDatagram(str));
    }
}

void Udp::writeDatagram(QString data)
{
    QByteArray array = data.toUtf8();
    udpSocket->writeDatagram(array, udpRemoteHost, remotePort);
    //qDebug() << "==WD" << udpRemoteHost.toString() << remotePort << udpLocalHost.toString() << localPort << data;
}

void Udp::writeDatagram(QByteArray data)
{
    udpSocket->writeDatagram(data, udpRemoteHost, remotePort);
}

Udp::~Udp()
{
    delete udpSocket;
}



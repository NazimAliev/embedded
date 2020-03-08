/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef UDP_H
#define UDP_H

#include <QObject>
#include <QUdpSocket>
#include <QStringList>

class Udp : public QObject
{
    Q_OBJECT
public:
    explicit Udp(QString _localAddr, int _localPort, QString _remoteAddr, int _remotePort, QObject *parent = 0);

    QString localAddr;
    QString remoteAddr;
    int localPort;
    int remotePort;

    ~Udp();
    void writeDatagram(QString data);
    void writeDatagram(QByteArray data);

signals:
    void signalHasDatagram(QString data);

private slots:
    // for internal use only
    void slotReadDatagram();

private:
    QUdpSocket* udpSocket;
    QHostAddress udpLocalHost;
    QHostAddress udpRemoteHost;
};

#endif // UDP_H

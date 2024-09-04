/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

void Dialog::initUdp()
{
    // init for udp - interconnect with SIP
    udpLocalSocketSip = new QUdpSocket(this);

    QHostAddress local_ip;
    local_ip.setAddress(ip_atom_board);
    Q_ASSERT(udpLocalSocketSip->bind(local_ip, port_control_sip_board_src));
    connect(udpLocalSocketSip, SIGNAL(readyRead()), this, SLOT(slotReadDatagram()));

    // init for outcoming SIP udp
    host_sip_board.setAddress(ip_sip_board);
    writeDatagramDebugSip("#fthmi starts\n");  // necessary to fill up udp addr info
}

// read data from ftsip, parse string into t_isip format and call sipEvent(t_isip isip)
// input string format: "id;isip_message;peer;parm"
void Dialog::slotReadDatagram()
{
    while (udpLocalSocketSip->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpLocalSocketSip->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpLocalSocketSip->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString str(datagram);

        if(datagram[0] == '#')
        {
            // debug message from ftsip
            str.remove('\n');
            qDebug() << "ftsip>" << str;
            return;
        }
        // parse string into t_isip structure
        str.remove('\n');
        str.remove('\r');
        e_sip sip = static_cast<e_sip>(str.section(';', 0, 0).toInt());
        QString peer = str.section(';', 1, 1);
        QString header = str.section(';', 2, 2);
        QString body = str.section(';', 3, 3);
        QString parm = str.section(';', 4, 4);
        QString text = str.section(';', 5, 5);
        log(QString("<< %1;%2;%3;%4;%5;%6").arg(sip).arg(peer).arg(header).arg(body).arg(parm).arg(text), LOG_UDP);
        eventChain(peer, E_NULL, sip, header, body, parm);
    }
}

// convert data from t_osip to string with delimiter ';' and send to ftsip
void Dialog::writeDatagramSip(e_sip sip, QString peer, QString header, QString parm, QString text)
{
    QString str = QString("%1;%2;%3;%4;%5\n").arg(sip).arg(peer).arg(header).arg(parm).arg(text);
    QByteArray data = str.toUtf8();
    udpLocalSocketSip->writeDatagram(data, host_sip_board, port_control_sip_board_dst);
    log(QString("HMI#%1 >> ").arg(cwp) += str.remove('\n'), LOG_UDP);
}

void Dialog::writeDatagramDebugSip(QString str)
{
    str += '\n';
    QByteArray data = str.toUtf8();
    udpLocalSocketSip->writeDatagram(data, host_sip_board, port_control_sip_board_dst);
}


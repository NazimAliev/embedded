/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QSqlQuery>
#include <QObject>
#include <QSocketNotifier>
#include <QTextStream>
#include <QFile>
#include <QUdpSocket>
#include <QDebug>

#include <stdio.h>
#include "csengine.h"
#include "udp.h"

#define C_R QByteArray("\033[0;31m")
#define C_G QByteArray("\033[0;32m")
#define C_Y QByteArray("\033[0;33m")
#define C_B QByteArray("\033[0;34m")
#define C_M QByteArray("\033[0;35m")
#define C_C QByteArray("\033[0;36m")
#define C_H QByteArray("\033[0;37m")
#define C_ QByteArray("\033[0m")

// log staff - log type (for different colors and permit to show) and log function

typedef enum
{
    LOG_SIP,
    LOG_ON,
    LOG_UDP,
    LOG_INFO,
    LOG_WARN,
    LOG_ERR,
    LOG_OTHER
} e_log;

void log(QString msg, e_log log);

class Console : public QObject
{
Q_OBJECT
public:
    explicit Console(QObject *parent = 0);
    ~Console();
    int initConfig(QString squery);
    void udpInit();
    int cwp;

signals:

public slots:
    void slotReadAtomDatagram();
    void slotReadDspDatagram();
    void slotWriteAtomDatagram(e_sip sip, QString peer, QString header, QString body, QString parm, QString text);
    void slotWriteDspDatagram(QString cmd, QString local_ip, int local_port, QString remote_ip, int remote_port,
                              QString mask, bool sw, QString parm, QString text);

private:
    e_sip focus_event;
    e_disp focus_disp;
    QString focus_peer;
    QString focus_header;
    QString focus_body;
    QString focus_parm;
    QString focus_text;
    QString referto;
    QString replaces;
    QString parseField(QStringList &list, QString t);
    void parseAtomCmd(e_sip sip, QString peer, QString header, QString parm);
    void parseDspMsg(QString peer, QString data);
    void confServer(e_sip sip, QString peer, QString header, QString body, QString parm, QString);
    void confCam(e_sip sip, QString peer, QString header, QString body, QString parm, QString);
    bool SIP_FOCUS(e_disp disp_check, e_sip event_check, e_disp disp_new, QString text);
    bool HMI_FOCUS(e_disp disp_check, e_sip hmi, e_disp disp_new, QString text);

protected:
    Csengine csengine;
    uint console_id;
    QUdpSocket* udpLocalSocketAtom;
    QUdpSocket* udpLocalSocketDsp;
    // remote parms will be stored after uncoming udp
    QHostAddress host_atom_board;
    QHostAddress host_dsp_board;

    QString uri;
    QString ip_atom_board;  // only for copy to ip_sip_board if ip_sip_board is empty in DB (debug mode)
    QString ip_sip_board;
    QString ip_dsp_board;
    QString ip_sip_listen;
    QString ip_sip_contact;
    quint16 port_sip_listen;
    quint16 port_control_sip_board_src;
    quint16 port_control_dsp_board_src;
    quint16 port_control_sip_board_dst;
    quint16 port_control_dsp_board_dst;
    quint16 port_audio;
    QString conf_uri;
    bool isRx;
    bool isTx;
    bool isOn;
    bool isCoupled;
    QString snMethod;
    QString freq;
    QString rmode;
    QString type;

    void writeAtomDatagramDebug(QString data);
    void writeDspDatagramDebug(QString data);
};


#endif // CONSOLE_H

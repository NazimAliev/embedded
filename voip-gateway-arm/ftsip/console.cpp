/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "console.h"
#define PREFIX

Console::Console(QObject *parent) :
    QObject(parent)
{
    console_id = 0;
    focus_disp = D_OFF;
}

Console::~Console()
{
}

// header is only for calls from ftsip focus to put "isfocus" header
void Console::parseAtomCmd(e_sip sip, QString peer, QString header, QString parm)
{
    // parse command from fthmi
    switch(sip)
    {
    // initiate call from hmi
    case EC_CALL:
        csengine.sendInvite(peer, STYPE_PLAIN, header, parm);
        break;
        // initiate radio call from hmi
    case EC_CALL_RADIO:
        csengine.sendInvite(peer, STYPE_RADIO, header, parm);
        break;
        // ringing
    case EC_RINGING:
        csengine.sendRinging(peer);
        break;
        // hmi answer for call from peer by click
    case EC_200_OK:
        csengine.sendAnswer(peer, STYPE_PLAIN, header, parm);
        break;
    case EC_SUBSCRIBE:
        csengine.sendSubscribe(peer, header);
        break;
    case EC_UNSUBSCRIBE:
        csengine.sendUnsubscribe(peer);
        break;
    case EC_REFER:
        csengine.sendRefer(peer, header, parm);
        break;
    case EC_TERM_IN:
        csengine.sendTerminate(peer, false);
        break;
    case EC_TERM_OUT:
        csengine.sendTerminate(peer, true);
        break;
        // hmi answer for call from peer by click
    default:
        log(QString("undefined UDP command=%1").arg(sip), LOG_ERR);
        break;
    }

}

void Console::parseDspMsg(QString peer, QString data)
{
    // msg from VoIP module: radio parms
    log(QString("parseDspMsg peer=[%1] data=[%2]").arg(peer).arg(data), LOG_INFO);
}

QString Console::parseField(QStringList &list, QString t)
{
    // find matched template string in the list
    // parsed string without prefix, remove parsed data from list
    int i;
    QString res;
    QRegExp rx(QString("%1*").arg(t));
    //qDebug() << list << t;
    rx.setPatternSyntax(QRegExp::Wildcard);

    i = list.indexOf(rx);
    if(i == -1)
    {
        log(QString("Console::parseField(): template [%1] not found").arg(t), LOG_WARN);
        return "-1";
    }
    res = list.takeAt(i).remove(t);
    return res;
}


int Console::initConfig(QString squery)
{
    QSqlQuery query;
    bool ok;
    ok = query.prepare(squery);
    Q_ASSERT(ok);
    ok = query.exec();
    Q_ASSERT(ok);
    ok = query.first();
    Q_ASSERT(ok);

    if(GW)
    {
        uri = query.value(RADIO_URI).toString();
        ip_sip_board = query.value(RADIO_IP_SIP_BOARD).toString();
        ip_dsp_board = query.value(RADIO_IP_DSP_BOARD).toString();
        ip_sip_listen = query.value(RADIO_IP_SIP_LISTEN).toString();
        if(ip_sip_listen == "")
            ip_sip_listen = ip_sip_board;
        ip_sip_contact = query.value(RADIO_IP_SIP_CONTACT).toString();
        if(ip_sip_contact == "")
            ip_sip_contact = ip_sip_board;
        port_sip_listen = query.value(RADIO_PORT_SIP_LISTEN).toUInt();
        port_control_dsp_board_src = query.value(RADIO_PORT_CONTROL_DSP_BOARD_SRC).toUInt();
        port_control_dsp_board_dst = query.value(RADIO_PORT_CONTROL_DSP_BOARD_DST).toUInt();
        port_audio = query.value(RADIO_PORT_AUDIO).toUInt();
        isRx = query.value(RADIO_ISRX).toInt();
        isTx = query.value(RADIO_ISTX).toInt();
        isOn = query.value(RADIO_ISON).toInt();
        isCoupled = query.value(RADIO_ISCOUPLED).toInt();
        snMethod = query.value(RADIO_SN_METHOD).toString();

        // !!! wrong - need select from freq table
        freq = query.value(FREQ_FREQ).toString();

        if(!isRx && !isTx)
            rmode = "inactive";
        else if(isRx && !isTx)
            rmode = "recvonly";
        else if(!isRx && isTx)
            rmode = "sendonly";
        else if(isRx && isTx)
            rmode = "sendrecv";
        if(isCoupled)
            type = "coupling";
        else
            type = "radio";
        // ToDo: class var to store sendonly, recvonly... local caps
        log(QString("GW local caps: mode:%1 type:%2 bss:%3 fid:%4").arg(rmode).arg(type).arg(snMethod).arg(freq), LOG_OTHER);
    }
    else
    {
        uri = query.value(BOOK_URI).toString();
        ip_sip_board = query.value(BOOK_IP_SIP_BOARD).toString();
        ip_atom_board = query.value(BOOK_IP_ATOM_BOARD).toString();
        if(ip_sip_board == "")
            ip_sip_board = ip_atom_board;
        ip_dsp_board = query.value(BOOK_IP_DSP_BOARD).toString();
        ip_sip_listen = query.value(BOOK_IP_SIP_LISTEN).toString();
        if(ip_sip_listen == "")
            ip_sip_listen = ip_sip_board;
        ip_sip_contact = query.value(BOOK_IP_SIP_CONTACT).toString();
        if(ip_sip_contact == "")
            ip_sip_contact = ip_sip_board;
        port_sip_listen = query.value(BOOK_PORT_SIP_LISTEN).toUInt();
        port_control_sip_board_src = query.value(BOOK_PORT_CONTROL_SIP_BOARD_SRC).toUInt();
        port_control_dsp_board_src = query.value(BOOK_PORT_CONTROL_DSP_BOARD_SRC).toUInt();
        port_control_sip_board_dst = query.value(BOOK_PORT_CONTROL_SIP_BOARD_DST).toUInt();
        port_control_dsp_board_dst = query.value(BOOK_PORT_CONTROL_DSP_BOARD_DST).toUInt();
        port_audio = query.value(BOOK_PORT_AUDIO).toUInt();
        //conf_uri = QString("conf@%1:%2").arg(ip_sip_board).arg(port_sip_listen);
        // ToDo: change it - select from DB
        //conf_uri = "conf1@192.168.54.1:5060";

        // get conf focus for this cwp
        int ref_conf = query.value(BOOK_REF_CONF).toUInt();
        ok = query.prepare("SELECT * FROM book WHERE key=?");
        Q_ASSERT(ok);
        query.bindValue(0, ref_conf);
        ok = query.exec();
        Q_ASSERT(ok);
        ok = query.first();
        Q_ASSERT(ok);
        conf_uri = query.value(BOOK_URI).toString();
    }

    {
        log(QString("uri=%1").arg(uri), LOG_OTHER);
        if(!GW)
            log(QString("ip_atom_board=%1").arg(ip_atom_board), LOG_OTHER);
        log(QString("ip_sip_board=%1").arg(ip_sip_board), LOG_OTHER);
        log(QString("ip_dsp_board=%1").arg(ip_dsp_board), LOG_OTHER);
        log(QString("ip_sip_listen=%1").arg(ip_sip_listen), LOG_OTHER);
        log(QString("ip_sip_contact=%1").arg(ip_sip_contact), LOG_OTHER);
        log(QString("port_sip_listen=%1").arg(port_sip_listen), LOG_OTHER);
        if(!GW)
            log(QString("port_control_sip_board_src=%1").arg(port_control_sip_board_src), LOG_OTHER);
        log(QString("port_control_dsp_board_src=%1").arg(port_control_dsp_board_src), LOG_OTHER);
        if(!GW)
            log(QString("port_control_sip_board_dst=%1").arg(port_control_sip_board_dst), LOG_OTHER);
        log(QString("port_control_dsp_board_dst=%1").arg(port_control_dsp_board_dst), LOG_OTHER);
        log(QString("port_audio=%1").arg(port_audio), LOG_OTHER);
        if(!GW)
            log(QString("focus=%1").arg(conf_uri), LOG_OTHER);
    }

    int res = csengine.initConfig(ip_sip_listen, ip_sip_contact, port_sip_listen, uri, ip_dsp_board, port_audio, conf_uri,
                                  rmode, type, snMethod, freq);
    if(res != 0)
        return 1;

    /*
    m_q_in.open(stdin, QIODevice::ReadOnly);
    m_stream_in = new QTextStream(&m_q_in);
    m_q_out.open(stdout, QIODevice::WriteOnly);
    m_stream_out = new QTextStream(&m_q_out);
    // Demand notification when there is data to be read from stdin
    notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read );
    connect(notifier, SIGNAL(activated(int)), this, SLOT(readStdin(int)));

    *m_stream_out << QString("ftsip> ");
    m_stream_out->flush();

    printf("CS Control Terminal %s\n", rcsid);
    printf("Usage:\n");
    printf("(n)ew [user@]peer[:port] new outgoing SIP session, returns session ID\n");
    printf("r(i)nging       peer\n");
    printf("(a)nswer        peer [200 | 202]\n");
    printf("(r)efer         peer refer_header replaces_header\n");
    printf("(t)erminate as caller | callee     peer 0 | 1\n");
    printf("(p)roxy         register on proxy\n");
    printf("(u)nregister    unregister on proxy\n");
    printf("(q)uit          quit the program\n");
*/
    udpInit();


    return 0;
}

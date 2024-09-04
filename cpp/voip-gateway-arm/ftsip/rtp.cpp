/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "csengine.h"
#include "console.h"

void Csengine::runRtp(int id, bool start)
{
    session_t* session = NULL;
    QString mode;

    if(start)
    {
        session = getSessionByDid(id);
    	Q_ASSERT(session != NULL);
        session->valid = true;
    }
    else
    {
        session = getSessionByCid(id);
    	Q_ASSERT(session != NULL);
        session->valid = false;
    }
    /* logic to define kind of media session */

    // check send/recv capability for radio mode
    bool cwp_recv = false;
    bool cwp_send = false;
    bool gw_recv = false;
    bool gw_send = false;

    if(session->type == STYPE_RADIO)
    {
        // after SDP negotiations, session->attr_list string contains valid parameters mode:type:bss:fid
        mode = session->attr_list.section(':', 0, 0);
        if(!GW && (mode.contains("recv")))
            cwp_recv = true;
        if(!GW && (mode.contains("send")))
            cwp_send = true;
        if(GW && (mode.contains("recv")))
            gw_recv = true;
        if(GW && (mode.contains("send")))
            gw_send = true;
    }
    /* main logic */

    if(!GW)
    {
        // CWP side
        if(session->type == STYPE_PLAIN)
        {
            if(!FOCUS)
			{
				messageToDsp(id, R_CH_A2RTP, session, start, mode, "media from our CWP to remote CWP");
				messageToDsp(id, R_CH_RTP2A, session, start, mode, "media from remote CWP to our CWP");
			}
            else
            {
                messageToDsp(id, R_CH_RTP2RTP, session, start, mode, "media from/to remote CWP to/from our FOCUS");
            }
        }
        else if(session->type == STYPE_RADIO)
        {
            if(cwp_recv)
                messageToDsp(id, R_CH_ERTP2A, session, start, mode, "media from remote Rx to our CWP");
            if(cwp_send)
                messageToDsp(id, R_CH_A2ERTP, session, start, mode, "media from our CWP to remote Tx");
        }
        else
        {
            log("runRtp:Unknown session type", LOG_ERR);
            return;
        }

    }
    else
    {
        // GW side
        if(session->type == STYPE_PLAIN)
        {
            log("incompatible conditions - STYPE_PLAIN is wrong in GW mode", LOG_ERR);
            return;
        }
        if(session->sdp_type == SDP_OFFER)
        {
            log("incompatible conditions - SDP_OFFER is wrong in GW mode", LOG_ERR);
            return;
        }

        if(gw_recv)
            messageToDsp(id, R_CH_RX2ERTP, session, start, mode, "media from our Rx to remote CWP");
        if(gw_send)
            messageToDsp(id, R_CH_ERTP2TX, session, start, mode, "media from remote CWP to our Tx");
    }
}

void Csengine::messageToDsp(int id, e_ortp command, session_t* session, bool start, QString parm, QString text)
{
    QString cmd;
    QString str;
    QString sstart = "STOP";

    // here we ignore remote port for incoming media (always from 5555) and local port for outcoming media (also always 5555)
    // replace with 5555
    int local_port = session->local_audio_port;
    int remote_port = session->remote_audio_port;

    switch(command)
    {
    case R_CH_A2RTP:
        cmd = "CH_A2RTP";
        local_port = FIX_PORT;
        str = "#%1 CH_A2RTP localCwp[%2:%3] >send> remoteCwp[%4:%5] %6";
        break;
    case R_CH_RTP2A:
        cmd = "CH_RTP2A";
        remote_port = FIX_PORT;
        str = "#%1 CH_RTP2A localCwp[%2:%3] <wait< remoteCwp[%4:%5] %6";
        break;
    case R_CH_RTP2RTP:
        cmd = "CH_RTP2RTP";
        str = "#%1 CH_RTP2RTP localFocus[%2:%3] <wait/send> remoteCwp[%4:%5] %6";
        break;
    case R_CH_A2ERTP:
        cmd = "CH_A2ERTP";
        local_port = FIX_PORT;
        str = "#%1 CH_A2ERTP localCwp[%2:%3] >send> remoteTx[%4:%5] %6";
        break;
    case R_CH_ERTP2A:
        cmd = "CH_ERTP2A";
        remote_port = FIX_PORT;
        str = "#%1 CH_ERTP2A localCwp[%2:%3] <wait< remoteRx[%4:%5] %6";
        break;
    case R_CH_RX2ERTP:
        cmd = "CH_RX2ERTP";
        local_port = FIX_PORT;
        str = "#%1 CH_RX2ERTP localRx[%2:%3] >send> remoteCwp[%4:%5] %6";
        break;
    case R_CH_ERTP2TX:
        cmd = "CH_ERTP2TX";
        remote_port = FIX_PORT;
        str = "#%1 CH_ERTP2TX localTx[%2:%3] <wait< remoteCwp[%4:%5] %6";
        break;
    default:
        ;
    }

    if(start)
        sstart = "START";
    log(str.arg(id).
        arg(session->local_audio_ip).arg(local_port).
        arg(session->remote_audio_ip).arg(remote_port).
        arg(sstart), LOG_INFO);


    Q_EMIT signalWriteDspDatagram(cmd, session->local_audio_ip, local_port,
                                  session->remote_audio_ip, remote_port,
                                  "M", start, parm, text);
}

/*
// convert data from ertp_t to string with delimiter ';' and send to ftsip
void Dialog::writeDatagramDsp(ertp_t ertp)
{
    QString str;
    Q_ASSERT(ertp.ortp <= R_TONE2A);
    switch(ertp.ortp)
    {
    case R_CH_A2RTP:
    case R_CH_RTP2A:
    case R_CH_A2ERTP:
    case R_CH_ERTP2A:
    case R_CH_MONITOR:
        str = QString("%1;mask=%2;addr=%3:%4;switch=%5\n")
                .arg(listRtp.at(ertp.ortp)).arg(ertp.mask).arg(ertp.addr.ip).arg(ertp.addr.port).arg(ertp.sw);
        break;
    case R_CH_RTP2RTP:
        str = QString("%1;addr=%2:%3;mix=%4:%5;switch=%6\n")
                .arg(listRtp.at(ertp.ortp)).arg(ertp.addr.ip).arg(ertp.addr.port).arg(ertp.mix.ip).arg(ertp.mix.port).arg(ertp.sw);
        break;
    case R_CMD_RTPXHDR:
        str = QString("%1;addr=%2:%3;ptt_type=%4;squ=%5;ptt_id=%6;sct=%7;vf=%8;x=%9;")
                .arg(listRtp.at(ertp.ortp)).arg(ertp.addr.ip).arg(ertp.addr.port)
                .arg(ertp.eheader->ptt_type).arg(ertp.eheader->squ).arg(ertp.eheader->ptt_id)
                .arg(ertp.eheader->sct).arg(ertp.eheader->vf).arg(ertp.eheader->x);
        str += QString("ext_type=%1;ext_length=%2;ext_value=%3\n")
                .arg(ertp.eheader->ext_type).arg(ertp.eheader->ext_length).arg(ertp.eheader->ext_value);
        break;
    case R_DTMF2A:
        str = QString("%1;mask=%2;dtmf=%3\n")
                .arg(listRtp.at(ertp.ortp)).arg(ertp.mask).arg(ertp.dtmf);
        break;
    case R_DTMF2RTP:
        str = QString("%1;addr=%2:%3;dtmf=%4\n")
                .arg(listRtp.at(ertp.ortp)).arg(ertp.mask).arg(ertp.addr.ip).arg(ertp.addr.port).arg(ertp.dtmf);
        break;
    case R_TONE2A:
        str = QString("%1;mask=%2;switch=%3,tone=%4\n")
                .arg(listRtp.at(ertp.ortp)).arg(ertp.mask).arg(ertp.sw).arg(ertp.tone);
        break;
    default:
        Q_ASSERT_X(false, "Dialog::writeDatagramDsp(ertp_t ertp)", "wrong ertp.ortp");
    }

    QByteArray data = str.toUtf8();
    udpLocalSocketDsp->writeDatagram(data, host_dsp_board, port_control_dsp_board_dst);
    log(QString(">> ") += str.remove('\n'), LOG_UDP);
}
*/

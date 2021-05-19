/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "console.h"

void Console::confServer(e_sip event, QString peer, QString header, QString body, QString parm, QString)
{
    focus_event = event;
    focus_peer = peer;
    focus_header = header;
    focus_body = body;
    focus_parm = parm;
    //qDebug() << "confServer" << text;

    /*
     ****************
     Incoming call from conf initiator
     ****************
    */

    if(SIP_FOCUS(D_OFF,EM_INVITE,D_TRYING_OUT, "D_OFF > EM_INVITE > D_TRYING_OUT"))
    {
        qDebug() << "\tFocus receives invite from conf initiator:" << focus_peer;
        // if incomint invite, we have to send ringing automaticly - operator doesn't do that from hmi
        parseAtomCmd(EC_RINGING, focus_peer, focus_header, "isfocus");
        parseAtomCmd(EC_200_OK, focus_peer,  focus_header, "isfocus");
        focus_disp = D_CALL_IN;
        return;
    }
    if(SIP_FOCUS(D_CALL_IN,EM_ACK,D_CALL_IN, "D_CALL_IN > EM_ACK > D_CALL_IN"))
    {
        qDebug() << "\tIncoming call from conf initiator established";
        return;
    }

    /*
     ****************
     Incoming subscribe to conf for incoming call (initiator)
     ****************
    */

    if(SIP_FOCUS(D_CALL_IN,EM_SUBSCRIBE,D_SUBS_CONF_INIT_OK, "D_CALL_IN > EM_SUBSCRIBE > D_SUBS_CONF_INIT_OK"))
    {
        qDebug() << "\tFocus receives confevent subscribe request from conf initiator:" << focus_peer;
        return; // ftsip sent subscribe OK without us
    }
    if(SIP_FOCUS(D_SUBS_CONF_INIT_OK,EM_NOTIFY_OK,D_NOTIFY_CONF_OK, "D_SUBS_CONF_INIT_OK > EM_NOTIFY_OK > D_NOTIFY_CONF_OK"))  // sent notify wihout us
        // at this point focus is established
        return;

    /*
     ****************
     Incoming REFER and INVITE refered future conf member
     ****************
    */

    if(SIP_FOCUS(D_NOTIFY_CONF_OK,EM_REFER,D_REFER_TRYING, "D_NOTIFY_CONF_OK > EM_REFER > D_REFER_TRYING"))
    {
        qDebug() << "\tFocus receives refer from conf initiator:" << focus_peer;
        // parse REFER headers
        // "sip:cwp2@192.168.52.2:5060/sip:cwp1@192.168.52.1:5060;sip:cwp2@192.168.52.2:5060"
        // QString header = focus_header.remove("sip:");
        referto = focus_header;
        referto.remove("sip:");
        replaces = focus_parm;
        // sideA had connection with sideB
        // sideB should brake connection with sideA and replace to focus
        // to provide this:
        // sideA sent referto (sideB) to focus that focus invite sideB
        // sideA also sent replaces (sideA;sideB) that focus invite sideB to call and then tell sideB terminate call with sideA
        qDebug() << "Refer headers:" << header << referto << replaces;
        return;
    }
    // start real REFER staff only after subscribe initiator to REFER notifies
    if(SIP_FOCUS(D_REFER_TRYING,EM_SUBSCRIBE,D_SUBS_REFER_OK, "D_REFER_TRYING > EM_SUBSCRIBE > D_SUBS_REFER_OK"))
    {
        qDebug() << "\tFocus receives refer subscribe request from conf initiator:" << focus_peer;
        return;
    }
    if(SIP_FOCUS(D_SUBS_REFER_OK,EM_NOTIFY_OK,D_NOTIFY_REFER_OK, "D_SUBS_REFER_OK > EM_NOTIFY_OK > D_NOTIFY_REFER_OK"))
    {
        qDebug() << "\tFocus invites refered member:" << referto;
        parseAtomCmd(EC_CALL, referto, "isfocus", replaces);
        return;
    }

    if(SIP_FOCUS(D_NOTIFY_REFER_OK,EM_TRYING,D_TRYING_IN, "D_REFER_TRYING > EM_TRYING > D_TRYING_IN")) return;
    if(SIP_FOCUS(D_TRYING_IN,EM_RINGING,D_BLINK_OUT, "D_TRYING_IN > EM_RINGING > D_BLINK_OUT")) return;
    if(SIP_FOCUS(D_BLINK_OUT,EM_200_OK,D_CALL_OUT, "D_BLINK_OUT > EM_200_OK > D_CALL_OUT"))
    {
        qDebug() << "\tFocus established call with invited conf member";
        return;
    }


    /*
     ****************
     Subscribe invited member to conf event
     ****************
    */

    if(SIP_FOCUS(D_CALL_OUT,EM_SUBSCRIBE,D_SUBS_CONF_NEXT_OK, "D_CALL_OUT > EM_SUBSCRIBE > D_SUBS_CONF_NEXT_OK"))
    {
        qDebug() << "\tFocus receives confevent subscribe request from member:" << focus_peer;
        return; // ftsip sent subscribe OK without us
    }
    if(SIP_FOCUS(D_SUBS_CONF_NEXT_OK,EM_NOTIFY_OK,D_NOTIFY_CONF_OK, "D_SUBS_CONF_NEXT_OK > EM_NOTIFY_OK > D_NOTIFY_CONF_OK")) return;
    // to catch all notify ok from a lot of members
    if(SIP_FOCUS(D_NOTIFY_CONF_OK,EM_NOTIFY_OK,D_NOTIFY_CONF_OK, "D_NOTIFY_CONF_OK > EM_NOTIFY_OK > D_NOTIFY_CONF_OK")) return;

    /*
     ****************
     Member left focus
     ****************
    */

    if(SIP_FOCUS(D_NOTIFY_CONF_OK,EM_UNSUBSCRIBE,D_NOTIFY_CONF_OK, "D_NOTIFY_CONF_OK > EM_UNSUBSCRIBE > D_NOTIFY_CONF_OK")) return;
    if(SIP_FOCUS(D_NOTIFY_CONF_OK,EM_TERM_IN,D_NOTIFY_CONF_OK, "D_NOTIFY_CONF_OK > EM_TERM_IN > D_NOTIFY_CONF_OK")) return;
    if(SIP_FOCUS(D_NOTIFY_CONF_OK,EM_FOCUS_EMPTY,D_OFF, "D_NOTIFY_CONF_OK > EM_FOCUS_EMPTY > D_OFF")) return;

    /*
     ****************
     Last member left focus and release
     ****************
    */

    if(SIP_FOCUS(D_OFF,EM_TERM_IN,D_OFF, "D_OFF > EM_TERM_IN > D_OFF")) return;

    log(QString("SIP_FOCUS: Didn't catch SIP event peer [%1] event:%2 disp:%3").
        arg(focus_peer).arg(focus_event).arg(focus_disp), LOG_WARN);
}

bool Console::SIP_FOCUS(e_disp disp_check, e_sip event_check, e_disp disp_new, QString text)
{
    if(event_check == focus_event && disp_check == focus_disp)
    {
        log(QString("SIP_FOCUS %1 host=%2 header:%3").arg(text).arg(focus_peer).arg(focus_header), LOG_ON);
        focus_disp = disp_new;
        return true;
    }
    return false;
}

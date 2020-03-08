/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <stdio.h>
#include <eXosip2/eXosip.h>

#include "csengine.h"
#include "console.h"

void Csengine::procHeaders(eXosip_event_t *event)
{
    header = "";
    header_expires = "";
    header_ss = "";
    sbody = "";
    url_from = event->request->from->url;
    url_to = event->request->to->url;
    addr_from = QString("%1@%2:%3").arg(url_from->username).arg(url_from->host).arg(url_from->port);
    addr_to = QString("%1@%2:%3").arg(url_to->username).arg(url_to->host).arg(url_to->port);

    content_type = osip_message_get_content_type(event->request);
    QString type = "";
    if(content_type)
        type = QString(content_type->type);
    if(type == "message" || type == "text")
    {
        int pos = -1;
        while(osip_message_get_body(event->request, ++pos, &body) != -1)
        {
            osip_body_to_str(body, &str, &length);
            sbody = QString(str).left(length);
            sbody.remove("\n\r");
        }
    }
    osip_message_header_get_byname (event->request, "Event", 0, &event_head);
    if(event_head)
        header = event_head->hvalue;
    osip_message_header_get_byname (event->request, "Expires", 0, &expires_head);
    if(expires_head)
        header_expires = expires_head->hvalue;
    osip_message_header_get_byname (event->request, "Subscription-State", 0, &ss_head);
    if(ss_head)
        header_ss = ss_head->hvalue;
    //qDebug() << "header, body:" << header << sbody;
}

void Csengine::procMessageNew(eXosip_event_t *event)
{
    // qDebug() << "EXOSIP_MESSAGE_NEW";
    extLog(event);
    if(MSG_IS_OPTIONS(event->request))
    {
        log("<< procMessageNew:OPTIONS", LOG_SIP);
        return;
    }

    if(MSG_IS_REFER(event->request))
    {
        // refer outside a dialog - we don't use it
        log(QString("<< procMessageNew:REFER cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        log("Doesn't support ouside call REFER", LOG_WARN);
        return;
/*
        osip_message_header_get_byname (event->request, "Refer-to", 0, &referto_head);
        if(referto_head)
        {
            referto = referto_head->hvalue;
            log(QString("referto_head: %1").arg(referto), LOG_SIP);
        }
        osip_message_header_get_byname (event->request, "Replaces", 0, &replaces_head);
        if(replaces_head)
        {
            replaces = replaces_head->hvalue;
            log(QString("replaces_header: %1").arg(replaces), LOG_SIP);
        }
        parm = replaces;
        Q_EMIT signalWriteAtomDatagram(EM_REFER, addr_from, header, "EM_REFER");
        sendOk(event->tid, 202);
        sendNotify("", "refer", "SIP/2.0 100 Trying\n\r", event->did);
        return;
*/
    }
    if(MSG_IS_SUBSCRIBE(event->request))
    {
        log("<< procMessageNew:SUBSCRIBE", LOG_SIP);
        return;
    }

    if(MSG_IS_NOTIFY(event->request))
    {
        log(QString("<< procMessageNew:NOTIFY event=%1 expires=%2 ss=%3").arg(header).arg(header_expires).arg(header_ss), LOG_SIP);
        return;
    }

}

bool Csengine::procCallProceeding(eXosip_event_t *event)
{
    if(event->did == 0)
        // skip first 100 Trying
        return false;
    // process second 101 Dialog Establishement
    reference = eXosip_call_get_reference(event->cid);
    Q_ASSERT(reference);
    session = static_cast<session_t*>(reference);
    session->did = event->did;
    session->last_tid = event->tid;
    return true;
}

bool Csengine::procSubsProceeding(eXosip_event_t *event)
{
    // session->s->did was stored in sendSubscribe
    // now, we obtained read did by s_did and store real did in subscribe session
    session = getSessionBySid(event->sid);
    if(session == NULL)
        return false;

    session->last_tid = event->tid;
    session->did = event->did;
    return true;
}

bool Csengine::procCallAnswered(eXosip_event_t *event)
{
    reference = eXosip_call_get_reference(event->cid);
    Q_ASSERT(reference);
    session = static_cast<session_t*>(reference);

    if(checkRemoteSdp(event->did, session->type, session->sdp_type))
    {
        sendAck(event->did);
		// FIXME: exception rise here. fix sdp!
        if(session->type == STYPE_RADIO)
        {
            QStringList parsedBody = session->remote_sdp_body.split("a=");
            QRegExp re;
            re.setPattern(QString("^%1.*$").arg("mode:"));
            int res = parsedBody.indexOf(re);
            Q_ASSERT(res != -1);
            parm = parsedBody.at(res);
            parm.remove('\n');
            parm.remove('\r');
            parm.remove("mode:");
        }
        return true;
    }
    else
    {
        sendError415(event->tid);
        return false;
    }

}

bool Csengine::procCallInvite(eXosip_event_t *event)
{
    int res;
    // ***** NEW IN SESSION *****
    // first, test input SDP
    if(GW)
        session_type = STYPE_RADIO;
    else
        session_type = STYPE_PLAIN;

    // check if we already have sessions with the same from and to - kill it
    for(int i=0; i<slist.count(); ++i)
        if(slist.at(i)->addr_from == addr_from and slist.at(i)->addr_to == addr_to)
            slist.removeAt(i);
    session = new(session_t);
    session->valid = false;
    session->type = session_type;
    session->cid = event->cid;
    session->did = event->did;
    session->last_tid = event->tid;
    session->s_did = -1;
    session->addr_from = addr_from;
    session->addr_to = addr_to;
    session->sdp_type = SDP_ANSWER;
    session->log = "receive invite";
    slist.append(session);
    res = eXosip_call_set_reference (event->cid, session);
    if(res != 0)
    {
        log(QString("procCallInvite: can't set reference for cid=%1").arg(event->cid), LOG_ERR);
        return res;
    }

    osip_message_header_get_byname (event->request, "Event", 0, &event_head);
    if(event_head)
    {
        header = event_head->hvalue;
        log(QString("event_header: %1").arg(header), LOG_SIP);
    }
    osip_message_header_get_byname (event->request, "Replaces", 0, &replaces_head);
    if(replaces_head)
    {
        parm = replaces_head->hvalue;
        log(QString("replaces_header: %1").arg(parm), LOG_SIP);
    }

    if(!checkRemoteSdp(event->did, session_type, session->sdp_type))
    {
        sendError415(event->tid);
        return false;
    }
        // ToDo: send answer in radio.cpp? as focus
    if(GW)
        sendAnswer(addr_from, STYPE_RADIO, "", m_localRadioModes);

    return true;
}

bool Csengine::procCallReInvite(eXosip_event_t *event)
{
    if(GW)
        session_type = STYPE_RADIO;
    else
        session_type = STYPE_PLAIN;
    if(!checkRemoteSdp(event->did, session_type, session->sdp_type))
    {
        sendError415(event->tid);
        return false;
    }

    // should update tid - after reinvite came new tid
    session->last_tid = event->tid;
    if(GW)
        sendAnswer(addr_from, STYPE_RADIO, "", m_localRadioModes);
    return true;

}

void Csengine::procCallAck(eXosip_event_t *event)
{
    reference = eXosip_call_get_reference(event->cid);
    Q_ASSERT(reference);
    session = static_cast<session_t*>(reference);
    session->last_tid = event->tid;
    runRtp(event->did, TRUE);
}

void Csengine::procInSubscriptionNew(eXosip_event_t *event)
{
    /*
        supports several kinds of subscriptions
        1) FOCUS receives REFER - just send 100 Trying and 200 Ok NOTIFY to track REFER executing by conf initiator, who sent this REFER
           store did to send 200 Ok in future
        2) GW receives WG67 header - new CWP has connected to Tx, add CWP uri to radio list, NOTIFY whole list to all CWPs connected to this CW
        3) FOCUS receives "confevent" header - new CWP becomes conf member now. Send NOTIFY to all conf members about only this new member
    */

    static int refer_did;
    if(FOCUS && header == "refer")
    {
        // to track REFER
        sendSubscribeAnswer(event->tid);
        sendNotify("", "refer", "SIP/2.0 100 Trying\n\r", event->did);
        refer_did = event->did;
        return;
    }
    // notify all subscribers about new subscriber and include new subscriber in subscribers list

    // prepare body to send NOTIFY
    subscribe_t* subs = new(subscribe_t);
    subs->peer = addr_from;
    subs->event = header;
    subs->did = event->did;
    subs->tid = event->tid;
    if(GW && header == "WG67 KEY-IN")
    {
        // subscribe to Tx ptt_id notify
        subscribeRadioList.append(subs);
        // ToDo: unsubscribe to release ptt_id
        subs->ptt_id = subscribeRadioList.count();
        sbody = "";
        // send all ptt-id in one body
        for(int i=0; i<subscribeRadioList.count(); ++i)
        {
            subs = subscribeRadioList.at(i);
            sbody += QString("%1,sip:%2\r\n").arg(subs->ptt_id);
        }
        sendSubscribeAnswer(event->tid);
        // sent NOTIFYs
        for(int i=0; i<subscribeRadioList.count(); ++i)
            sendNotify("", subscribeRadioList.at(i)->event, sbody, event->did);

    }
    else if(FOCUS && header =="confevent")
    {
        QString body = "";
        sendSubscribeAnswer(event->tid);
        if(header_expires != "")
        {
            // subs is new member
            subscribeConfList.append(subs);

            // new member should receive full member list
            // to know about conf members before its
            for(int i=0; i<subscribeConfList.count(); ++i)
            {
                subscribe_t* current_sub = subscribeConfList.at(i);
                // case when conf initiator was skipped above - avoid send empty NOTIFY
                if(current_sub->peer == subs->peer && subscribeConfList.count() > 1)
                    continue;
                // body includes delimiter '/' to split up list in ftgui
                body += QString("FTCONF,+,%1\n\r/").arg(current_sub->peer);
            }
            log(QString("notify new member [%1] about current members [%2]").arg(subs->peer).arg(body), LOG_SIP);
            sendNotify("", subs->event, body, subs->did);

            // send notify about new member to all subscribers
            // exluding sending to new member about itself - to avoid send double notify before notify answer come for first notify
            body = QString("FTCONF,+,%1\n\r").arg(subs->peer);
            // sent NOTIFYs
            for(int i=0; i<subscribeConfList.count(); ++i)
            {
                subscribe_t* current_sub = subscribeConfList.at(i);
                if(current_sub->peer == subs->peer)
                    continue;
                log(QString("notify member [%1] about new member [%2]").arg(current_sub->peer).arg(subs->peer), LOG_SIP);
                sendNotify("", current_sub->event, body, current_sub->did);
            }
            // complete REFER transaction
            if(refer_did != 0)
                sendNotify("", "refer", "SIP/2.0 200 Ok\n\r", refer_did);
        }
        else
        {
            // unsubscribe: expires=0

            // ToDo: should we reply NOTIFY to unsubsribe?
            // left member should receive "terminated" status
            // log(QString("notify member [%1] that it left conf").arg(subs->peer), LOG_SIP);
            // sendNotify("", subs->event, "", subs->did, true);

            // send notify about left member to all subscribers
            // exluding sending to left member about itself - to avoid send double notify before notify answer come for first notify
            body = QString("FTCONF,-,%1\n\r").arg(subs->peer);
            // sent NOTIFYs
            int save = -1;
            for(int i=0; i<subscribeConfList.count(); ++i)
            {
                subscribe_t* current_sub = subscribeConfList.at(i);
                if(current_sub->peer == subs->peer)
                {
                    // no need send notify to left member
                    sendNotify("", current_sub->event, body, current_sub->did, true);
                    log(QString("notify member [%1] that it left").arg(current_sub->peer), LOG_SIP);
                    save = i;
                    continue;
                }
                log(QString("notify member [%1] about left member [%2]").arg(current_sub->peer).arg(subs->peer), LOG_SIP);
                sendNotify("", current_sub->event, body, current_sub->did);
            }
            Q_ASSERT(save != -1);
            subscribeConfList.removeAt(save);

            /*
            session_t* session;
            session = getSessionByFrom(subs->peer, false);
            if(session != NULL)
            {
                slist.removeAll(session);
                log(QString("remove from subscribe list and session conf initiator [%1]").arg(subs->peer), LOG_SIP);
            }
            else
            {
                session = getSessionByTo(subs->peer, false);
                if(session != NULL)
                {
                    slist.removeAll(session);
                    log(QString("remove from subscribe list and session conf member [%1]").arg(subs->peer), LOG_SIP);
                }
                else
                    log(QString("procInSubscriptionNew: can't find session peer=[%1] for remove").arg(subs->peer), LOG_WARN);
            }
            */

        }
    }
    else
    {
        log(QString("Unexpected subscribe: %1").arg(header), LOG_WARN);
        sendSubscribeAnswer(event->tid, 400);
    }
    //
    log(QString("cons/subs: %1/%2").arg(listSessions(false)).arg(subscribeConfList.count()), LOG_SIP);
}


void Csengine::procByeSenderClosed(eXosip_event_t *event)
{
    // called when we are BYE initiator and received BYE OK
    reference = eXosip_call_get_reference(event->cid);
    Q_ASSERT(reference);
    session = static_cast<session_t*>(reference);
    index = slist.indexOf(session);
    if(index != -1)
    {
        runRtp(event->cid, FALSE);
        slist.removeAt(index);
    }
    else
        log(QString("procByeSenderClosed: couldn't find session cid=%1 in list for removing").arg(event->cid), LOG_WARN);
    listSessions();
    return;
}

void Csengine::procByeReceiverClosed(eXosip_event_t *event)
{
    // called when we received BYE from peer, sent BYE OK automatically and receive CALL_CLOSED from sip lib
    // BYE 200 OK send exosip itself?

    // BYE from last conf member? we should say focus to reset state engine
    if(FOCUS && subscribeConfList.isEmpty())
    {
        header = "";
        sbody = "";
        parm = "";
        messageToAtom(EM_FOCUS_EMPTY, "", "No more conf members");
    }
    reference = eXosip_call_get_reference(event->cid);
    Q_ASSERT(reference);
    session = static_cast<session_t*>(reference);
    index = slist.indexOf(session);
    if(index != -1)
    {
        runRtp(event->cid, FALSE);
        slist.removeAt(index);
    }
    else
        log(QString("procByeReceiverClosed: couldn't find session cid=%1 in list for removing").arg(event->cid), LOG_WARN);

    listSessions();
    return;
}

void Csengine::procCallMessageNewRefer(eXosip_event_t *event)
{
    reference = eXosip_call_get_reference (event->cid);
    Q_ASSERT(reference);
    session = static_cast<session_t*>(reference);
    session->last_tid = event->tid;

    osip_message_header_get_byname (event->request, "Refer-to", 0, &referto_head);
    if(referto_head)
    {
        header = referto_head->hvalue;
        log(QString("referto_head: %1").arg(header), LOG_SIP);
    }
    else
        header = "";
    osip_message_header_get_byname (event->request, "Replaces", 0, &replaces_head);
    if(replaces_head)
    {
        parm = replaces_head->hvalue;
        log(QString("replaces_header: %1").arg(parm), LOG_SIP);
    }
    else
        parm = "";
    sendOk(event->tid, 202);
}

// called two times during registration
// first with current contact ip:port
// second with new contact received:rport
void Csengine::procRegisterProxy(eXosip_event_t *event)
{
    osip_via_t* via;
    //osip_contact_t* contact;
    char* string;

    if(rport == 0)
    {
        log(QString("<< AUTO REGISTRATION PASS1 SUCCESS rid=%1").arg(event->rid), LOG_SIP);
        osip_message_get_via(event->response, 0, &via);
        if(via)
        {
            osip_via_to_str(via, &string);
            QStringList list = QString(string).split(';');
            received = parseField(list, "received=");
            rport = parseField(list, "rport=").toInt();

            log(QString("Via received: [%1] rport: [%2]").arg(received).arg(rport), LOG_SIP);
        }
        log(QString("Change contact data from [%1:%2] to [%3:%4] and reregister").
            arg(m_contact_ip).arg(m_contact_port).arg(received).arg(rport), LOG_SIP);
        sendRegister(received, rport, 3600);
        eXosip_masquerade_contact(received.toAscii().data(), rport);
    }
    else
    {
        log(QString("<< AUTO REGISTRATION PASS2 SUCCESS rid=%1").arg(event->rid), LOG_SIP);
        // for sendAnswer()
        //m_contact_ip = "5.231.68.122";
        //eXosip_masquerade_contact(m_contact_ip.toAscii().data(), m_contact_port);
    }

/*
    int pos = 0;
    bool pass2 = FALSE;

    while(TRUE)
    {
        osip_message_get_contact(event->response, pos++, &contact);
        if(contact == NULL)
            break;

        osip_contact_to_str(contact, &string);
        if(received == m_contact_ip)
            // contact ip already changed - we are in pass2
            pass2 = TRUE;
        //log(QString("Contact: [%1]").arg(QString(string)), LOG_SIP);
    }
    if(!pass2)
    {
        // first (no password, gray IP) registration passed
        log(QString("<< REGISTRATION PASS1 SUCCESS rid=%1").arg(event->rid), LOG_SIP);
        log(QString("Change contact data from [%1:%2] to [%3:%4]").
            arg(m_contact_ip).arg(m_contact_port).arg(received).arg(rport), LOG_SIP);
        //m_contact_ip = received;
        //m_contact_port = rport;
        //eXosip_masquerade_contact(m_contact_ip.toAscii().data(), m_contact_port);
        // register again (pass2) with new contact ip obtained from received:port
        // ToDo: remove 1st registration?
        //sendRegister(0);
        sendRegister(received, 5060, 3600);
    }
    else
        log(QString("<< REGISTRATION PASS2 SUCCESS rid=%1").arg(event->rid), LOG_SIP);
*/
}

void Csengine::procCallReleased(eXosip_event_t *event)
{
    // should find stored session object for deleting session from list
    // attention! session_t session reference has already been deleted by eXosip after releasing!
    // no warning because previois session with the same addr_from and addr_to removed during incoming or outcoming INVITE
    qDebug() << "release";
    session = getSessionByCid(event->cid, false);
    if(session != NULL)
        log(QString("procCallReleased: session cid=%1 was not deleted during BYE transaction").arg(event->cid), LOG_ERR);

    /*
    Q_ASSERT(session != NULL);

    // can't use procHeaders(): after release event structure becomes invalid
    // use addrs stored in session
    addr_from = session->addr_from;
    addr_to = session->addr_to;
    e_sdp_type res = session->sdp_type;
    index = slist.indexOf(session);
    if(index == -1)
        log(QString("Couldn't find session %1 in list for removing").arg(event->cid), LOG_ERR);
    else
        slist.removeAt(index);
    return res;
    // dont't place deleting object here - resources have already released!
    */
}

/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <stdio.h>
#include <eXosip2/eXosip.h>

#include "csengine.h"
#include "console.h"

CEvent::CEvent()
{
}

void CEvent::slotStartWait()
{
    eXosip_event_t *event = NULL;

    while(1)
    {
        event = eXosip_event_wait (0, 50);
        if(event != NULL)
            signalSipEvent(event);
    }

}

void Csengine::slotSipEvent(eXosip_event_t *event)
{
    if (!event)
    {
        eXosip_event_free(event);
        log("empty event", LOG_WARN);
        return;
    }

    //qDebug() << "event type:" << event->type;

    eXosip_lock();
    eXosip_automatic_action();
    // for proxy authentification processing
    //eXosip_default_action(event);
    eXosip_unlock();

    // osip_content_disposition_t* content_disposition;

    if(event->type == EXOSIP_MESSAGE_REQUESTFAILURE)
    {
        log("MESSAGE_REQUESTFAILURE", LOG_ERR);
        extLog(event);
        eXosip_event_free(event);
        return;
    }

    //int i;
    // qDebug() << "Exosip event";
    // crash if event->type = 7
    // importante that comparing with RELEASED will be first to avoid crash asking incorrect event request
    if(event->type != EXOSIP_CALL_RELEASED && event->type != EXOSIP_CALL_NOANSWER
            && event->type != EXOSIP_CALL_CLOSED && MSG_IS_REQUEST(event->request))
    {
        procHeaders(event);
    }
    switch (event->type)
    {
    // запрос OPTIONS. можно не отвечать, здесь и далее - как бы не испортить cid, tid, did
    case EXOSIP_MESSAGE_NEW:
        log(QString("<< MESSAGE_NEW %1 cid=%2, tid=%3, did=%4").
            arg(event->request->sip_method).arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        procMessageNew(event);
        break;

        /******************************
          ACTIVE FROM side - we send a call
         *****************************/

        // мы вызывали peer - к нам пришло Trying
    case  EXOSIP_CALL_PROCEEDING:
        log(QString("<< CALL_PROCEEDING cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        if(procCallProceeding(event))
            messageToAtom(EM_TRYING, addr_to, "EM_TRYING");
        break;

        // we sent Subsribe - Subscribe Trying coming
    case  EXOSIP_SUBSCRIPTION_PROCEEDING:
        log(QString("<< SUBSCRIPTION_PROCEEDING cid=%1, tid=%2, did=%3 s_did=%4").
            arg(event->cid).arg(event->tid).arg(event->did).arg(event->sid), LOG_SIP);
        procSubsProceeding(event);
        break;

        // мы вызывали peer - после Trying пришло Ringing
    case  EXOSIP_CALL_RINGING:
        log(QString("<< CALL_RINGING tid=%1").arg(event->tid), LOG_SIP);
        messageToAtom(EM_RINGING, addr_to, "EM_RINGING");
        break;

        // мы вызывали peer - там сняли трубку (200 OK)
    case  EXOSIP_CALL_ANSWERED:
        log(QString("<< CALL_ANSWERED cid=%1, tid=%2, did=%3 head:%4").
            arg(event->cid).arg(event->tid).arg(event->did).arg(header), LOG_SIP);

        if(procCallAnswered(event))
            messageToAtom(EM_200_OK, addr_to, "EM_200_OK");
        else
            messageToAtom(EM_ERROR, addr_to, "SDP fail");
        break;

        // we sent Subscribe to peer and receive 200 OK - subscribe sucsessfull
    case  EXOSIP_SUBSCRIPTION_ANSWERED:
        log(QString("<< SUBSCRIPTION_ANSWERED cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        messageToAtom(EM_SUBSCRIBE_OK, addr_to, "EM_SUBSCRIBE_OK");
        break;

        // we sent Notify to peer and receive 200 OK - notify sucsessfull
    case  EXOSIP_NOTIFICATION_ANSWERED:
        log(QString("<< NOTIFICATION_ANSWERED cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        notify_flag_did = event->did;
        messageToAtom(EM_NOTIFY_OK, addr_to, "EM_NOTIFY_OK");
        break;

        // peer is calling us in progress and break before establish connection
    case  EXOSIP_CALL_CANCELLED:
        log(QString("<< CALL_CANCELLED cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        break;

        // мы вызывали peer - он не снимает трубку
    case  EXOSIP_CALL_TIMEOUT:
        log(QString("<< CALL_TIMEOUT cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        break;

        // we are calling in progress and drop before call established
        // cancel request
    case  EXOSIP_CALL_REQUESTFAILURE:
        log(QString("<< CALL_REQUESTFAILURE cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        procByeSenderClosed(event);
        messageToAtom(EM_CANCEL, addr_to, "EM_CANCEL");
        break;

        // we calling in progress and peer drop our call (send terminate) before call established.
        // so, we have globalfailure instead of bye
        // decline message
    case  EXOSIP_CALL_GLOBALFAILURE:
        log(QString("<< CALL_GLOBALFAILURE cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        procByeSenderClosed(event);
        messageToAtom(EM_DECLINE, addr_to, "EM_DECLINE");
        break;

        // answer to BYE and REFER
    case EXOSIP_CALL_MESSAGE_ANSWERED:
        if(QString(event->request->sip_method) == "BYE")
        {
            // мы посылали peer сообщение BYE - в ответ получили 200 Ok, опознаем его по cid
            log(QString("<< CALL_MESSAGE_ANSWERED: BYE %1 cid=%2, tid=%3, did=%4").
                arg(event->request->status_code).arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
            procByeSenderClosed(event);
            // note the BYE is request, not status, and BYE initiator addr is always addr_from
            messageToAtom(EM_TERM_OUT, addr_to, "EM_TERM_OUT");
            break;
        }
        if(QString(event->request->sip_method) == "REFER")
        {
            // мы посылали peer сообщение REFER - в ответ получили 202 Accepted
            log(QString("<< CALL_MESSAGE_ANSWERED: REFER %1 cid=%2, tid=%3, did=%4").
                arg(event->request->status_code).arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
            messageToAtom(EM_REFER_ACCEPT, addr_to, "EM_REFER_ACCEPT");
            break;
        }

        log(QString("<< UNPROCESSED CALL_MESSAGE_ANSWERED %1 cid=%2, tid=%3, did=%4").
            arg(event->request->sip_method).arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        break;


        /******************************
        PASSIVE TO side - we receive a call
        *****************************/

        // peer вызывает нас в первый раз
    case  EXOSIP_CALL_INVITE:
        // ToDo: distinct WG event and standard events
        log(QString("<< CALL_INVITE cid=%1, tid=%2, did=%3 %4 > %5 header:%6").
            arg(event->cid).arg(event->tid).arg(event->did).arg(addr_from).arg(addr_to).arg(header), LOG_SIP);

        if(!procCallInvite(event))
            messageToAtom(EM_ERROR, addr_from, "EM_ERROR:SDP fail in ANSWER mode");
        else
            messageToAtom(EM_INVITE, addr_from, "EM_INVITE");
        break;

        // reinvite with new sdp
    case  EXOSIP_CALL_REINVITE:
        log(QString("<< CALL_REINVITE cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);

        if(!procCallReInvite(event))
            messageToAtom(EM_ERROR, addr_from, "EM_ERROR:SDP fail in ANSWER mode");
        else
            messageToAtom(EM_REINVITE, addr_from, "EM_REINVITE");
        break;

        // peer вызывал нас - мы ответили 200 OK и теперь получили ACK, сессия установлена
    case  EXOSIP_CALL_ACK:
        // should be updated because ACK set new tid but eXosip won't
        log(QString("<< CALL_ACK cid=%1, tid=%2, did=%3").arg(event->cid).
            arg(event->tid).arg(event->did), LOG_SIP);
        procCallAck(event);
        messageToAtom(EM_ACK, addr_from, "EM_ACK");
        break;

    // peer sent us Subscribe request
    case  EXOSIP_IN_SUBSCRIPTION_NEW:
        log(QString("<< IN_SUBSCRIPTION_NEW cid=%1, tid=%2, did=%3 header=%4").
            arg(event->cid).arg(event->tid).arg(event->did).arg(header), LOG_SIP);
        procInSubscriptionNew(event);
        if(header_expires != "")
            messageToAtom(EM_SUBSCRIBE, addr_from, "EM_SUBSCRIBE");
        else
            messageToAtom(EM_UNSUBSCRIBE, addr_from, "EM_UNSUBSCRIBE");
        break;

    // peer sent us Notify request
    case  EXOSIP_SUBSCRIPTION_NOTIFY:
        log(QString("<< SUBSCRIPTION_NOTIFY cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        if(header == "refer")
        {
            if(sbody.contains("100"))
            {
                messageToAtom(EM_NOTIFY_REFER_100, addr_from, "EM_NOTIFY_REFER_100");
                break;
            }
            if(sbody.contains("200"))
            {
                messageToAtom(EM_NOTIFY_REFER_200, addr_from, "EM_NOTIFY_REFER_200");
                break;
            }
            log(QString("REFER NOTIFY: unexpected body %1").arg(sbody), LOG_WARN);
            break;
        }
        if(header == "confevent")
            messageToAtom(EM_NOTIFY_CONF, addr_from, "EM_NOTIFY_CONF");
        break;

        // eXosip inform that it sent BYE 200 OK itself, without as as replies to incoming BYE
    case  EXOSIP_CALL_CLOSED:
        log(QString("<< CALL_CLOSED cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        procByeReceiverClosed(event);
        // note the BYE is request, not status, and BYE initiator addr is always addr_from
        messageToAtom(EM_TERM_IN, addr_from, "EM_TERM_IN");
    break;

        break;

        // receive BYE or REFER
    case EXOSIP_CALL_MESSAGE_NEW:
        if(QString(event->request->sip_method) == "BYE")
        {
            log(QString("<< CALL_MESSAGE_NEW: BYE cid=%1, tid=%2, did=%3").
                arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
            // procCallMessageNewBye(event);
            //  messageToAtom(EM_TERM, addr_from, "EM_TERM");
            break;
        }
        if(QString(event->request->sip_method) == "REFER")
        {
            // refer inside the dialog
            log(QString("<< CALL_MESSAGE_NEW: REFER cid=%1, tid=%2, did=%3").
                arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
            procCallMessageNewRefer(event);
            messageToAtom(EM_REFER, addr_from, "EM_REFER");
            // sendNotify("", "refer", "SIP/2.0 100 Trying", event->did);
            break;
        }

        log(QString("<< UNPROCESSED CALL_MESSAGE_NEW %1 cid=%2, tid=%3, did=%4").
            arg(event->request->sip_method).arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        break;


        /******************************
          event for both caller and callee
          *****************************/

        // дополнительная информация о том, что все ресурсы освобождены после завершения звонка
        // ссылки на память в event становятся недействительными. это сообщение формирует сам eXosip2
    case  EXOSIP_CALL_RELEASED:
        log(QString("<< CALL_RELEASED cid=%1, tid=%2, did=%3").
            arg(event->cid).arg(event->tid).arg(event->did), LOG_SIP);
        break;

        // ссылки на память в event становятся недействительными. это сообщение формирует сам eXosip2
    case  EXOSIP_CALL_NOANSWER:
        log(QString("<< CALL_NOANSWER cid=%1").arg(event->cid), LOG_SIP);
        break;

        /******************************
          REGISTRATION
          *****************************/

        // ссылки на память в event становятся недействительными. это сообщение формирует сам eXosip2
    case  EXOSIP_REGISTRATION_FAILURE:
        log(QString("<< REGISTRATION_FAILURE rid=%1, should be automated action").arg(event->rid), LOG_SIP);
        break;

        // ссылки на память в event становятся недействительными. это сообщение формирует сам eXosip2
    case  EXOSIP_REGISTRATION_SUCCESS:
        procRegisterProxy(event);
        // log in procRegisterProxy()
        break;

    default:
        log(QString("<< UNDEFINED MESSAGE type=%1 cid=%2, tid=%3, did=%4").
            arg(event->type).arg(event->cid).arg(event->tid).arg(event->did), LOG_WARN);
        extLog(event);

        break;
    }

    eXosip_event_free(event);
}

void Csengine::messageToAtom(e_sip sip, QString peer, QString text)
{
    Q_EMIT signalWriteAtomDatagram(sip, peer, header, sbody, parm, text);
}

void Csengine::extLog(eXosip_event_t* event)
{
    qDebug() << "textinfo:" << event->textinfo;
    if(event->request == NULL)
    {
        qDebug() << "REQ is NULL!";
    }
    else if(MSG_IS_REQUEST(event->request))
    {
        qDebug() << "\tREQ" << "sm" << event->request->sip_method << "rf" << event->request->reason_phrase;
        return;
    }

    if(event->response == NULL)
    {
        qDebug() << "\tRES is NULL!";
        return;
    }
    if(MSG_IS_RESPONSE(event->request))
    {
            qDebug() << "RES, for" << event->response->cseq->method;
            return;
    }

}


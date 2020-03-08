/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "dialog.h"

// called by:
// UDP when input SIP event comes from ftsip: Event != E_NULL
// Dialog when clicked addr book: Event = E_NULL, Command only one = EH_DA
// Dialog when clicked call submenu (conf, hold, term...): Event = E_NULL, Command
void Dialog::eventChain(QString peer, e_sip command, e_sip event, QString header, QString body, QString parm)
{
    chain_peer = peer;
    chain_command = command;
    chain_event = event;
    chain_header = header;
    chain_body = body;
    chain_parm = parm;

    //qDebug() << "Dialog::eventChain" << "peer, command, event, header, body, parm"
      //       << chain_peer << chain_command << chain_event << chain_header << chain_body << chain_parm;

    call = calls->getCallByPeer(chain_peer, "Dialog::eventChain1", false);
    if(call == NULL)
    {
        // no current calls and DA clicked - start INVITE
        chain_disp = D_OFF;
        calls->addCall(chain_peer);
        // callView.resizeColumnsToContents();
        call = calls->getCallByPeer(chain_peer, "Dialog::eventChain2");
        Q_ASSERT(call);
    }
    else
    {
        // call exists - retrieve status of the call
        Q_ASSERT(call);
        chain_disp = call->model->status;
    }


    /*
    ****************
    Outgoing radio call: connect to GW
    ****************
    */

    if(HMI_PROCESS(D_OFF,EH_RADIO,EC_CALL_RADIO,D_HOST_DOWN_RADIO,
                   "D_OFF > EH_RADIO > EC_CALL_RADIO > D_HOST_DOWN_RADIO")) return;

    if(SIP_PROCESS(D_HOST_DOWN_RADIO,EM_TRYING,D_TRYING_RADIO,
                   "D_HOST_DOWN_RADIO > EM_TRYING > D_TRYING_RADIO")) return;
    if(SIP_PROCESS(D_TRYING_RADIO,EM_200_OK,D_CALL_OUT,
                   "D_TRYING_RADIO > EM_200_OK > D_CALL_OUT")) return;
    // reinvite case - invite in exist radio session
    if(HMI_PROCESS(D_CALL_OUT,EH_RADIO,EC_CALL_RADIO,D_HOST_DOWN_RADIO,
                   "D_CALL_OUT > EH_RADIO > EC_CALL_RADIO > D_HOST_DOWN_RADIO")) return;
    // reinvite after host down
    if(HMI_PROCESS(D_HOST_DOWN_RADIO,EH_RADIO,EC_CALL_RADIO,D_HOST_DOWN_RADIO,
                   "D_HOST_DOWN_RADIO > EH_RADIO > EC_CALL_RADIO > D_HOST_DOWN_RADIO")) return;

    /*
    ****************
    Outgoing call: plain or +to focus
    ****************
    */

    if(HMI_PROCESS(D_OFF,EH_DA,EC_CALL,D_HOST_DOWN,"D_OFF > EH_DA > EC_CALL > D_HOST_DOWN")) return;

    if(SIP_PROCESS(D_HOST_DOWN,EM_TRYING,D_TRYING_IN, "D_HOST_DOWN > EM_TRYING > D_TRYING_IN")) return;
    if(SIP_PROCESS(D_TRYING_IN,EM_RINGING,D_BLINK_OUT, "D_TRYING_IN > EM_RINGING > D_BLINK_OUT"))
    {
        call->model->addAction(EH_TERM, "Terminate");
        return;
    }
    if(SIP_PROCESS(D_BLINK_OUT,EM_200_OK,D_CALL_OUT, "D_BLINK_OUT > EM_200_OK > D_CALL_OUT"))
    {
        // outgoing call established
        if(chain_peer == conf_uri)
        {
            //we call focus, we should subscribe to it as initiator only at first time
            chain_disp = call->model->status;
            chain_header = "confevent";
            chain_event = E_NULL;
            HMI_PROCESS(D_CALL_OUT,E_NULL,EC_SUBSCRIBE,D_SUBS_CONF_INIT_TRYING,
                        "conf_uri: D_CALL_OUT > E_NULL > EC_SUBSCRIBE > D_SUBS_CONF_INIT_TRYING Conf initiator subscribes to focus messages...");
        }
        else
        {
            // menu for plain outgoing call
            call->model->addAction(EH_CONF_START, "Start conf");
        }
        return;
    }

    /*
     ****************
     Finish of initiator to focus subscribing
     ****************
    */

    if(SIP_PROCESS(D_SUBS_CONF_INIT_TRYING,EM_SUBSCRIBE_OK,D_SUBS_CONF_INIT_OK,
                   "D_SUBS_CONF_INIT_TRYING > EM_SUBSCRIBE_OK > D_SUBS_CONF_INIT_OK"))
    {
        // focus auto sent notify after subscribing without us - no need use EC_NOTIFY command - it is not present in command list
        return;
    }
    if(SIP_PROCESS(D_SUBS_CONF_INIT_OK,EM_NOTIFY_CONF,D_INITIATOR2FOCUS_COMPLETE,
                   "D_SUBS_CONF_INIT_OK > EM_NOTIFY_CONF > D_INITIATOR2FOCUS_COMPLETE"))
    {
        // finished contact to focus at first time as conf initiator; now make focus to turn peer from us to focus
        // this NOTIFY informs us that we are first and alone conf member
        qDebug() << "\tWe are conf initiator and receive NOTIFY from focus that we are first member:" << chain_body;
        // refer
        chain_disp = call->model->status;
        chain_command = EH_CONF_NEXT;
        chain_event = E_NULL;
        chain_header = chain_refer;
        chain_parm = chain_replaces;
        chain_body = "";
        call->model->addAction(EH_CONF_LEAVE, "Leave conf");
        // start inviting 2-nd member by initiator
        HMI_PROCESS(D_INITIATOR2FOCUS_COMPLETE,EH_CONF_NEXT,EC_REFER,D_REFER_TRYING,
                    "D_INITIATOR2FOCUS_COMPLETE > EH_CONF_NEXT > EC_REFER > D_REFER_TRYING");
        calls->setGlobalMode(GLOBAL_CONF_INITIATOR);
        return;
     }

    /*
     ****************
     Ingoing call: plain or +from focus
     ****************
    */

    if(SIP_PROCESS(D_OFF,EM_INVITE,D_TRYING_OUT, "D_OFF > EM_INVITE > D_TRYING_OUT"))
    {
        // if incoming invite, we have to send ringing automaticly - operator doesn't do that from hmi
        chain_disp = call->model->status;
        chain_event = E_NULL;
        if(HMI_PROCESS(D_TRYING_OUT,E_NULL,EC_RINGING,D_BLINK_IN,"D_TRYING_OUT > E_NULL > EC_RINGING > D_BLINK_IN Auto ringing"))
        {
            if(chain_header == "isfocus")
            {
                // INVITE comes from focus - we should send auto answer, parse replace header and terminate other connection
                chain_disp = call->model->status;
                calls->setGlobalMode(GLOBAL_CONF_MEMBER);
                if(!HMI_PROCESS(D_BLINK_IN,E_NULL,EC_200_OK,D_BLINK_IN,
                                "isfocus: D_BLINK_IN > E_NULL > EC_200_OK > D_BLINK_IN Auto answer to focus")) return;
                QString replaces = chain_parm;
                if(replaces != "")
                {
                    // drop current call with conf initiator - we don't have to do it if focus invited us not as 1-st member after initiator
                    chain_peer = replaces.section(';', 0, 0);
                    chain_disp = call->model->status;
                    if(!HMI_PROCESS(D_BLINK_IN,E_NULL,EC_TERM_IN,D_BLINK_IN,
                                    "isfocus: D_BLINK_IN > E_NULL > EC_TERM_IN > D_BLINK_IN We invited to conf by focus with replaces and drop initiator")) return;
                }
            }
            else
            {
                call->model->addAction(EH_ANSWER, "Answer");
                call->model->addAction(EH_BUSY, "Drop call");
            }
        }
        return;
    }
    if(HMI_PROCESS(D_BLINK_IN,EH_DA,EC_200_OK,D_BLINK_IN,"D_BLINK_IN > EH_DA > EC_200_OK > D_BLINK_IN")) return;   // session will establish only after ACK coming, still blinking
    if(SIP_PROCESS(D_BLINK_IN,EM_ACK,D_CALL_IN, "D_BLINK_IN > EM_ACK > D_CALL_IN"))
    {
        // call established. if we called by focus - invite to conf, we should subscribe to it
        // this subscribe repeats every time when focus invite to conf
        if(chain_header == "isfocus")
        {
            chain_disp = call->model->status;
            chain_event = E_NULL;
            chain_header = "confevent";
            HMI_PROCESS(D_CALL_IN,E_NULL,EC_SUBSCRIBE,D_SUBS_CONF_NEXT_TRYING,"isfocus: D_CALL_IN > E_NULL > EC_SUBSCRIBE > D_SUBS_CONF_NEXT_TRYING Invited by focus peer subscribes to focus messages...");
        }
        else
        {
            // menu for plain ingoing call
            call->model->deleteAction(EH_BUSY);
            call->model->deleteAction(EH_ANSWER);
            call->model->addAction(EH_TERM, "Terminate");
        }
        return;
    }


    /*
     ****************
     Finish of next member to focus subscribing
     ****************
    */

    if(SIP_PROCESS(D_SUBS_CONF_NEXT_TRYING,EM_SUBSCRIBE_OK,D_SUBS_CONF_NEXT_OK, "D_SUBS_CONF_NEXT_TRYING > EM_SUBSCRIBE_OK > D_SUBS_CONF_NEXT_OK"))
    {
        // focus auto sent notify after subscribing without us - no need use EC_NOTIFY command - it is not present in command list
        return;
    }
    if(SIP_PROCESS(D_SUBS_CONF_NEXT_OK,EM_NOTIFY_CONF,D_CALL_IN, "D_SUBS_CONF_NEXT_OK > EM_NOTIFY_CONF > D_CALL_IN"))
    {
        // we are next member and subscribes to focus
        // this NOTIFY iforms us that we are next member
        qDebug() << "\tNOTIFY for new member, all members in conf:" << chain_body;
        call->model->parseNotify(chain_body);
        call->model->addAction(EH_CONF_LEAVE, "Leave conf");
        return;
    }


    /*
     ****************
     Conf next - REFER, called every time when we invite next member
     ****************
    */

    // we are conf initiator and stay here after inviting previous members. Now, we will invite next member
    if(HMI_PROCESS(D_NOTIFY_CONF_OK,EH_CONF_NEXT,EC_REFER,D_REFER_TRYING,"D_NOTIFY_CONF_OK > EH_CONF_NEXT > EC_REFER > D_REFER_TRYING")) return;
    if(SIP_PROCESS(D_REFER_TRYING,EM_REFER_ACCEPT,D_REFER_ACCEPT, "D_REFER_TRYING > EM_REFER_ACCEPT > D_REFER_ACCEPT"))
    {
        // subscribe
        chain_disp = call->model->status;
        chain_event = E_NULL;
        chain_header = "refer";
        HMI_PROCESS(D_REFER_ACCEPT,E_NULL,EC_SUBSCRIBE,D_SUBS_REFER_TRYING,
                    "conf_uri: D_REFER_ACCEPT > E_NULL > EC_SUBSCRIBE > D_SUBS_REFER_TRYING Conf initiator subscribes to focus REFER messages...");
        return;
    }
    if(SIP_PROCESS(D_SUBS_REFER_TRYING,EM_SUBSCRIBE_OK,D_SUBS_REFER_OK, "D_SUBS_REFER_TRYING > EM_SUBSCRIBE_OK > D_SUBS_REFER_OK")) return;
    if(SIP_PROCESS(D_SUBS_REFER_OK,EM_NOTIFY_REFER_100,D_REFER_PROCEED, "D_SUBS_REFER_OK > EM_NOTIFY_REFER_100 > D_REFER_PROCEED"))
    {
        qDebug() << "\tREFER NOTIFY 100 Trying:" << chain_body;
        return;
    }

    if(SIP_PROCESS(D_REFER_PROCEED,EM_NOTIFY_REFER_200,D_NOTIFY_CONF_OK, "D_REFER_PROCEED > EM_NOTIFY_REFER_200 > D_NOTIFY_CONF_OK"))
    {
        // now make focus to turn peer from us to focus
        // this NOTIFY iforms us that we are first and alone conf member
        call->model->deleteAction(EH_CONF_START);
        call->model->deleteAction(EH_TERM);
        qDebug() << "\tREFER NOTIFY 200 OK:" << chain_body;
        return;
    }


    /*
     ****************
     Leave conf
     ****************
    */
    if(HMI_PROCESS(D_NOTIFY_CONF_OK,EH_CONF_LEAVE,EC_UNSUBSCRIBE,D_UNSUBS_CONF_INITIATOR_TRYING,
                   "D_NOTIFY_CONF_OK > EH_CONF_LEAVE > EC_UNSUBSCRIBE > D_UNSUBS_CONF_INITIATOR_TRYING"))
    {
        // conf initiator wanted leave conf and click "Leave conf"
        // return global mode from conf mode to plain mode
        // no matter menu status because this call will be closed anyway after leaving
        calls->setGlobalMode(GLOBAL_IDLE);
        return;
    }
    if(HMI_PROCESS(D_CALL_IN,EH_CONF_LEAVE,EC_UNSUBSCRIBE,D_UNSUBS_CONF_MEMBER_TRYING,
                   "D_CALL_IN > EH_CONF_LEAVE > EC_UNSUBSCRIBE > D_UNSUBS_CONF_MEMBER_TRYING"))
    {
        // conf member wanted leave conf and click "Leave conf"
        // no matter menu status because this call will be closed anyway after leaving
        calls->setGlobalMode(GLOBAL_IDLE);
        return;
    }
    // ToDo: unsubscribe, obtain notify without left member


    /*
     ****************
     TERM or RELEASE
     ****************
    */

    if(HMI_PROCESS(D_CALL_IN,EH_DA,EC_TERM_IN,D_OFF,"D_CALL_IN > EH_DA > EC_TERM_IN > D_OFF")) return;
    if(HMI_PROCESS(D_CALL_OUT,EH_DA,EC_TERM_OUT,D_OFF,"D_CALL_OUT > EH_DA > EC_TERM_OUT > D_OFF")) return;
    if(HMI_PROCESS(D_CALL_IN,EH_TERM,EC_TERM_IN,D_OFF,"D_CALL_IN > EH_TERM > EC_TERM_IN > D_OFF")) return;
    if(HMI_PROCESS(D_CALL_OUT,EH_TERM,EC_TERM_OUT,D_OFF,"D_CALL_OUT > EH_TERM > EC_TERM_OUT > D_OFF")) return;
    // term conf initiator to focus session after initiator unsubscribing
    if(SIP_PROCESS(D_UNSUBS_CONF_INITIATOR_TRYING,EM_SUBSCRIBE_OK,D_OFF,
                   "D_UNSUBS_CONF_INITIATOR_TRYING > EM_SUBSCRIBE_OK > D_OFF"))
    {
        // conf initiator unsubscribed from focus and is going to terminate call to focus
        chain_event = E_NULL;
        chain_disp = D_OFF;
        HMI_PROCESS(D_OFF,E_NULL,EC_TERM_OUT,D_OFF,"D_OFF > EH_NULL > EC_TERM_OUT > D_OFF");
        calls->setGlobalMode(GLOBAL_IDLE);
        return;
    }
    // term focus to member session after focus unsubscribing
    if(SIP_PROCESS(D_UNSUBS_CONF_MEMBER_TRYING,EM_SUBSCRIBE_OK,D_OFF,
                   "D_UNSUBS_CONF_MEMBER_TRYING > EM_SUBSCRIBE_OK > D_OFF"))
    {
        // conf member unsubscribed from focus and is going to terminate call from focus
        chain_event = E_NULL;
        chain_disp = D_OFF;
        HMI_PROCESS(D_OFF,E_NULL,EC_TERM_IN,D_OFF,"D_OFF > EH_NULL > EC_TERM_IN > D_OFF");
        calls->setGlobalMode(GLOBAL_IDLE);
        return;
    }
    if(chain_event == EM_TERM)
    {
        log(QString("CATCH_SIP TERM [%1]").arg(chain_peer), LOG_ON);
        bookModel.setStatus(chain_peer, D_OFF);
        calls->deleteCall(chain_peer);
        return;
    }

    /*
     ****************
     At least - process notifies about other members
     ****************
    */

    if(chain_event == EM_NOTIFY_CONF)
    {
        log(QString("CATCH_SIP NOTIFY [%1] %2").arg(chain_peer).arg(chain_body), LOG_ON);
        call->model->parseNotify(chain_body);
        if(call->model->membersCount() == 0)
        {
            chain_event = E_NULL;
            if(HMI_PROCESS(D_CALL_IN,E_NULL,EC_UNSUBSCRIBE,D_UNSUBS_CONF_MEMBER_TRYING,
                           "D_CALL_IN > E_NULL > EC_UNSUBSCRIBE > D_UNSUBS_CONF_MEMBER_TRYING"))
            {
                // conf member stays alone in conf - leave
                // no matter menu status because this call will be closed anyway after leaving
                log("CATCH_SIP We becomes alone conf member - others left conf. Unsubscribe from focus", LOG_ON);
                calls->setGlobalMode(GLOBAL_IDLE);
            }
        }
        return;
    }

    /*
     ****************
     Unknown event
     ****************
    */

    if(chain_event != E_NULL)
        log(QString("CATCH_SIP Didn't catch SIP event peer:%1 event:%2 disp=%3").arg(chain_peer).arg(chain_event).arg(chain_disp), LOG_WARN);
    else
        log(QString("CATCH_HMI Didn't catch HMI command peer:%1 command:%2 disp=%3").arg(chain_peer).arg(command).arg(chain_disp), LOG_WARN);
    return;
}

void Dialog::hmiRadioEvent(int key)
{
    // radio staff
    // here key = current_freq
    // radio OFF, Rx or Tx
    QString peer;

    // fill model variables for selected freq
    radioModel.setLevel(R_DISABLED);
    radioModel.setQuery(Radio.arg(current_freq));
    e_freq fmode = freqModel.getMode(key);
    // qDebug() << "Radio Rx mode is:" << fmode;
    if(fmode == F_OFF)
    {
        log(QString("Dialog::hmiEvent: fmode for freq=%1 is F_OFF, why was not disabled in freqView?").arg(key), LOG_WARN);
        return;
    }
    // ToDo: in dialog.cpp after set Rx or RxTx mode test if at least one radio with this mode is present

    // PASS 1 - define isRx, isTx
    bool isRx = false;
    bool isTx = false;
    for (int i=0; i < radioModel.getNumVars(); ++i)
    {
        if(radioModel.getVar(i, COL_RADIO_ISRX).toInt() == 1)
            isRx = true;
        if(radioModel.getVar(i, COL_RADIO_ISTX).toInt() == 1)
            isTx = true;
    }

    // now, if isRx is true, then at least one radio has Rx capability
    // if isTx is true, at least one radio has Tx capability

    // PASS 2: test conditions after PASS 1
    if(!isRx)
    {
        log(QString("Dialog::hmiEvent: for freq=%1 no one Rx is present, frequency shoud be disabled!").arg(key), LOG_WARN);
        return;
    }
    if(fmode == F_RXTX && !isTx)
    {
        log(QString("Dialog::hmiEvent: freq=%1 has Tx capability, but no one Tx radio is present!").arg(key), LOG_WARN);
        return;
    }
    // conditions is OK, send INVITE for each radio
    QString rmode = "disabled";
    for (int i=0; i < radioModel.getNumVars(); ++i)
    {
        isRx = false;
        isTx = false;
        if(radioModel.getVar(i, COL_RADIO_ISRX).toInt() == 1)
            isRx = true;
        if(radioModel.getVar(i, COL_RADIO_ISTX).toInt() == 1)
            isTx = true;
        if(fmode == F_RX && isRx)
            rmode = "recvonly";
        if(fmode == F_RXTX && isRx && !isTx)
            rmode = "recvonly";
        if(fmode == F_RXTX && !isRx && isTx)
            rmode = "sendonly";
        if(fmode == F_RXTX && isRx && isTx)
            rmode = "sendrecv";
        peer = radioModel.getVar(i, COL_RADIO_URI).toString();
        // ToDo: RADIO/COUPLING, RSSI, "123.000"
        QString radio;
        if(radioModel.getVar(i, COL_RADIO_ISCOUPLED).toInt() == 0)
            radio = "radio";
        else
            radio = "coupling";
        QString sn = radioModel.getVar(i, COL_RADIO_SNMETHOD).toString();
        QString freq = radioModel.getVar(i, COL_RADIO_FREQ).toString();
        QString radio_parm = QString("%1:%2:%3:%4").arg(rmode).arg(radio).arg(sn).arg(freq);
        // log(QString("radio_parm=%1").arg(radio_parm), LOG_OTHER);
        //writeDatagramSip(EC_CALL_RADIO, peer, "", radio_parm, "Call Radio");
        eventChain(peer, EH_RADIO, E_NULL, "", "", radio_parm);
    }
}

bool Dialog::HMI_PROCESS(e_disp disp_check, e_sip command_check, e_sip hmi, e_disp disp_new, QString text)
{
    if(chain_event != E_NULL)
        return false;
    // qDebug() << disp_check << chain_disp << command_check << chain_command;
    if(disp_check == chain_disp && command_check == chain_command)
    {
        log(QString("CATCH_HMI %1 [%2]").arg(text).arg(chain_peer), LOG_ON);
        bookModel.setStatus(chain_peer, disp_new);
        call->model->setStatus(disp_new);
        writeDatagramSip(hmi, chain_peer, chain_header, chain_parm, text);
        return true;
    }
    return false;
}

bool Dialog::SIP_PROCESS(e_disp disp_check, e_sip event_check, e_disp disp_new, QString text)
{
    //qDebug() << "disp_check:" << disp_check << "event_check:" << event_check << "disp:" << disp << "peer:" << peer;
    if(event_check == chain_event && disp_check == chain_disp)
    {
        log(QString("CATCH_SIP %1 [%2]").arg(text).arg(chain_peer), LOG_ON);
        bookModel.setStatus(chain_peer, disp_new);
        call->model->setStatus(disp_new);
        return true;
    }
    return false;
}



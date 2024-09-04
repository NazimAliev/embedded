/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <QDebug>
#include <netdb.h>
#include <QProcess>

#include "csengine.h"
#include "console.h"

Csengine::Csengine(QObject *parent) :
	QObject(parent)
{
	thread = new(QThread);
	cevent = new(CEvent);
	rid = 0;
}

Csengine::~Csengine()
{
	delete thread;
	delete cevent;
	sendRegister(m_contact_ip, 0, 0);
	eXosip_quit();
}


/******************************
  ACTIVE FROM side - we are caller
 *****************************/

// call by console or udp - start new outcoming call
int Csengine::sendInvite(QString peer, e_session_type session_type, QString header, QString parm)
{
	// parm may be "recvonly:radio:RSSI:123.000" for radio session or
	// "isfocus" if invite sent from conference focus

	// ***** NEW OUT SESSION *****

	osip_message_t* invite;
	int res;
	session_t* session = NULL;

	// check if we already have session for this peer - in this case we should reinvite
	session = getSessionByTo(peer, false);
	if(session != NULL)
	{
		// invalid session: duplicated invite for the same peer?
		if(!session->valid)
		{
			log(QString("Csengine::sendInvite: duplicated invite for host [%1]").arg(peer), LOG_WARN);
			return -1;
		}
		// session exists - reinvite
		return sendReInvite(session->did, parm);
	}

	QString s_to = QString("sip:%1").arg(peer);
	QString s_from;
	if(header == "isfocus")
		s_from = QString("sip:%1").arg(m_conf_uri);
	else
		s_from = QString("sip:%1").arg(m_uri);

	int cid = -1;

	res = eXosip_call_build_initial_invite(&invite,
			s_to.toAscii().data(),
			s_from.toAscii().data(),
			NULL, "Fonoteka call");
	if (res != 0)
	{
		// TRYING not received - host unreachable
		log(QString("Csengine::sendInvite: host %1 unavailable").arg(peer), LOG_ERR);
		return cid;
	}

	osip_message_set_supported(invite, "100rel");
	osip_message_set_supported(invite, "replaces");
	if(header != "")
		osip_message_set_header (invite, "Event", header.toAscii().data());

	// parm is replaces
	if(session_type == STYPE_PLAIN && parm != "")
		osip_message_set_header (invite, "Replaces", parm.toAscii().data());

	/* add sdp body */
	session_t fake_session; // only to retrieve local_audio_ip and local_audio_port from buildSdpBody

	QString sdp_body;
	if(session_type == STYPE_RADIO)
		sdp_body = buildSdpBody(m_radioBodyInvList, session_type, m_port_audio, parm, &fake_session);
	else
		sdp_body = buildSdpBody(m_plainBodyInvList, session_type, m_port_audio, "", &fake_session);

	osip_message_set_body(invite, sdp_body.toAscii().data(), sdp_body.size());
	osip_message_set_content_type(invite, "application/sdp");

	eXosip_lock ();
	cid = eXosip_call_send_initial_invite(invite); // посылка INVITE вернула нам cid
	eXosip_unlock ();
	if (cid <= 0)
	{
		log("send Invite failed", LOG_ERR);
		eXosip_unlock();
		return cid;
	}

	// check if we already have sessions with the same from and to - kill it
	for(int i=0; i<slist.count(); ++i)
		if(QString("sip:%1").arg(slist.at(i)->addr_from) == s_from and QString("sip:%1").arg(slist.at(i)->addr_to) == s_to)
			slist.removeAt(i);
	// пока знаем только cid исходящей сессии,
	// tid, did получим после того как придет Trying
	session = new(session_t);
	session->valid = false;
	session->type = session_type;
	session->sdp_type = SDP_OFFER;
	session->cid = cid;
	session->s_did = -1;
	session->addr_from = m_uri;
	session->addr_to = peer;
	session->local_sdp_pack = m_plainBodyInvPack;
	session->local_audio_ip = fake_session.local_audio_ip;
	session->local_audio_port = fake_session.local_audio_port;
	session->attr_list = fake_session.attr_list;
	session->log = "send invite";
	slist.append(session);
	res = eXosip_call_set_reference(cid, session);
	if(res != 0)
	{
		log(QString("sendInvite: can't set reference for cid=%1").arg(cid), LOG_ERR);
		return res;
	}
	log(QString(">> INVITE cid=%1 %2 > %3 header:%4 parm:%5").arg(cid).arg(s_from).arg(s_to).arg(header).arg(parm), LOG_SIP);

	return cid;
}

int Csengine::sendReInvite(int did, QString parm)
{
	int res;
	osip_message_t* invite;
	res = eXosip_call_build_request(did, "INVITE", &invite);

	if(res != 0)
	{
		log(QString("build reinvite did=%1 fails").arg(did), LOG_ERR);
		return res;
	}

	session_t* session = NULL;
	session = getSessionByDid(did);
	if(session == NULL)
		return -1;

	/* add sdp body */
	session_t fake_session; // only to retrieve local_audio_ip and local_audio_port from buildSdpBody
	// FIXME: only radio may be reinvite?
	QString sdp_body = buildSdpBody(m_radioBodyInvList, session->type, m_port_audio, parm, &fake_session);

	osip_message_set_body(invite, sdp_body.toAscii().data(), sdp_body.size());
	osip_message_set_content_type(invite, "application/sdp");
	eXosip_lock();
	res = eXosip_call_send_request(did, invite);
	eXosip_unlock();
	if (res != 0)
	{
		log(QString("send reinvite did=%1 fails").arg(did), LOG_ERR);
		return res;
	}
	// refresh body - may changed by freq panel (change recvonly -> sendrecv)
	session->local_sdp_pack = m_plainBodyInvPack;
	session->local_audio_ip = fake_session.local_audio_ip;
	session->local_audio_port = fake_session.local_audio_port;
	session->attr_list = fake_session.attr_list;
	session->log = "send reinvite";
	log(QString(">> REINVITE did=%1").arg(did), LOG_SIP);
	return 0;
}

int Csengine::sendSubscribe(QString peer, QString header)
{
	osip_message_t* subscribe;
	session_t* session;
	int res;

	// "WG67 KEY-IN"
	QString s_to = QString("sip:%1").arg(peer);
	QString s_from = QString("sip:%1").arg(m_uri);

	res = eXosip_subscribe_build_initial_request(&subscribe,
			s_to.toAscii().data(),
			s_from.toAscii().data(),
			NULL, header.toAscii().data(), 600);
	if (res != 0)
	{
		log(QString("sendSubscribe: build subscribe failed, peer:%1 header:%2").arg(peer).arg(header), LOG_ERR);
		return res;
	}

	eXosip_lock ();
	res = eXosip_subscribe_send_initial_request(subscribe);
	// ToDo: subscribe is outside current dialog and creates dialog. res = s_did
	eXosip_unlock ();

	// create session for did, subscribe dialog is independed from media session (cid=0 for subscribe)
	// we need this session for unsubscribe and will destroy it after unsubscribing
	session = new(session_t);
	// prevent finding this subscribe session as plain session
	session->cid = 0;

	// temporary store session->s_did, session->did will obtain after proceeding
	session->s_did = res;
	session->addr_from = m_uri;
	session->addr_to = peer;
	session->event = header;
	session->log = QString("send subscribe %1").arg(header);
	slist.append(session);
	// will obtain real s_did, tid after subscribe proceeding
	log(QString(">> SUBSCRIBE %1 s_did=%2 header:%3").arg(peer).arg(res).arg(header), LOG_SIP);

	// ToDo: refresh and finish subscribe
	// eXosip_subscribe_build_refresh_request(s_did, subscribe);
	// eXosip_subscribe_send_refresh_request(s_did, subscribe);
	// eXosip_subscribe_remove(s_did)
	return res;
}

int Csengine::sendUnsubscribe(QString peer)
{
	osip_message_t* subscribe;
	session_t* session = NULL;
	int did;
	int res;

	// for conf initiator, it is caller, focus is callee, so addr_from is initiator and addr_to is focus
	// for invited member, it is callee, focus is caller, so addr_from is focus and addr_to is member
	session = getSessionSubs();
	if(session == NULL)
		return -1;

	did = session->did;
	if(did < 0)
	{
		log(QString("sendUnsubcribe: no subscribe did found for peer=[%1]").arg(peer), LOG_WARN);
		return -1;
	}

	res = eXosip_subscribe_build_refresh_request(did, &subscribe);
	if (res != 0)
	{
		// TRYING not received - host unreachable
		log(QString("build unsubscribe failed, peer:%1").arg(peer), LOG_ERR);
		return res;
	}
	osip_message_set_expires(subscribe, 0);
	osip_message_set_header(subscribe, "Event", session->event.toAscii().data());
	eXosip_lock ();
	res = eXosip_subscribe_send_refresh_request(did, subscribe);
	eXosip_unlock ();

	/*
	   res = eXosip_subscribe_remove(did);
	   if (res != 0)
	   {
	// TRYING not received - host unreachable
	log(QString("send unsubscribe failed, peer:%1").arg(peer), LOG_ERR);
	return res;
	}
	 */
	slist.removeAll(session);

	log(QString(">> UNSUBSCRIBE and remove subs session cid=%1 did=%2 tid=%3 header:%4").
			arg(session->cid).arg(session->did).arg(session->last_tid).arg(session->event), LOG_SIP);
	return res;
}

// call by event processing after EXOSIP_CALL_ANSWERED (200 OK) input message
void Csengine::sendAck(int did)
{
	osip_message_t *ack;
	int res;

	eXosip_lock();
	res = eXosip_call_build_ack(did, &ack);

	if (res != 0)
	{
		log("build ack failed", LOG_ERR);
		eXosip_unlock();
		return;
	}
	res = eXosip_call_send_ack(did, ack);
	eXosip_unlock();

	if (res != 0)
	{
		log("send ack failed", LOG_ERR);
	}
	else
	{
		log(QString(">> ACK did=%1").arg(did), LOG_SIP);
		runRtp(did, TRUE);
	}
}

void Csengine::sendRefer(QString peer, QString ref, QString replaces)
{
	//sendReferOutsideCall(peer, ref);
	//return;
	// send REFER inside the dialog
	osip_message_t* refer;
	session_t* session = NULL;
	int did;
	// this should return did:
	//int eXosip_call_find_by_replaces(char* replaces)

	session = getSessionByTo(peer);
	if(session == NULL)
		return;
	did = session->did;

	QString sref = QString("sip:%1").arg(ref);
	int res = eXosip_call_build_refer(did, sref.toAscii().data(), &refer);
	if (res != 0)
	{
		// error build refer
		log(QString("Build refer failed peer:%1 refer:%2 did=%3").arg(peer).arg(ref).arg(did), LOG_ERR);
		return;
	}

	QString srep = QString("%1;%2").arg(replaces.section('/', 0, 0)).arg(replaces.section('/', 1, 1));
	if(replaces != "")
	{
		res = osip_message_set_header(refer, "Replaces", srep.toAscii().data());
		if (res != 0)
		{
			// error build header
			log("Build refer replaces header failed", LOG_ERR);
			return;
		}
	}
	eXosip_lock ();
	res = eXosip_call_send_request(did, refer);
	eXosip_unlock ();
	if (res != 0)
	{
		// error send refer
		log("send refer", LOG_ERR);
		return;
	}
	log(QString(">> REFER did=%1 Refer-to: %2 Replaces: %3").arg(did).arg(sref).arg(srep), LOG_SIP);
	// Doesn't refer work as subscribe and we have to send subcribe? RFC requeries that. Notify won't work wihout that.
	// sendSubscribe(peer, "refer");
}

void Csengine::sendReferOutsideCall(QString peer, QString ref)
{
	/*
	   osip_message_t *info;
	   char dtmf_body[1000];
	   int i;
	   eXosip_lock ();
	   i = eXosip_call_build_info (ca->did, &info);
	   if (i == 0)
	   {
	   snprintf (dtmf_body, 999, "Signal=%c\r\nDuration=250\r\n", c);
	   osip_message_set_content_type (info, "application/dtmf-relay");
	   osip_message_set_body (info, dtmf_body, strlen (dtmf_body));
	   i = eXosip_call_send_request (ca->did, info);


	   int eXosip_call_build_notify	(	int 	did,
	   int 	subscription_status,
	   osip_message_t ** 	request
	   )

	   int eXosip_call_send_request	(	int 	did,
	   osip_message_t * 	request
	   )

	 */
	// send REFER
	osip_message_t *refer;
	int res;
	QString s_to = QString("sip:%1").arg(peer);
	QString s_from = QString("sip:%1").arg(m_uri);

	res = eXosip_refer_build_request(&refer, QString("sip:%1").arg(ref).toAscii().data(), s_from.toAscii().data(), s_to.toAscii().data(), NULL);
	if (res != 0)
	{
		// error build refer
		log("build refer", LOG_ERR);
		return;
	}

	QString replaces = QString("sip:%1;sip:%2").arg(m_uri).arg(ref);
	res = osip_message_set_header (refer, "Replaces", replaces.toAscii().data());
	if (res != 0)
	{
		// error build header
		log("build header", LOG_ERR);
		return;
	}

	eXosip_lock ();
	res = eXosip_refer_send_request(refer);
	eXosip_unlock ();
	if (res != 0)
	{
		// error send refer
		log("send refer", LOG_ERR);
		return;
	}
}


/******************************
  PASSIVE TO side - we are callee
 *****************************/

// call by event processing after EXOSIP_CALL_INVITE input message
void Csengine::sendRinging(QString peer)
{
	session_t* session = NULL;
	int tid;

	session = getSessionByFrom(peer);
	if(session == NULL)
		return;
	tid = session->last_tid;

	eXosip_lock();
	eXosip_call_send_answer (tid, 180, NULL);
	eXosip_unlock();
	log(QString(">> RINGING tid=%1").arg(tid), LOG_SIP);
}

// call by console or udp - answer codeOk=200 OK or other to incoming call
// default value of codeOk is 200
void Csengine::sendAnswer(QString peer, e_session_type session_type, QString header, QString parm, int codeOk)
{
	// sending codeOk
	osip_message_t *answer = NULL;
	int res;
	session_t* session = NULL;
	int tid;

	session = getSessionByFrom(peer);
	if(session == NULL)
		return;
	tid = session->last_tid;

	eXosip_lock();
	res = eXosip_call_build_answer(tid, codeOk, &answer);

	/*
	// set contact header
	// all future requests from peer will be addressed to contact ip

	osip_contact_t* contact;
	res = osip_message_get_contact(answer, 0, &contact);
	if (res != 0)
	{
	log("sendAnswer: osip_message_get_contact failed", LOG_ERR);
	}
	osip_uri_t* uri = osip_contact_get_url(contact);
	uri->host = m_contact_ip.toAscii().data();
	uri->port = QString("%1").arg(m_contact_port).toAscii().data();

	osip_contact_set_url(contact, uri);
	 */
	/*
	   example of contact manupulation
	   fprintf(stderr, "Contact host=%s\n", contact->uri->host);
	   osip_contact_to_str (contact, &str);
	   fprintf(stderr, "Contact: %s\n", str);
	 */
	//osip_uri_t* uri = osip_contact_get_url(contact);
	//strcat(uri->host, ";isfocus");
	//osip_contact_set_url(contact, uri);
	//    osip_contact_param_add(contact, osip_strdup("isfocus"), osip_strdup(""));

	if (res != 0)
	{
		eXosip_call_send_answer (tid, 400, NULL);
		eXosip_unlock();
		log(QString("Build answer failed, sent 400 tid=%1").arg(tid), LOG_WARN);
		return;
	}
	osip_message_set_supported(answer, "100rel");

	/* add sdp body */
	// TODO: pass as function parameter after processing local capabilities
	QString sdp_body;
	if(session_type == STYPE_RADIO)
	{
		sdp_body = buildSdpBody(m_radioBodyInvList, session_type, m_port_audio, parm, session);
		session->local_sdp_pack = m_plainBodyAnsPack;
	}
	else
	{
		sdp_body = buildSdpBody(m_plainBodyInvList, session_type, m_port_audio, "", session);
		session->local_sdp_pack = m_radioBodyAnsPack;
	}
	//QString sdp_body = buildSdpBody(session_type, m_port_audio, parm, session);

	osip_message_set_body(answer, sdp_body.toAscii().data(), sdp_body.size());
	osip_message_set_content_type(answer, "application/sdp");
	if(header != "")
		osip_message_set_header (answer, "Event", header.toAscii().data());
	eXosip_call_send_answer(tid, codeOk, answer);
	log(QString(">> ANSWER %1 tid=%2 header:%3 parm:%4").arg(codeOk).arg(tid).arg(header).arg(parm), LOG_SIP);
	eXosip_unlock();
}

int Csengine::sendNotify(QString peer, QString header, QString body, int did, bool term)
{
	int res;
	QString ss_log;

	if(did == -1)
	{
		session_t* session = getSessionByTo(peer);
		if(session == NULL)
			return -1;
		did = session->did;
	}

	osip_message_t* notify_request;
	if(!term)
	{
		res = eXosip_insubscription_build_notify(did, EXOSIP_SUBCRSTATE_ACTIVE, 0, &notify_request);
		ss_log = "ss:active";
	}
	else
	{
		res = eXosip_insubscription_build_notify(did, EXOSIP_SUBCRSTATE_TERMINATED, NORESOURCE, &notify_request);
		ss_log = "ss:terminated";
	}

	if (res != 0)
	{
		log(QString("Build notify failed, did=%1").arg(did), LOG_ERR);
		return res;
	}
	osip_message_set_header (notify_request, "Event", header.toAscii().data());
	res = osip_message_set_body (notify_request, body.toAscii().data(), body.size());
	if (res != 0)
	{
		log(QString("Set notify body failed, did=%1").arg(did), LOG_ERR);
		return res;
	}
	if((GW && header == "WG67 KEY-IN") || header =="confevent")
		osip_message_set_content_type(notify_request, "text/plain");
	else
		osip_message_set_content_type(notify_request, "message/sipfrag;version=2.0");
	eXosip_lock();
	eXosip_insubscription_send_request(did, notify_request);
	eXosip_unlock();
	log(QString(">> NOTIFY did=%1 event:%2 body:%3 %4").
			arg(did).arg(header).arg(body.remove("\n\r")).arg(ss_log), LOG_SIP);
	return res;
}

void Csengine::sendSubscribeAnswer(int tid, int codeOk)
{
	osip_message_t* subscribe_answer;
	int res;
	// send subscribe OK
	res = eXosip_insubscription_build_answer(tid,  codeOk, &subscribe_answer);
	if (res != 0)
	{
		log(QString("Build subscribe answer failed, tid=%1").arg(tid), LOG_ERR);
		return;
	}
	eXosip_lock();
	eXosip_insubscription_send_answer (tid, codeOk, subscribe_answer);
	eXosip_unlock();
	log(QString(">> SUBSCRIBE 200 OK tid=%2").arg(tid), LOG_SIP);
}

void Csengine::sendNotifyAnswer(int tid, int codeOk)
{
	osip_message_t* notify_answer;
	int res;
	// send Notify OK
	res = eXosip_insubscription_build_answer(tid,  codeOk, &notify_answer);
	if (res != 0)
	{
		log(QString("build notify answer failed, tid=%1").arg(tid), LOG_ERR);
		return;
	}
	eXosip_lock();
	eXosip_insubscription_send_answer (tid, codeOk, notify_answer);
	eXosip_unlock();
}

void Csengine::sendOptions(QString peer)
{
	osip_message_t *options;
	int res;

	eXosip_lock();
	res = eXosip_options_build_request(&options, peer.toAscii().data(), m_uri.toAscii().data(), NULL);

	if (res != 0)
	{
		log("build options failed", LOG_ERR);
		eXosip_unlock();
		return;
	}
	res = eXosip_options_send_request(options);
	eXosip_unlock();

	if (res != 0)
	{
		log("send options failed", LOG_ERR);
		return;
	}
	log(QString(">> OPTIONS: peer [%1]").arg(peer), LOG_SIP);
}

void Csengine::sendPing(QString peer)
{
	osip_message_t *ping;
	int res;

	eXosip_lock();
	res = eXosip_message_build_request(&ping, QString("PING").toAscii().data(), peer.toAscii().data(), m_uri.toAscii().data(), NULL);

	if (res != 0)
	{
		log("build ping failed", LOG_ERR);
		eXosip_unlock();
		return;
	}
	res = eXosip_message_send_request(ping);
	eXosip_unlock();

	if (res != 0)
	{
		log("send ping failed", LOG_ERR);
		return;
	}
	log(QString(">> PING: peer [%1]").arg(peer), LOG_SIP);
}

int Csengine::sendRegister(QString contactIp, int contactPort, int expires)
{
	int res;
	const char* from = "sip:test@5.231.68.122";
	const char* proxy = "sip:5.231.68.122";
	QString contact = "sip:";
	contact += contactIp;
	contact += QString(":%1").arg(contactPort);
	osip_message_t* reg;

	if(expires == 0)
	{
		res = eXosip_register_build_register(rid, expires, &reg);
		if(res != 0)
		{
			log(QString("sendRegister: build register update fails res=%1").arg(res), LOG_ERR);
			return res;
		}
		log(QString(">> REGISTER REMOVE rid=%1").arg(rid), LOG_SIP);
	}
	else
	{
		rid = eXosip_register_build_initial_register(from, proxy, contact.toAscii().data(), expires, &reg);
		if(rid <= 0)
		{
			log(QString("sendRegister: build register fails rid=%1").arg(rid), LOG_ERR);
			return rid;
		}
		log(QString(">> REGISTER with contact [%2:%3]").arg(contactIp).arg(contactPort), LOG_SIP);
	}

	eXosip_lock();
	res = eXosip_register_send_register(rid, reg);
	eXosip_unlock();

	if(res != 0)
	{
		log(QString("proxy register fails res=%1").arg(res), LOG_ERR);
		return res;
	}

	if(expires == 0)
		eXosip_register_remove(rid);

	return 0;
}

/******************************
  END of sides
 *****************************/

// call by console or udp - terminate call cid
void Csengine::sendTerminate(QString peer, bool asCaller)
{
	session_t* session = NULL;
	int cid;
	int did;

	// ToDo: how terminate radio session? STYPE_RADIO
	// two types of termination may be - as caller and as callee
	if(asCaller)
		// we was a initiator of this session and peer addr is "to"
		session = getSessionByTo(peer);
	else
		// peer was a initiator of this session and peer addr is "from"
		session = getSessionByFrom(peer);

	if(session == NULL)
		return;
	cid = session->cid;
	did = session->did;

	eXosip_lock();
	eXosip_call_terminate(cid, did);
	eXosip_unlock();

	// TODO
	// rtp session will stop after receive CALL_MESSAGE_ANSWERED: BYE
	// maybe here is better? timeout need if message not get
	//runRtp(cid, FALSE);
	log(QString(">> BYE (CANCEL, DECLINE) cid=%1 did=%2").arg(cid).arg(did), LOG_SIP);
}

// call by event processing after EXOSIP_CALL_CLOSED or EXOSIP_CALL_MESSAGE_NEW(BYE) input message
// answer to BYE
void Csengine::sendOk(int tid, int codeOk)
{
	// answer to BYE
	osip_message_t *answer = NULL;

	eXosip_lock();
	eXosip_call_build_answer (tid, codeOk, &answer);
	eXosip_call_send_answer (tid, codeOk, answer);
	eXosip_unlock();

	log(QString(">> %1 OK tid=%2").arg(codeOk).arg(tid), LOG_SIP);
}

session_t* Csengine::getSessionByCid(int cid, bool warn)
{
	session_t* session;
	for (int i=0; i < slist.size(); ++i)
	{
		session = slist.at(i);
		//qDebug() << "getSessionByCid: from:" << session->addr_from << "to:" << session->addr_to;
		if (cid == session->cid)
			return session;
	}
	if(warn)
	{
		log(QString("No found such cid=%1 in slist").arg(cid), LOG_WARN);
		listSessions();
	}
	return NULL;
}

session_t* Csengine::getSessionByDid(int did, bool warn)
{
	session_t* session;
	for (int i=0; i < slist.size(); ++i)
	{
		session = slist.at(i);
		//qDebug() << "getSessionByDid: from:" << session->addr_from << "to:" << session->addr_to << "did:" << session->did;
		if (did == session->did)
			return session;
	}
	if(warn)
	{
		log(QString("No found such did=%1 in slist").arg(did), LOG_WARN);
		listSessions();
	}
	return NULL;
}

session_t* Csengine::getSessionBySid(int s_did, bool warn)
{
	session_t* session;
	for (int i=0; i < slist.size(); ++i)
	{
		session = slist.at(i);
		if (s_did == session->s_did)
			return session;
	}
	if(warn)
	{
		log(QString("No found such s_did=%1 in slist").arg(s_did), LOG_WARN);
		listSessions();
	}
	return NULL;
}

session_t* Csengine::getSessionByFrom(QString addr_from, bool warn)
{
	session_t* session;
	for (int i=0; i < slist.size(); ++i)
	{
		session = slist.at(i);
		//qDebug() << "getSessionByFrom:" << session->addr_from;
		if (session->sdp_type == SDP_ANSWER && addr_from == session->addr_from)
			return session;
	}
	if(warn)
	{
		log(QString("No found such addr_from=%1 in slist").arg(addr_from), LOG_WARN);
		listSessions();
	}
	return NULL;
}

session_t* Csengine::getSessionByTo(QString addr_to, bool warn)
{
	session_t* session;
	for (int i=0; i < slist.size(); ++i)
	{
		session = slist.at(i);
		// qDebug() << "getSessionByTo:" << session->addr_to;
		if (addr_to == session->addr_to)
			return session;
	}
	if(warn)
	{
		log(QString("No found such addr_to=%1 in slist").arg(addr_to), LOG_WARN);
		listSessions();
	}
	return NULL;
}

// uses in sendUnsubscribe to find subscribe session, not plain session
session_t* Csengine::getSessionSubs(bool warn)
{
	session_t* session;
	for (int i=0; i < slist.size(); ++i)
	{
		session = slist.at(i);
		if (session->addr_from == m_uri && session->cid == 0)
			return session;
	}
	if(warn)
	{
		log("No found subscribe in slist", LOG_WARN);
		listSessions();
	}
	return NULL;
}


int Csengine::listSessions(bool print)
{
	int sessions = slist.count();
	if(sessions == 0)
	{
		log("listSessions: slist is empty", LOG_OTHER);
		return 0;
	}
	if(!print)
		return sessions;

	session_t* session;
	log("addr_from\t\t\taddr_to\t\t\t\tcid\tdid\ttid\ts_did\tlog", LOG_OTHER);
	log("------------------------------------------------------------------------", LOG_OTHER);
	for (int i=0; i < sessions; ++i)
	{
		session = slist.at(i);
		log(QString("%1\t\t%2\t\t%3\t%4\t%5\t%6\t%7").
				arg(session->addr_from).
				arg(session->addr_to).
				arg(session->cid).
				arg(session->did).
				arg(session->last_tid).
				arg(session->s_did).
				arg(session->log), LOG_OTHER);
	}
	return sessions;
}

int Csengine::initConfig(QString listen_ip, QString contact_ip, int listen_port, QString uri,
		QString ip_dsp_board, int port_audio, QString conf_uri,
		QString rmode, QString type, QString snMethod, QString freq)
{
	m_listen_ip = listen_ip;
	m_contact_ip = contact_ip;
	if(m_listen_ip != m_contact_ip)
	{
		log(QString("listen IP [%1] and contact IP [%2] are differ").arg(m_listen_ip).arg(m_contact_ip), LOG_WARN);
	}
	// ToDo: contact port separate in DB
	m_contact_port = listen_port;
	m_uri = uri;
	m_ip_dsp_board = ip_dsp_board;
	m_port_audio = port_audio;
	m_conf_uri = conf_uri;
	m_rmode = rmode;
	m_type = type;
	m_snMethod = snMethod;
	m_freq = freq;

	TRACE_INITIALIZE (INFO1, stderr);
	if (eXosip_init())
	{
		log("eXosip_init failed", LOG_ERR);
		return (1);
	}

	QSqlQuery query("SELECT key,uri,proxy,gateway,expires,username,userid,realm FROM proxy");
	bool ok = query.first();
	Q_ASSERT(ok);

	/*
	   key|uri                   |proxy        |gateway    |expires|username|userid  |realm
	   0  |sip:nazim.ru@sipnet.ru|sip:sipnet.ru|192.168.1.1|3600   |nazim.ru|nazim.ru|etc.tario.ru
	 */

	proxy_uri = query.value(1).toString();
	proxy_proxy = query.value(2).toString();
	proxy_gw = query.value(3).toString();
	proxy_expires = query.value(4).toInt();
	proxy_username = query.value(5).toString();
	proxy_userid = query.value(6).toString();
	proxy_realm = query.value(7).toString();
	char address[24];
	eXosip_guess_localip(AF_INET, address, 24);
	QString qaddress(address);
	log(QString("local IP=%1").arg(qaddress), LOG_OTHER);
	if(m_listen_ip != qaddress)
	{
		// in this case, even masquarading turn on, INVITE VIA header will include wrong local IP, not listen IP
		// as a result, ASK message will be directed to wrong local IP addr and peer wont receive it
		log(QString("listen IP [%1] and local IP [%2] are differ, try to fix local IP").arg(m_listen_ip).arg(qaddress), LOG_WARN);
		QByteArray ba = m_listen_ip.toLatin1();
		const char *c_str = ba.data();
		eXosip_set_option(EXOSIP_OPT_SET_IPV4_FOR_GATEWAY, c_str);
		eXosip_guess_localip(AF_INET, address, 24);
		log(QString("Now local IP=%1").arg(address), LOG_OTHER);
	}

	const char* username = "test";
	const char* userid = "test";
	const char* realm = "5.231.68.122";
	int res;

	// to avoid auto changing contact_ip to real ip addr
	//eXosip_masquerade_contact(contact_ip.toAscii().data(), listen_port);

	res = eXosip_listen_addr(IPPROTO_UDP, m_listen_ip.toAscii().data(), listen_port, AF_INET, 0);
	if (res != 0)
	{
		eXosip_quit();
		log(QString("Could not initialize transport layer with ip:port=%1:%2").
				arg(QString(m_listen_ip)).arg(listen_port), LOG_ERR);
		return (1);
	}

	initSdp();

	connect(this, SIGNAL(signalStartWait()), cevent, SLOT(slotStartWait()));
	connect(cevent, SIGNAL(signalSipEvent(eXosip_event_t*)), this, SLOT(slotSipEvent(eXosip_event_t*)));

	cevent->moveToThread(thread);
	thread->start();
	Q_EMIT(signalStartWait());
	return 0;
}

QString Csengine::parseField(const QStringList &list, QString t)
{
	// find matched template string in the list
	// parsed string without prefix
	int i;
	QString res;
	QRegExp rx(QString("%1*").arg(t));
	//qDebug() << list << t;
	rx.setPatternSyntax(QRegExp::Wildcard);

	i = list.indexOf(rx);
	if(i == -1)
	{
		log(QString("Csengine::parseField(): template [%1] not found").arg(t), LOG_WARN);
		return "-1";
	}
	res = list.at(i);
	res.remove(t);
	return res;
}






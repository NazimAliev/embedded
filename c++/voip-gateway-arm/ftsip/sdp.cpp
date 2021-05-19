/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "csengine.h"
#include "console.h"

/*

SDP exchange algorithm

========================================================
initSdp(): prepare SDP body, included local audio ip:port

at first, sdp body gives as text string.
As a result sdp body store packed (hash and string lists structure) in sdp_pack_t struct:
typedef struct
{
    QHash<QString, QString> hash;
    QStringList media;
    QStringList payloads;
} sdp_pack_t;
    sdp_pack_t m_plainBodyInvPack;
    sdp_pack_t m_plainBodyAnsPack;
    sdp_pack_t m_radioBodyInvPack;
    sdp_pack_t m_radioBodyAnsPack;
packed sdp user as local caps to compare with input sdp from peer
Text string body uses to send sdp to peer in invite.

separate Inv and Ans messages allows test sdp with different set of payloads and codecs.
Of course, in final configuration sdp for Inv and Ans are the same.

================================================================
buildSdpBody(): build sdp body for invite and answer msgs in csengine.cpp

Function work with session_t session parms, where local and remote sdp body are stored.

Used 2 times for generate invite (plain and radio), in radio invite body includes parms.

Used 1 time for reinvine and 1 time for answer.
FIXME: only store port and ip in body and nothing more?

======================================================================
checkRemoteSdp(): parse peer sdp body in procevent.cpp (input sip events)

Used 1 time in call answered (when we sent invite to peer and get answer)
and 2 times in invite and reinvite (when we receive invite from peer)

sdp body parsed into sdp_pack_t struct:

Example sdp body and parsing:

v=0
o=fonoteka 17 2700 IN IP4 192.168.53.2
s=conversation
c=IN IP4 192.168.53.2
t=0 0
m=audio 7778 RTP/AVP 0 8 101
a=rtpmap:0 PCMU/8000/1
a=rtpmap:0 PCMU/8000
a=rtpmap:8 PCMA/8000/1
a=rtpmap:8 PCMA/8000
a=rtpmap:101 telephone-event/8000/1
a=rtpmap:101 telephone-event/8000

sdp_pack_t pack.media:
 ("audio", "7778", "RTP/AVP") 

sdp_pack_t pack.payloads:
"0
8
101" 

sdp_pack_t pack.hash:
(("addr", "192.168.53.2")("addrtype", "IP4")("nettype", "IN")("8", "PCMA/8000")("username", "fonoteka")("sid", "17")("s", "conversation")("0", "PCMU/8000")("v", "0")("101", "telephone-event/8000"))

pack.hash[pack.payloads[1]] = "telephone-event/8000";

=======================================================================
OLD COMMENTS:

we can ignore strings (not presented in our standard sdp) - this is a chance that peer will agree to remove its

receive INVITE: call checkRemoteSdp(SDP_OFFER) to check offer body from active side
checkRemoteSdp(SDP_OFFER) checking offer SDP body staff:
    v=: ignore
    o=: store received audio ip as remote audio addr in session object
    s=: ignore
    c=: compare to o=, if differ, show warning
    t=: ignore
    m=: store received audio port as remote audio port in session object and run a cycle to count all rtpmaps
        a=rtpmap: check in m= cicle. If GW - check radio rtpmaps; otherwise - plain rtpmaps.
        Logic:
        1. Retrieve media_number=atoi((char*)osip_list_get(&remote_media->m_payloads, pos)); pos++
        2. For each sdp media_number (0, 8, 101, ...) seek string a=rtpmap:media_number in xrtp_params with match=1 and
           check the string as passed
        3. In case unwanted media number - skip with warning: peer offers unsupported payload, ignored.
           In case wanted media number - compare with string. If not matched - error: wrong parameter %1 peer payload
        4. After cicle, if unchecked string present - error: peer unsupports our payload %1
        5. In case error, send 415 answer and returns false

answer sdp is m_xrtp

we are PASSIVE (CALLEE) ANSWER SIDE SENDS ANSWER TO INVITE

send standard sdp body

we are ACTIVE (CALLER) OFFER SIDE RECEIVES SDP OK FROM ANSWER SIDE

we can't ignore strings (not presented in our standard sdp) - this is final answer - error 415
Logic is the same as for above excluding in case unwanted media number we can't skeep - this is error

*/

// TODO rfc3264: if answer is differ from offer, o= should set differ value in answer

/*
	init local sdp body for 4th variants: invite/answer, plain/radio
	store sdp body in lists:
	m_localPlainInvPayloadsList;
    m_localPlainAnsPayloadsList;
    m_localRadioInvPayloadsList;
    m_localRadioAnsPayloadsList;
    m_localPlainInvCodecsList;
    m_localPlainAnsCodecsList;
    m_localRadioInvCodecsList;
    m_localRadioAnsCodecsList;
*/

void Csengine::initSdp()
{
	QByteArray array;
	char* buffer;
	sdp_message_t* sdp;
    // adaptation parameters
    // interval = 10 | 20 | 30
    // ptt_rep = 0-3
    // R2S-KeepAlivePeriod = 20-1000
    // R2S-KeepAliveMultiplier = 2-50

	m_plainBodyInvList
			<< "v=0\n"
			<< QString("o=fonoteka 17 2700 IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "s=conversation\n"
			<< QString("c=IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "t=0 0\n"
			<< "m=audio %1 RTP/AVP 0 8 101\n"
			<< "a=rtpmap:0 PCMU/8000/1\n"
			<< "a=rtpmap:0 PCMU/8000\n"
			<< "a=rtpmap:8 PCMA/8000/1\n"
			<< "a=rtpmap:8 PCMA/8000\n"
			<< "a=rtpmap:101 telephone-event/8000/1\n"
			<< "a=rtpmap:101 telephone-event/8000\n";

	array = m_plainBodyInvList.join("").toLocal8Bit();
	buffer = array.data();
	sdp_message_init(&sdp);
	sdp_message_parse(sdp, buffer);
	packSdp(sdp, &m_plainBodyInvPack);
    sdp_message_free(sdp);

	m_plainBodyAnsList
			<< "v=0\n"
			<< QString("o=fonoteka 17 2700 IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "s=conversation\n"
			<< QString("c=IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "t=0 0\n"
			<< "m=audio %1 RTP/AVP 0 8 101\n"
			<< "a=rtpmap:0 PCMU/8000/1\n"
			<< "a=rtpmap:0 PCMU/8000\n"
			<< "a=rtpmap:8 PCMA/8000/1\n"
			<< "a=rtpmap:8 PCMA/8000\n"
			<< "a=rtpmap:101 telephone-event/8000/1\n"
			<< "a=rtpmap:101 telephone-event/8000\n";

	array = m_plainBodyAnsList.join("").toLocal8Bit();
	buffer = array.data();
	sdp_message_init(&sdp);
	sdp_message_parse(sdp, buffer);
	packSdp(sdp, &m_plainBodyAnsPack);
    sdp_message_free(sdp);

	m_radioBodyInvList
			<< "v=0\n"
			<< QString("o=fonoteka 17 2700 IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "s=radio\n"
			<< QString("c=IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "t=0 0\n"
			<< "m=audio %1 RTP/AVP 96 97 98\n"
			<< "a=rtpmap:96 x-ptt-pcmu/8000\n"
			<< "a=rtpmap:97 x-ptt-pcma/8000\n"
			<< "a=rtpmap:98 x-ptt-g728/8000\n"
			<< QString("a=interval:%1\n").arg(20)
			<< QString("a=sigtime:%1\n").arg(1)
			<< QString("a=ptt_rep:%1\n").arg(0)
			<< QString("a=R2S-KeepAlivePeriod:%1\n").arg(200)
			<< QString("a=R2S-KeepAliveMultiplier:%1\n").arg(10)
			<< "a=mode:%2\n"
			<< "a=type:%3\n"
			<< "a=bss:%4\n"
			<< "a=fid:%5\n";

	array = m_radioBodyInvList.join("").toLocal8Bit();
	buffer = array.data();
	sdp_message_init(&sdp);
	sdp_message_parse(sdp, buffer);
	packSdp(sdp, &m_radioBodyInvPack);
    sdp_message_free(sdp);

	m_radioBodyAnsList
			<< "v=0\n"
			<< QString("o=fonoteka 17 2700 IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "s=radio\n"
			<< QString("c=IN IP4 %1\n").arg(m_ip_dsp_board)
			<< "t=0 0\n"
			<< "m=audio %1 RTP/AVP 96 97 98\n"
			<< "a=rtpmap:96 x-ptt-pcmu/8000\n"
			<< "a=rtpmap:97 x-ptt-pcma/8000\n"
			<< "a=rtpmap:98 x-ptt-g728/8000\n"
			<< QString("a=interval:%1\n").arg(20)
			<< QString("a=sigtime:%1\n").arg(1)
			<< QString("a=ptt_rep:%1\n").arg(0)
			<< QString("a=R2S-KeepAlivePeriod:%1\n").arg(200)
			<< QString("a=R2S-KeepAliveMultiplier:%1\n").arg(10)
			<< "a=mode:%2\n"
			<< "a=type:%3\n"
			<< "a=bss:%4\n"
			<< "a=fid:%5\n";

	array = m_radioBodyAnsList.join("").toLocal8Bit();
	buffer = array.data();
	sdp_message_init(&sdp);
	sdp_message_parse(sdp, buffer);
	packSdp(sdp, &m_radioBodyAnsPack);
    sdp_message_free(sdp);

}

QString Csengine::buildSdpBody(QStringList bodyList, e_session_type session_type, int port_audio, QString parm, session_t* session)
{
	QString body;
	if(session_type == STYPE_RADIO)
		body = m_radioBodyInvList.join("").arg(7778).arg(10).arg(20).arg(30).arg(40).arg(50).arg("MODE").arg("TYPE").arg("BSS").arg("FID");
	else
		body = m_plainBodyInvList.join("").arg(7778);
	return body;
}

// call after received INVITE (SDP_ANSWER) and received ANSWER (SDP_OFFER)
bool Csengine::checkRemoteSdp(int did, e_session_type session_type, e_sdp_type sdp_type)
{
    sdp_message_t* remote_sdp = eXosip_get_remote_sdp(did);
    if(remote_sdp == NULL)
    {
        log(QString("No remote SDP body found for did=%1").arg(did), LOG_WARN);
        return false;
    }

	/*
    char *sdp;
    sdp_message_to_str(remote_sdp, &sdp);
	qDebug() << "sdp body to str\n" << sdp;
	*/

	packSdp(remote_sdp, &m_pack);
    sdp_message_free(remote_sdp);
    return TRUE;
}

// pack sdp text body into sdp_pack_t structure
// used as send as receive packed sdp to simplify comparing regords using hash
bool Csengine::packSdp(sdp_message_t* sdp, sdp_pack_t* pack)
{
	pack->media.clear();
	pack->payloads.clear();

	pack->hash.clear();
	pack->hash["v"] = sdp_message_v_version_get(sdp);
	pack->hash["s"] = sdp_message_s_name_get(sdp);
	pack->hash["addr"] = sdp_message_o_addr_get(sdp);
	pack->hash["addrtype"] = sdp_message_o_addrtype_get(sdp);
	pack->hash["nettype"] = sdp_message_o_nettype_get(sdp);
	pack->hash["sid"] = sdp_message_o_sess_id_get(sdp);
	pack->hash["username"] = sdp_message_o_username_get(sdp);
    // update session object with local and remote ip:port
    QString remote_audio_ip = sdp_message_o_addr_get(sdp);

	QString remote_media = NULL;

	// scan remote payloads
	int pos = 0;
	int pos_pl = 0;
	int pos_a = 0;
	QString m_p;
	sdp_attribute_t* m_a;

	remote_media = sdp_message_m_media_get(sdp, pos);
	QString m_port = sdp_message_m_port_get(sdp, pos);
	QString m_proto = sdp_message_m_proto_get(sdp, pos);
	pack->media << remote_media << m_port << m_proto;
	//qDebug() << "media\n" << pack->media.join(" ");

	while ((m_p = sdp_message_m_payload_get(sdp, pos, pos_pl++)) != NULL)
	{
		pack->payloads << m_p;
	}
	//qDebug() << "payloads\n" <<  pack->payloads.join("\n");

	while ((m_a = sdp_message_attribute_get(sdp, pos, pos_a++)) != NULL)
	{
		// input like:
		// a=rtpmap:98 x-ptt-g728/8000
		// a=interval:20
		//field: "rtpmap" attr: "98 x-ptt-g728/8000" 
		//field: "interval" attr: "20"

		QString field = QString(m_a->a_att_field);
		QString attr = QString(m_a->a_att_value);
		if(field == "rtpmap")
		{
			QStringList rtpmap = QString(attr).split(" ");
			pack->hash[rtpmap[0]] = rtpmap[1];
		}
		else
		{
			pack->hash[field] = attr;
		}
	}
	//qDebug() << "Hash:\n" << pack->hash;

	pos++;
	remote_media = sdp_message_m_media_get(sdp, pos);
	if(remote_media != NULL)
	{
        log(QString("More than one media 'm=' in sdp body"), LOG_WARN);
		qDebug() << remote_media;
		char *str;
		sdp_message_to_str(sdp, &str);
		qDebug() << "sdp body to str\n" << str;
		qDebug() << "\n=======\n";
	}
	//QString remotePayload = (char*)osip_list_get(&remote_media->m_payloads, pos);
	//qDebug() << remotePayload;
	// TODO: dont free remote_media?
    // ToDo: match local and remote radio vars and params

	return TRUE;
}

void Csengine::sendError415(int tid)
{
    osip_message_t *answer = NULL;
    int res;

    eXosip_lock();
    res = eXosip_call_build_answer(tid, 415, &answer);
    if (res != 0)
    {
        log(QString("fail build 415 answer tid=%1").arg(tid), LOG_WARN);
    }
    else
    {
        eXosip_call_send_answer (tid, 415, NULL);
        log("sent answer 415", LOG_WARN);
    }
    eXosip_unlock();
}

void Csengine::printSdpPack(sdp_pack_t pack)
{
	qDebug() << "Pack media:\n" << pack.media;
	qDebug() << "Pack payloads:\n" << pack.payloads;
	qDebug() << "Pack hash:\n";
	// print hash
	QHashIterator<QString, QString> iter(pack.hash);
	while (iter.hasNext())
	{
		iter.next();
		qDebug() << iter.key() << "=>" << iter.value();
	}
}

/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef CSENGINE_H
#define CSENGINE_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <QStringList>

#include <eXosip2/eXosip.h>
#include <eXosip2/eX_call.h>
#include "udp.h"

#include "../../fthmi/trunk/intersip.h"

extern bool GW;
extern bool FOCUS;

typedef enum
{
    STYPE_PLAIN,
    STYPE_RADIO,
    STYPE_CUSTOM
} e_session_type;


typedef enum
{
    SDP_OFFER,
    SDP_ANSWER
} e_sdp_type;

typedef enum
{
    RECVONLY,
    SENDRECV,
    SENDONLY
} e_sdp_radio_mode;

typedef enum
{
    RADIO,
    COUPLING
} e_sdp_radio_type;

typedef enum
{
    RSSI,
    AGC,
    CN,
    PSD
} e_sdp_radio_bss;

typedef struct
{
    QString event;
    int did;
    int tid;
    QString peer;
    QString body;
    int ptt_id;
} subscribe_t;

typedef struct
{
	QHash<QString, QString> hash;
	QStringList media;
	QStringList payloads;
} sdp_pack_t;

// data associated with sip dialog
// sorce data for rtp commants to dsp
typedef struct
{
    bool valid;
    e_session_type type;    // STYPE_PLAIN or STYPE_RADIO
    e_sdp_type sdp_type;    // SDP_OFFER or SDP_ANSWER
    int cid;                // session ID. never changed?
    int did;                // actually, unique ID for the session
    int last_tid;           // last transaction. do we need store previos transactions?
    int s_did;                // subscribe dialog id

    QString addr_from;      // uri site that send INVITE
    QString addr_to;        // uri site than receive INVITE

    // if SDP_OFFER (active): ip:port to send RTP
    // if SDP_ANSWER (passive): ip from which expect receive RTP
    QString remote_audio_ip;
    QString local_audio_ip;
    int remote_audio_port;
    int local_audio_port;
    QString mask;
    sdp_pack_t local_sdp_pack;
    QString remote_sdp_pack;
    QString event;  // for subscribe

    // STYPE_PLAIN only data
    QString refer;
    QString replace_from;
    QString replace_to;

    // STYPE_RADIO only data
    int ptt_id;
    int ptt_type;
    QString attr_list;   // store radio attr mode:type:bss:fid that we sent, to compare after receive ANSWER with remote GW capabilities
    // local radio vars is fixed
    QString log;

} session_t;

// worker class for thread
class CEvent : public QObject
{
    Q_OBJECT
public:
    CEvent();
signals:
    void signalSipEvent(eXosip_event_t* event);

public slots:
    void slotStartWait();

protected:
};

class Csengine : public QObject
{
Q_OBJECT
public:
    explicit Csengine(QObject *parent = 0);
    ~Csengine();
    int initConfig(QString listen_ip, QString contact_ip, int listen_port, QString uri, QString ip_dsp_board, int port_audio,
                   QString conf_uri, QString rmode, QString type, QString snMethod, QString freq);
    int sendInvite(QString peer, e_session_type session_type, QString header, QString parm);
    void sendRinging(QString peer);
    void sendAnswer(QString peer, e_session_type session_type, QString header, QString parm, int codeOk=200);
    void sendRefer(QString peer, QString ref, QString replaces);
    void sendReferOutsideCall(QString peer, QString ref);
    int sendSubscribe(QString peer, QString header);
    int sendUnsubscribe(QString peer);
    int sendRegister(QString contactIp, int contactPort, int expires);
    void sendTerminate(QString peer, bool asCaller);

    QString parseField(const QStringList &list, QString t);

    QString proxy_uri;
    QString proxy_proxy;
    QString proxy_gw;
    int proxy_expires;
    QString proxy_username;
    QString proxy_userid;
    QString proxy_realm;

signals:
    void signalWriteAtomDatagram(e_sip sip, QString peer, QString header, QString body, QString parm, QString text);
    void signalWriteDspDatagram(QString cmd, QString local_ip, int local_port, QString remote_ip, int remote_port,
                                QString mask, bool sw, QString parm, QString text);
    void signalStartWait();
public slots:
    void slotSipEvent(eXosip_event_t* event);

private:
    int sendReInvite(int did, QString parm);
    void sendAck(int did);
    int sendNotify(QString peer, QString header, QString body, int did=-1, bool term=false);
    void sendSubscribeAnswer(int tid, int codeOk=200);
    void sendNotifyAnswer(int tid, int codeOk=200);
    void sendOptions(QString peer);
    void sendPing(QString peer);

    int registerProxy();
    int registerStun ();

    void initSdp();
    QString buildSdpBody(QStringList bodyList, e_session_type session_type, int port_audio, QString parm, session_t* session);
	bool checkRemoteSdp(int did, e_session_type session_type, e_sdp_type sdp_type);
	bool packSdp(sdp_message_t* sdp, sdp_pack_t* pack);
	void printSdpPack(sdp_pack_t pack);
    void sendError415(int tid);
    void sendOk(int tid, int codeOk=200);
    int checkSipEvent();
    session_t* getSessionByCid(int cid, bool warn=true);
    session_t* getSessionByDid(int did, bool warn=true);
    session_t* getSessionBySid(int s_did, bool warn=true);
    session_t* getSessionByFrom(QString addr_from, bool warn=true);
    session_t* getSessionByTo(QString addr_to, bool warn=true);
    session_t* getSessionSubs(bool warn=true);
    int listSessions(bool print=true);
    void runRtp(int id, bool start);
    void messageToDsp(int id, e_ortp command, session_t *session, bool start, QString parm, QString text);
    void messageToAtom(e_sip sip, QString peer, QString text);

    void procHeaders(eXosip_event_t *event);
    void procMessageNew(eXosip_event_t* event);
    bool procCallProceeding(eXosip_event_t* event);
    bool procSubsProceeding(eXosip_event_t* event);
    bool procCallAnswered(eXosip_event_t* event);
    bool procCallInvite(eXosip_event_t* event);
    bool procCallReInvite(eXosip_event_t* event);
    void procCallAck(eXosip_event_t* event);
    void procInSubscriptionNew(eXosip_event_t* event);
    void procByeSenderClosed(eXosip_event_t* event);
    void procByeReceiverClosed(eXosip_event_t* event);
    void procCallMessageNewRefer(eXosip_event_t* event);
    void procCallReleased(eXosip_event_t* event);
    void procRegisterProxy(eXosip_event_t* event);

    void extLog(eXosip_event_t *event);

    QString m_uri;

	QStringList m_plainBodyInvList;
	QStringList m_plainBodyAnsList;
	QStringList m_radioBodyInvList;
	QStringList m_radioBodyAnsList;

	sdp_pack_t m_plainBodyInvPack;
	sdp_pack_t m_plainBodyAnsPack;
	sdp_pack_t m_radioBodyInvPack;
	sdp_pack_t m_radioBodyAnsPack;

	sdp_pack_t m_pack;

    QString m_localRadioModes;    //mode,type,bss,fid
    QString m_listen_ip;
    QString m_contact_ip;
    int m_contact_port;
    QString m_ip_dsp_board;
    int m_port_audio;
    QString m_conf_uri;

    QString m_rmode;
    QString m_type;
    QString m_snMethod;
    QString m_freq;

    QThread* thread;
    CEvent* cevent;
    QList<session_t*> slist;
    QList<subscribe_t*> subscribeRadioList;
    QList<subscribe_t*> subscribeConfList;

    int rid;
    QString received;
    int rport;

    int index;
    QString parm;
    session_t* session;
    e_session_type session_type;
    void* reference;
    osip_uri_t* url_from;
    osip_uri_t* url_to;
    QString addr_from;
    QString addr_to;
    osip_header_t *referto_head;
    osip_header_t *replaces_head;
    QString header; // means: header_event
    osip_header_t* event_head;
    QString header_expires;
    osip_header_t* expires_head;
    QString header_ss;
    osip_header_t* ss_head;
    QString header_via;
    osip_header_t* via_head;
    char* str;
    size_t length;
    osip_body_t* body;
    QString sbody;
    osip_content_type_t* content_type;
    int notify_flag_did;

    Udp* udp;
};


#endif // CSENGINE_H

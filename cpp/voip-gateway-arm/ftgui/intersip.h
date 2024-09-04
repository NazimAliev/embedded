/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef INTERSIP_H
#define INTERSIP_H

 /*
  * this header describes interface between fthmi and ftsip.
  * header is included in fthmi and ftsip project
  */

// min values are defined by tables primary keys (started from 100, 200, etc...)
#define MIN_BOOK 100
#define NUM_BOOK 128
#define MAX_BOOK MIN_BOOK+NUM_BOOK
#define MIN_FREQ 200
#define NUM_FREQ 24
#define MAX_FREQ MIN_FREQ+NUM_FREQ
#define MIN_RADIO 300
#define NUM_RADIO 48
#define MAX_RADIO MIN_RADIO+NUM_RADIO
#define MIN_BSS 400
#define NUM_BSS 8
#define MAX_BSS MIN_BSS+NUM_BSS
#define MIN_CLI 500
#define NUM_CLI 8
#define MAX_CLI MIN_CLI+NUM_CLI

// book table columns
typedef enum
{
    BOOK_KEY, BOOK_REF_CONF, BOOK_NAME_TECH, BOOK_NAME_DISP, BOOK_URI,
    BOOK_IP_ATOM_BOARD, BOOK_IP_SIP_BOARD, BOOK_IP_DSP_BOARD,
    BOOK_IP_SIP_LISTEN, BOOK_IP_SIP_CONTACT,
    BOOK_PORT_SIP_LISTEN,
    BOOK_PORT_CONTROL_SIP_BOARD_SRC, BOOK_PORT_CONTROL_DSP_BOARD_SRC,
    BOOK_PORT_CONTROL_SIP_BOARD_DST, BOOK_PORT_CONTROL_DSP_BOARD_DST,
    BOOK_PORT_AUDIO,
    BOOK_TYPE
} e_init_book;

// radio table columns
typedef enum
{
    RADIO_ID, RADIO_REF_FREQ, RADIO_NAME, RADIO_URI,
    RADIO_IP_SIP_BOARD, RADIO_IP_DSP_BOARD,
    RADIO_IP_SIP_LISTEN, RADIO_IP_SIP_CONTACT,
    RADIO_PORT_SIP_LISTEN,
    RADIO_PORT_CONTROL_DSP_BOARD_SRC, RADIO_PORT_CONTROL_DSP_BOARD_DST,
    RADIO_PORT_AUDIO, RADIO_SN_METHOD,
    RADIO_ISRX, RADIO_ISTX, RADIO_ISCOUPLED, RADIO_ISON,
    FREQ_KEY, FREQ_FREQ, FREQ_NAME, FREQ_RMODE
} e_init_radio;

typedef enum
{
    E_NULL = 0,

    // commands from HMI

    // FROM side
    EH_DA = 100,  // click DA on addr book panel
    EH_IA,          // click IA on addr book panel
    EH_RADIO,       // click freq panel
    EH_TERM,
    EH_CONF_START,
    EH_CONF_NEXT,
    EH_HOLD,
    EH_DIV,
    // TO side
    EH_BUSY,
    EH_ANSWER,
    EH_200_OK,
    // FROM or TO side
    EH_CONF_LEAVE,

    // commands to SIP

    // FROM side
    EC_CALL = 200,
    EC_CALL_RADIO,
    EC_REFER,
    EC_TERM_OUT,
    // TO side
    EC_RINGING,
    EC_200_OK,
    EC_REJECT,
    EC_TERM_IN,
    // FROM or TO side
    EC_SUBSCRIBE,
    EC_UNSUBSCRIBE,

    // events (messages) from SIP

    // FROM side
    EM_TRYING,
    EM_BUSY,
    EM_RINGING,
    EM_200_OK,
    EM_NOTIFY_CONF,
    EM_NOTIFY_REFER_100,
    EM_NOTIFY_REFER_200,
    EM_NOTIFY_OK,
    EM_REFER_ACCEPT,
    EM_REJECT,
    // TO side
    EM_INVITE = 300,
    EM_INVITE_RADIO,
    EM_REINVITE,
    EM_ACK,
    EM_REFER,
    // FROM or TO side
    EM_SUBSCRIBE,
    EM_UNSUBSCRIBE,
    EM_SUBSCRIBE_OK,
    EM_TERM,
    EM_FOCUS_EMPTY,
    EM_ERROR
} e_sip;

// call status display
typedef enum
{
    D_OFF,
    D_HOST_DOWN,
    D_HOST_DOWN_RADIO,
    D_TRYING_IN,
    D_TRYING_OUT,   // internal status for ftgui, doesn't used by ftsip
    D_TRYING_RADIO,
    D_BLINK_IN,
    D_BLINK_OUT,
    D_CALL_IN,
    D_CALL_OUT,
    D_SUBS_REFER_TRYING,
    D_SUBS_CONF_INIT_TRYING,
    D_SUBS_CONF_NEXT_TRYING,
    D_SUBS_REFER_OK,
    D_SUBS_CONF_INIT_OK,
    D_INITIATOR2FOCUS_COMPLETE,
    D_SUBS_CONF_NEXT_OK,
    D_UNSUBS_CONF_INITIATOR_TRYING,
    D_UNSUBS_CONF_MEMBER_TRYING,
    D_NOTIFY_REFER_TRYING,
    D_NOTIFY_CONF_TRYING,
    D_NOTIFY_REFER_OK,
    D_NOTIFY_CONF_OK,
    D_REFER_TRYING,
    D_REFER_ACCEPT,
    D_REFER_PROCEED,
    D_TERM,
    D_HOLD,
    D_CONF,
    D_DIV_ON,
    D_DIV_OFF,
    D_CONF_INIT_COMPLETE,
    D_CONF_NEXT_COMPLETE
} e_disp;

// Output RTP commands to VoIP module
typedef enum
{
    R_CH_A2RTP,
    R_CH_RTP2A,
    R_CH_A2ERTP,
    R_CH_ERTP2A,
    R_CH_RX2ERTP,
    R_CH_ERTP2TX,
    R_CH_RTP2RTP,
    R_CH_MONITOR,
    R_CMD_RTPXHDR,
    R_DTMF2A,
    R_DTMF2RTP,
    R_TONE2A
} e_ortp;

typedef struct
{
    QString ip;
    int port;
} peer_t;

typedef struct
{
    peer_t addr;    // Host address and port of target RTP stream
    int ptt_type;   // Operator Position (Client) PTT type
    int squ;        // Squelch indication from the Radio
    int ptt_id;     // The PTT-ID of the transmitting device
    int sct;        // Simultaneous transmission
    int vf;         // Visibility flag
    int x;          // Extension present
    int ext_type;   // Extension type
    int ext_length; // Extension length in bits
    int ext_value;  // Extension value
} eheader_t;

typedef struct
{
    e_ortp ortp;
    int sw;
    peer_t addr;
    peer_t mix;
    QString mask;
    QString dtmf;
    QString tone;
    eheader_t* eheader;
} ertp_t;

#endif // INTERSIP_H

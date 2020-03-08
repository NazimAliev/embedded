/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include <ortp/stun.h>
#include "csengine.h"
#include "console.h"
#include "udp.h"


int Csengine::registerStun()
{
    StunAddress4 dest;
    StunAddress4 sAddr;
    StunAddress4 sMappedAddr;
    StunAddress4 sChangedAddr;
    bool ok;
    int res;

    ok = stunParseServerName("91.121.209.194", &dest);
    Q_ASSERT(ok);
    QByteArray ba = m_listen_ip.toLocal8Bit();
    const char* peer = ba.data();
    // can't set port=0 here: set to 5555 and will change to zero after this function
    ok = stunParseHostName(peer, &sAddr.addr, &sAddr.port, 5555);
    Q_ASSERT(ok);

    sAddr.port = 0; // will be random instead of 5555

    res = stunTest(&dest, 1, &sAddr, &sMappedAddr, &sChangedAddr);
    if (res != 0)
    {
        qDebug() << "Fail stunTest";
        return -1;
    }
    static char tmp[512];
    struct in_addr inaddr;
    char *atmp;

    inaddr.s_addr = htonl(sMappedAddr.addr);
    atmp = (char*)inet_ntoa(inaddr);

    snprintf(tmp, 512, "%s:%i", atmp, sMappedAddr.port);
    qDebug() << tmp;

    inaddr.s_addr = htonl(sChangedAddr.addr);
    atmp = (char*)inet_ntoa(inaddr);

    snprintf(tmp, 512, "%s:%i", atmp, sChangedAddr.port);
    qDebug() << tmp;

    //StunMessage message;
    //stunParseMessage((char*)data.constData(), data.size(), &message);

    return 0;
}



/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef INTERVOIP_H
#define INTERVOIP_H

// channel from audio to RTP, up to 8 channels
// if channels have the same IP its should be mixed to the IP
// example:
// CH_A2RTP;addr=192.168.1.5:7078;switch=1;mask=MIC,HS1,HS2;text=Open channel from all microphones to 192.168.1.5:7078
#define CH_A2RTP 0

// channel from RTP to audio, up to 8 channels
// if channels have the same audio interface its should be mixed to the audio interface
// example:
// CH_RTP2A;addr=192.168.1.5:7078;switch=1;mask=SPL,SPR,HSL,HSR;text=Open channel from RTP 192.168.1.5:7078 to all speakers and headsets
#define CH_RTP2A 1

// channel from audio to eRTP, up to 8 channels
// if channels have the same IP its should be mixed to the IP
// example:
// CH_A2ERTP;addr=192.168.1.5:7078;switch=1;mask=MIC,HS1,HS2;text=Open channel from all microphones to 192.168.1.5:7078
#define CH_A2ERTP 2

// channel from eRTP to audio, up to 8 channels
// if channels have the same audio interface its should be mixed to the audio interface
// example:
// CH_ERTP2A;addr=192.168.1.5:7078;switch=1;mask=SPL,SPR,HSL,HSR;text=Open channel from eRTP 192.168.1.5:7078 to all speakers and headsets
#define CH_ERTP2A 3

// channel from E&M interface to eRTP, up to 8 channels
// if channels have the same IP its should be mixed to the IP
// example:
// CH_EM2ERTP;addr=192.168.1.5:7078;switch=1;if=0;text=Open channel from E&M interface number 0 to 192.168.1.5:7078
#define CH_EM2ERTP 4

// channel from eRTP to E&M interface, up to 8 channels
// if channels have the same E&M interface its should be mixed to the E&M interface considering PTT priority
// example:
// CH_ERTP2EM;addr=192.168.1.5:7078;switch=1;if=7;text=Open channel from eRTP 192.168.1.5:7078 to E&M interface number 7
#define CH_ERTP2EM 5

#define CH_MONITOR 6

// mixer from RTPs to RTP
// example:
// CH_RTP2RTP;addr=192.168.1.5:7078;switch=1;mix=192.168.1.6:7078;text=add 192.168.1.5 to mixer
#define CH_RTP2RTP 7

#endif // INTERVOIP_H

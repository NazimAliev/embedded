#pragma once

void tm1637Init(void);
void tm1637DisplayDecimal(int v, int displaySeparator);
void tm1637DisplayTime(int m, int sep);
void tm1637DisplayCurrent(int c, int sep);
void tm1637DisplayTimeRun(int t, int sep);
void tm1637DisplayCurrentRun(int c, int sep);
void tm1637DisplaySepRun(int sep);
void tm1637DisplayBattery(int b, int sep);
void tm1637DisplayArray(unsigned char digitArr[]);
void tm1637SetBrightness(char brightness);

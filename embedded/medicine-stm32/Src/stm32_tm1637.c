#include "stm32f1xx_hal.h"

#include "stm32_tm1637.h"

void _tm1637Start(void);
void _tm1637Stop(void);
void _tm1637ReadResult(void);
void _tm1637WriteByte(unsigned char b);
void _tm1637DelayUsec(unsigned int i);
void _tm1637ClkHigh(void);
void _tm1637ClkLow(void);
void _tm1637DioHigh(void);
void _tm1637DioLow(void);

// Configuration.

#define CLK_PORT GPIOA
#define DIO_PORT GPIOA
#define CLK_PIN GPIO_PIN_2
#define DIO_PIN GPIO_PIN_3


const char segmentMap[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, // 0-7
    0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, // 8-9, A-F
    0x00
};


void tm1637Init(void)
{
    tm1637SetBrightness(8);
}

void tm1637DisplayDecimal(int v, int displaySeparator)
{
    unsigned char digitArr[4];
		for (int i = 0; i < 4; ++i) 
		{
			digitArr[i] = segmentMap[v % 10];
			if (i == 2 && displaySeparator) 
			{
				digitArr[i] |= 1 << 7;
			}
      v /= 10;
    }
	tm1637DisplayArray(digitArr);
}

void tm1637DisplayTime(int m, int sep)
{
	unsigned char digitArr[4];
	digitArr[0] = 0x04;	// i sign
	digitArr[1] = 0x54;	// m sign
	digitArr[2] = segmentMap[m % 10];
	m /= 10;
	digitArr[3] = segmentMap[m % 10];
	if (sep == 1) 
	{
		digitArr[2] |= 1 << 7;
	}
	tm1637DisplayArray(digitArr);
}

void tm1637DisplayCurrent(int c, int sep)
{
	unsigned char digitArr[4];
	digitArr[0] = 0x40;
	digitArr[1] = 0x40;
	digitArr[2] = segmentMap[c % 10];
	c /= 10;
	digitArr[3] = segmentMap[c % 10];
	if (sep == 1) 
	{
		digitArr[2] |= 1 << 7;
	}
	tm1637DisplayArray(digitArr);
}

void tm1637DisplayTimeRun(int t, int sep)
{
	unsigned char digitArr[4];
	int s;
	int m;
	s = t % 60;
	m = t / 60;
	digitArr[0] = segmentMap[s % 10];
	s /= 10;
	digitArr[1] = segmentMap[s % 10];
	digitArr[2] = segmentMap[m % 10];
	m /= 10;
	digitArr[3] = segmentMap[m % 10];
	if (sep == 1) 
	{
		digitArr[2] |= 1 << 7;
	}
	tm1637DisplayArray(digitArr);
}

void tm1637DisplayCurrentRun(int c, int sep)
{
	unsigned char digitArr[4];
	digitArr[0] = 0x40; 
	digitArr[1] = segmentMap[c % 10];
	c /= 10;
	digitArr[2] = segmentMap[c % 10];
	c /= 10;
	digitArr[3] = segmentMap[c % 10];
	if (sep == 1) 
	{
		digitArr[2] |= 1 << 7;
	}
	tm1637DisplayArray(digitArr);
}

void tm1637DisplaySepRun(int sep)
{
	unsigned char digitArr[4];
	digitArr[0] = 0x0;
	digitArr[1] = 0x0;
	digitArr[2] = 0x0;
	digitArr[3] = 0x0;
	if (sep == 1) 
	{
		digitArr[2] |= 1 << 7;
	}
	tm1637DisplayArray(digitArr);
}
void tm1637DisplayBattery(int b, int sep)
{
	unsigned char digitArr[4];
	switch(b)
	{
		case 0:
			digitArr[0] = 0x08;	
			break;
		case 1:
			digitArr[0] = 0x48;	
			break;
		case 2:
		default:
			digitArr[0] = 0x49;	
			break;
	}
	digitArr[1] = 0x00;
	digitArr[2] = 0x7c;
	digitArr[3] = 0x00;
	if (sep == 1) 
	{
		digitArr[2] |= 1 << 7;
	}
	tm1637DisplayArray(digitArr);
}

void tm1637DisplayArray(unsigned char digitArr[])
{
    _tm1637Start();
    _tm1637WriteByte(0x40);
    _tm1637ReadResult();
    _tm1637Stop();

    _tm1637Start();
    _tm1637WriteByte(0xc0);
    _tm1637ReadResult();

    for (int i = 0; i < 4; ++i) 
		{
        _tm1637WriteByte(digitArr[3 - i]);
        _tm1637ReadResult();
    }

    _tm1637Stop();
}

// Valid brightness values: 0 - 8.
// 0 = display off.
void tm1637SetBrightness(char brightness)
{
    // Brightness command:
    // 1000 0XXX = display off
    // 1000 1BBB = display on, brightness 0-7
    // X = don't care
    // B = brightness
    _tm1637Start();
    _tm1637WriteByte(0x87 + brightness);
    _tm1637ReadResult();
    _tm1637Stop();
}

void _tm1637Start(void)
{
    _tm1637ClkHigh();
    _tm1637DioHigh();
    _tm1637DelayUsec(2*5);
    _tm1637DioLow();
}

void _tm1637Stop(void)
{
    _tm1637ClkLow();
    _tm1637DelayUsec(2*5);
    _tm1637DioLow();
    _tm1637DelayUsec(2*5);
    _tm1637ClkHigh();
    _tm1637DelayUsec(2*5);
    _tm1637DioHigh();
}

void _tm1637ReadResult(void)
{
    _tm1637ClkLow();
    _tm1637DelayUsec(5*5);
    // while (dio); // We're cheating here and not actually reading back the response.
    _tm1637ClkHigh();
    _tm1637DelayUsec(2*5);
    _tm1637ClkLow();
}

void _tm1637WriteByte(unsigned char b)
{
    for (int i = 0; i < 8; ++i) 
		{
        _tm1637ClkLow();
        if (b & 0x01) 
				{
            _tm1637DioHigh();
        }
        else 
				{
            _tm1637DioLow();
        }
        _tm1637DelayUsec(3);
        b >>= 1;
        _tm1637ClkHigh();
        _tm1637DelayUsec(3);
    }
}

void _tm1637DelayUsec(unsigned int i)
{
    for (; i>0; i--) 
		{
        for (int j = 0; j < 10; ++j) 
				{
            __asm__ __volatile__("nop\n\t":::"memory");
        }
    }
}

void _tm1637ClkHigh(void)
{
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_SET);
}

void _tm1637ClkLow(void)
{
    HAL_GPIO_WritePin(CLK_PORT, CLK_PIN, GPIO_PIN_RESET);
}

void _tm1637DioHigh(void)
{
    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_SET);
}

void _tm1637DioLow(void)
{
    HAL_GPIO_WritePin(DIO_PORT, DIO_PIN, GPIO_PIN_RESET);
}

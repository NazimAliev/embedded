/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#include "main.h"
#include "stm32f1xx_hal.h"

#include "stm32_tm1637.h"
#define MAX_POS (4)

unsigned char d_auto[] = {0x5c,0x78,0x1c,0x77};
unsigned char d_ma[] = {0x77,0x54,0x0,0x0};
unsigned char d_sec[] = {0x10,0x54,0x00,0x0};
unsigned char d_bat[] = {0x78,0x77,0x7c,0x0};

// joystick toggle programs -> display, wait Set button, screensaver
// 1st stage programs: manual, battery level
int ion_get_program(uint32_t angle)
{
	// previous joystick ADC data
	static uint32_t angle_prev;
	// joystick position
	static int pos;
	int mode;

	mode = -1;

	if((angle >= angle_prev) && (angle - angle_prev) >= 20)
	{
		pos++; 
		if(pos >= MAX_POS)
		{
			pos = 0;
		}
		angle_prev = angle;
		ion_buzz(0);
	}
	if((angle < angle_prev) && (angle_prev - angle) >= 20)
	{
		pos--; 

		if(pos < 0)
		{
			pos = MAX_POS-1;
		}
		angle_prev = angle;
		ion_buzz(0);
	}

	// joystick was rotated by user
	switch(pos)
	{
		case 0:	
			{
				// show auto program 
				tm1637DisplayArray(d_auto);
				mode = ST_AUTO;
				break;
			}
		case 1:	
			{
				// show set current 
				tm1637DisplayArray(d_ma);
				mode = ST_CURRENT;
				break;
			}
		case 2:	
			{
				// show set time 
				tm1637DisplayArray(d_sec);
				mode = ST_TIME;
				break;
			}
		case 3: 
			{
				// show battery level
				tm1637DisplayArray(d_bat);
				mode = ST_BAT;
				break;
			}
		default:
		{
			ion_trace(-1, (uint8_t*)"ERROR: reach default\r\n");
			break;
		}
	}
	if(ion_flag_set == 1)
	{
		// set pressed
		ion_flag_set = 0;
		//ion_trace(mode, (uint8_t*)"Set in ion_get_program mode:\r\n");
		return mode;
	}
	else
	{
		return -1;
	};
} // ion_get_program

// Top level menu set current selected
// Select current or time for manual program
// Function called every while cycle time
// until data will be selected.
// if no data selected yet, function returns -1.
// if data was selected function returns data

int ion_set(uint32_t angle, bool mode)
{
	int ret;
	// joystick position
	if(mode)
	{
		// max angle 100 <-> max current 10mA 
		ret = angle / 10 + 1;
		tm1637DisplayCurrent(ret, 0);
	}
	else
	{
		// max angle 100 <-> max time 30m
		ret = (angle * 3) / 10 + 1;
		tm1637DisplayTime(ret, 0);
	}
	if(ion_flag_set == 1)
	{
		// set pressed
		ion_flag_set = 0;
		//ion_trace(ret, (uint8_t*)"Set in ion_set\r\n");
		ion_buzz(0);
	}
	else
	{
		ret = 0;
	}
	return ret;
}

// start current. 30V on, beep, joystick show current settings,
int ion_run(uint32_t angle)
{
	static uint32_t angle_prev;
	static bool flip_disp;
	static int sep;
	static char ss = 8;	// screensaver
	int mode;
	int disp;
	int diff;

	//ion_trace(-1, (uint8_t*)"Im in ion_run\r\n");
	
	mode = 0;
	disp = (int)(ion_adc_current * K_CURRENT_RD);
	diff = ion_selected_current - disp/10;
	if(abs(diff) > 2)
	{
		ion_buzz(1);
	}

	if(abs(angle - angle_prev) > 10)
	{
		angle_prev = angle;
		// set max brightness
		ss = 8;
		//TODO: monitor timer, show timer, screensaver

		flip_disp = !flip_disp;
	}
	else
	{
		if(ss > 1)
			ss--;
	}
	tm1637SetBrightness(ss);
	if(ss > 1)
	{
		if(flip_disp == true)
		{
			// disp 0.1mA step
			tm1637DisplayCurrentRun(disp, sep);
			//ion_trace(disp, (uint8_t*)"disp\r\n");
		}
		else
		{
			// timer 1s step
			tm1637DisplayTimeRun(ion_timer, sep);
			//ion_trace(sep, (uint8_t*)"Time Run\r\n");
		}
	}
	else
	{
		tm1637DisplaySepRun(sep);
	}
	sep = sep == 1 ? 0 : 1;
	if(ion_timer-- <= 0)
	{
		mode = -1;
		tm1637SetBrightness(8);
		ion_buzz(1);
	}
	HAL_Delay(400);
  return mode;
}
// return on stop or timer finished

// return on stop or timer finished

int ion_bat()
{
	// TODO: show battery level
	tm1637DisplayBattery(1, 1);
	HAL_Delay(500);
	//ion_trace(-1, (uint8_t*)"Im in ion_bat\r\n");
	return 0;
}

void ion_buzz(int mode)
{
	if(mode == 0)
	{
		// tick
		HAL_GPIO_WritePin(GPIOA, GPIO_Output_Buzz_Pin, GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA, GPIO_Output_Buzz_Pin, GPIO_PIN_RESET);
	}
	else
	{
		// tone
		for(int i = 0; i<50; i++)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_Output_Buzz_Pin, GPIO_PIN_SET);
			HAL_Delay(2);
			HAL_GPIO_WritePin(GPIOA, GPIO_Output_Buzz_Pin, GPIO_PIN_RESET);
			HAL_Delay(2);
		}
	}
}

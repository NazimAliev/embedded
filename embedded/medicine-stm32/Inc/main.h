/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <stdint.h> // to avoid 'unknown type uint16_t message'
#include <stdlib.h>
#include <stdbool.h>

#define MAX_CURRENT (10)
#define K_CURRENT_WR (0.36)
#define K_CURRENT_RD (0.0833)

void ion_trace(uint16_t value, uint8_t* str);
void ion_trace32(uint32_t value);
int ion_get_program(uint32_t angle);
int ion_set(uint32_t angle, bool mode);
int ion_run();
int ion_bat();
void ion_buzz(int tone);

extern uint32_t ion_adc_current;
extern uint32_t ion_adc_set;
extern uint16_t ion_duty_cycle;

int ion_selected_time;
int ion_selected_current;
extern int ion_timer;

extern bool ion_flag_set;
extern bool ion_flag_start;
extern bool ion_flag_stop;
extern bool ion_flag_tim2_busy;

typedef enum {
    ST_BEGIN,	// wait when program will selected by user
    ST_AUTO,	// user select program P1, wait start button in IDLE state 
    ST_CURRENT,	// 1st state manual program - set current
    ST_TIME,	// 2nd state manual program - set time
		ST_BAT,		// show battery status
    ST_RUN		// run, 30V on
	} ion_state_t;
extern ion_state_t ion_state;

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define ADC1_IN0_Joystic_Pin GPIO_PIN_0
#define ADC1_IN0_Joystic_GPIO_Port GPIOA
#define ADC_IN1_Shunt_Pin GPIO_PIN_1
#define ADC_IN1_Shunt_GPIO_Port GPIOA
#define GPIO_Output_CLK_Pin GPIO_PIN_2
#define GPIO_Output_CLK_GPIO_Port GPIOA
#define GPIO_Output_DIO_Pin GPIO_PIN_3
#define GPIO_Output_DIO_GPIO_Port GPIOA
#define GPIO_Input_Set_Pin GPIO_PIN_4
#define GPIO_Input_Set_GPIO_Port GPIOA
#define GPIO_Input_Set_EXTI_IRQn EXTI4_IRQn
#define GPIO_Input_Stop_Pin GPIO_PIN_5
#define GPIO_Input_Stop_GPIO_Port GPIOA
#define GPIO_Input_Stop_EXTI_IRQn EXTI9_5_IRQn
#define GPIO_Input_Start_Pin GPIO_PIN_6
#define GPIO_Input_Start_GPIO_Port GPIOA
#define GPIO_Input_Start_EXTI_IRQn EXTI9_5_IRQn
#define GPIO_Output_Buzz_Pin GPIO_PIN_7
#define GPIO_Output_Buzz_GPIO_Port GPIOA
#define GPIO_Output_30V_Pin GPIO_PIN_9
#define GPIO_Output_30V_GPIO_Port GPIOA

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

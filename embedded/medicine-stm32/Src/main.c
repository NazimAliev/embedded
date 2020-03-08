
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "stm32_tm1637.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                                

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void ion_pwm_setvalue(uint16_t value);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

// defined in main.h as extern

// shunt current
uint32_t ion_adc_current = 0;
// joystick data
uint32_t ion_adc_set = 0;
// max pwm value = preload (99 produced in Cube TIM1 conf)
uint16_t ion_duty_cycle = 0;
int ion_timer = 0;

bool ion_flag_set = false;
bool ion_flag_start = false;
bool ion_flag_stop = false;
bool ion_flag_tim2_busy = false;

ion_state_t ion_state = ST_BEGIN;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	ion_flag_set = 0;
	ion_flag_start = 0;
	ion_flag_stop = 0;
	//TODO: reset 30V, set display, beep
	ion_trace(-1, (uint8_t*)"Start");
	ion_trace(-1, (uint8_t*)"\r\n");
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	tm1637Init();
	HAL_GPIO_WritePin(GPIOA, GPIO_Output_30V_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_Output_Buzz_Pin, GPIO_PIN_RESET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	// ==============================================================
	/*
	prescaler = 8-1 / 1MHz
	counter period = 100-1 / 10kHz 100us
	PWM 100% = 100us
	*/
	int sep = 0;
	int mode;
	int angle;

	ion_state = 0;
	int default_time = 30;
	int default_current = 8;
	ion_selected_time = default_time;
	ion_selected_current = default_current;
	ion_duty_cycle = (int)((TIM1->ARR * ion_selected_current * K_CURRENT_WR) / MAX_CURRENT);
	ion_pwm_setvalue(ion_duty_cycle);
	
  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);

		// get ADC data: joystick and measered current
		HAL_ADC_Start(&hadc1);
		HAL_ADC_Start(&hadc2);
		HAL_ADC_PollForConversion(&hadc1, 100);
		HAL_ADC_PollForConversion(&hadc2, 100);
		ion_adc_set = HAL_ADC_GetValue(&hadc1);
		// full ADC range=4096, joystick range = 100
		angle = ion_adc_set/41; 
		ion_adc_current = HAL_ADC_GetValue(&hadc2);
		HAL_ADC_Stop(&hadc1);
		HAL_ADC_Stop(&hadc2);

		if(ion_flag_start == 1)
		{
			// start pressed
			ion_flag_start = 0;
			ion_state = ST_RUN;
			ion_timer = ion_selected_time * 60;
			HAL_GPIO_WritePin(GPIOA, GPIO_Output_30V_Pin, GPIO_PIN_SET);
			//ion_trace(-1, (uint8_t*)"START\r\n");
		};
		if(ion_flag_stop == 1)
		{
			// stop pressed	
			ion_flag_stop = 0;
			ion_state = ST_BEGIN;	
			HAL_GPIO_WritePin(GPIOA, GPIO_Output_30V_Pin, GPIO_PIN_RESET);
			tm1637SetBrightness(8);
			//ion_trace(-1, (uint8_t*)"STOP\r\n");
		};
		// --------------------------------------------
		// state machine
		switch(ion_state)
		{
			case ST_BEGIN:
				{
					// top level menu
					// show rotated joistick pos and return -1
					// Set pressed: return mode
					// no delay
					mode = ion_get_program(angle);
					if(mode != -1)
					{
						// ST_AUTO
						// ST_CURRENT
						// ST_TIME
						// ST_BAT
						ion_state = mode;
					}
					else
					{
						ion_state = ST_BEGIN;
					}

					break;
				}
			case ST_AUTO:
				{
					// program auto: fixed current and time
					ion_selected_current = default_current;
					ion_selected_time = default_time;
					ion_duty_cycle = (int)((TIM1->ARR * ion_selected_current * K_CURRENT_WR) / MAX_CURRENT);
					ion_pwm_setvalue(ion_duty_cycle);
					// TODO: display default current and time instead 1234
					tm1637DisplayDecimal(ion_selected_time*100 + ion_selected_current, 1);
					// transition to idle state: wait start or stop button
					ion_state = ST_AUTO;
					break;
				}
			case ST_CURRENT:
				{
					// set current. call function every while cycle time
					// until current will be selected.
					// if no current selected yet, function returns 0.
					// if stop was selected, function returns -1.
					// if current was selected function returns current
					mode = ion_set(angle, true);
					if(mode == 0)
					{
						// nothing happen, continue select current
						ion_state = ST_CURRENT;
					}
					else
					{
						// Set was pressed
						// mode = selected current
						ion_selected_current = mode;
						// if ion_selected_current = MAX_CURRENT,
						// duty cycle = timer auto reload value = 100% duty
						ion_duty_cycle = (int)((TIM1->ARR * ion_selected_current * K_CURRENT_WR) / MAX_CURRENT);
						ion_pwm_setvalue(ion_duty_cycle);

						// select time now
						ion_state = ST_TIME;
					}
					break;
				}
			case ST_TIME:
				{
					// set time. call function every while cycle time
					// until time will be selected.
					// if no time selected yet, function returns 0.
					// if stop was selected, function returns -1.
					// if time was selected function returns time
					mode = ion_set(angle, false);
					if(mode == 0)	
					{
						// nothing happen, continue select time 
						ion_state = ST_TIME;
					}
					else
					{
						// Set was pressed
						// mode = selected time 
						ion_selected_time = mode;
						ion_state = ST_CURRENT;
					}
					break;
				}

				// TODO: set ion_selected_current, ion_selected_time to -1 after stop and end of run

			case ST_RUN:
				{
					//ion_trace(ion_selected_current, (uint8_t*)"<--current ST_RUN\r\n");
					//ion_trace(ion_selected_time, (uint8_t*)"<--time ST_RUN\r\n");
					//ion_trace(-1, (uint8_t*)"\r\n");
					// monitor timer and cancel
					mode = ion_run(angle);
					if(mode == -1)
					{
						// timer elapsed
						ion_state = ST_BEGIN;
					}
					break;
				}
			case ST_BAT:
			default:
				{
					// show battery state
					ion_bat();
					// return to top level menu
					ion_state = ST_BEGIN;
					break;
				}
		} // switch(ion_state)

		//ion_trace(ion_adc_set, (uint8_t*)"<-- Set\r\n");
		//ion_trace(ion_adc_current, (uint8_t*)"<-- ADC current\r\n");
		//ion_trace(ion_duty_cycle, (uint8_t*)"<-- Duty cycle\r\n");
		//ion_trace(ion_selected_current, (uint8_t*)"<-- selected current\r\n");
		//ion_trace(-1, (uint8_t*)"\r\n");

		//tm1637DisplayDecimal(ion_adc_set, sep);

		//fdisp = (float)ion_adc_current * scale_12bit2mv;	
		//tm1637DisplayDecimal(disp, sep);

		if(sep == 0)
			sep = 1;
		else
			sep = 0;
	} // while

  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* ADC2 init function */
static void MX_ADC2_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Common config 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure Regular Channel 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);

}

/* TIM2 init function */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 3200;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 50;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_Output_CLK_Pin|GPIO_Output_DIO_Pin|GPIO_Output_Buzz_Pin|GPIO_Output_30V_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_Output_CLK_Pin GPIO_Output_DIO_Pin */
  GPIO_InitStruct.Pin = GPIO_Output_CLK_Pin|GPIO_Output_DIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_Input_Set_Pin GPIO_Input_Stop_Pin GPIO_Input_Start_Pin */
  GPIO_InitStruct.Pin = GPIO_Input_Set_Pin|GPIO_Input_Stop_Pin|GPIO_Input_Start_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_Output_Buzz_Pin */
  GPIO_InitStruct.Pin = GPIO_Output_Buzz_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO_Output_Buzz_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : GPIO_Output_30V_Pin */
  GPIO_InitStruct.Pin = GPIO_Output_30V_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIO_Output_30V_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

void ion_u2a(uint_fast16_t d, uint8_t* buf)
{
    buf[5] = '\0';
    buf[4] = '0' + ( d       )    % 10;
    buf[3] = '0' + ( d /= 10 )    % 10;
    buf[2] = '0' + ( d /= 10 )    % 10;
    buf[1] = '0' + ( d /= 10 )    % 10;
    buf[0] = '0' + ( d /  10 )    % 10;
}

void ion_u2a32(uint_fast32_t d, uint8_t* buf)
{
    buf[10] = '\0';
    buf[9] = '0' + ( d       )    % 10;
    buf[8] = '0' + ( d /= 10 )    % 10;
    buf[7] = '0' + ( d /= 10 )    % 10;
    buf[6] = '0' + ( d /= 10 )    % 10;
    buf[5] = '0' + ( d /= 10 )    % 10;
    buf[4] = '0' + ( d /= 10 )    % 10;
    buf[3] = '0' + ( d /= 10 )    % 10;
    buf[2] = '0' + ( d /= 10 )    % 10;
    buf[1] = '0' + ( d /= 10 )    % 10;
    buf[0] = '0' + ( d /  10 )    % 10;
}

// trace uint16 and string to UART
void ion_trace(uint16_t value, uint8_t* str)
{
    int len = 0;
    int c = 6;
    uint8_t buf[c];
    while(str[len])
    {
        len += 1;
    }
    // if value == -1 print string only
    if(value != 65535)
    {
        ion_u2a(value, buf);
        HAL_UART_Transmit(&huart1, buf, c, 500);
        HAL_UART_Transmit(&huart1, (uint8_t*)" ", 1, 500);
    }
    HAL_UART_Transmit(&huart1, str, len, 500);
}

// trace uint16 and string to UART
void ion_trace32(uint32_t value)
{
    int c = 11;
    uint8_t str[c];
    ion_u2a32(value, str);
    HAL_UART_Transmit(&huart1, str, c, 500);
    HAL_UART_Transmit(&huart1, (uint8_t*)" ", 1, 500);
}

void ion_pwm_setvalue(uint16_t value)
{
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, value);
	/*
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);  
	TIM_OC_InitTypeDef sConfigOC;

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = value;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);  
	*/
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
    ion_trace(-1,(uint8_t*)"Error Handler. Halt\r\n");
    ion_trace(line, (uint8_t*)file);

  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static const uint8_t CO2_ADDR = 0x70 << 1;
static const uint8_t REG_CO2 = 0x0C;
static const uint8_t Temp_ADDR = 0x27 << 1;
static const uint8_t REG_Temp = 0x00;
static const uint8_t Pressure_ADDR = 0x77 << 1;
static const uint8_t REG_Pressure = 0x04;
static const uint8_t REG_PW_CTRL = 0x1B;
static const uint8_t PW_CTRL_DATA = 0x33;
static const uint8_t REG_OSR = 0x1C;
static const uint8_t OSR_DATA = 0x00;

char Rx_data[11] = { 0 };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void convert(int8_t CO2_raw, uint8_t VOC_raw, uint16_t temp_raw, uint16_t hum_raw, uint16_t light_raw, uint32_t pressure_raw, uint32_t pressure_temp_raw, float* CO2_real, float* VOC_real, float* temp_real, float* hum_real, float* light_real, float* pressure_real, float* pressure_temp_real) {
    *CO2_real =  (CO2_raw - 13) * 1600 / 229 + 400;
    *VOC_real = (VOC_raw - 13) * 1000 / 229;
	*temp_real = temp_raw * 165 / (16384 - 2) - 40;
    *hum_real = hum_raw * 100 / (16384 - 2);
    *light_real = light_raw * 1000 / 4096;
    *pressure_real = pressure_raw / 2.64;
    *pressure_temp_real = pressure_temp_raw / 0.005;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  HAL_UART_Receive_IT(&huart1, (uint8_t*) Rx_data, 11);
  HAL_UART_Transmit(&huart1, (uint8_t*) "AOFFBONC", 8, 1000);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	HAL_StatusTypeDef ret;
	uint8_t buf[12];

	int8_t CO2_raw;
	int8_t VOC_raw;
	int16_t temp_raw;
	int16_t hum_raw;
	uint16_t light_raw;
	uint32_t pressure_raw;
	uint32_t pressure_temp_raw;

	float CO2_real;
	float VOC_real;
	float temp_real;
	float hum_real;
	float light_real;
	float pressure_real;
	float pressure_temp_real;

	//HAL_I2C_Mem_Write(&hi2c1, Pressure_ADDR, REG_PW_CTRL, 1, &PW_CTRL_DATA, 1, HAL_MAX_DELAY);
	//HAL_I2C_Mem_Write(&hi2c1, Pressure_ADDR, REG_OSR, 1, &OSR_DATA, 1, HAL_MAX_DELAY);
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, (uint8_t*) Rx_data, 11);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //CO2 & VOC sensor
	  buf[0] = REG_CO2;
	  ret = HAL_I2C_Master_Transmit(&hi2c1, CO2_ADDR, buf, 1, HAL_MAX_DELAY);
	  if ( ret != HAL_OK ) {
		  strcpy((char*)buf, "Error Tx\r\n");
	  } else {
		  HAL_Delay(100);
		  ret = HAL_I2C_Master_Receive(&hi2c1, CO2_ADDR, buf, 2, HAL_MAX_DELAY); //2 * 1 byte -> 2 bytes of data
		  if ( ret != HAL_OK ) {
			  strcpy((char*)buf, "Error Rx\r\n");
		  } else {
			  CO2_raw = buf[0];
			  VOC_raw = buf[1];
		  }
	  }

	  //Temperature & humidity sensor
	  buf[0] = REG_Temp;
	  ret = HAL_I2C_Master_Transmit(&hi2c3, Temp_ADDR, buf, 1, HAL_MAX_DELAY);
	  if ( ret != HAL_OK ) {
		  strcpy((char*)buf, "Error Tx\r\n");
	  } else {
		  HAL_Delay(100);
		  ret = HAL_I2C_Master_Receive(&hi2c3, Temp_ADDR, buf, 4, HAL_MAX_DELAY); //2 * 14 bits -> 4 bytes of data
		  if ( ret != HAL_OK ) {
			  strcpy((char*)buf, "Error Rx\r\n");
		  } else {
			  temp_raw = (buf[2] << 6) | (buf[3] >> 2);
			  hum_raw = (buf[0] << 8) | (buf[1]);
		  }
	  }

	  //Light sensor
	  HAL_ADC_Start(&hadc1);
	  if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK){
		  light_raw = HAL_ADC_GetValue(&hadc1);
	  }
	  HAL_ADC_Stop(&hadc1);

	  //Pressure sensor (I2C has to be set to 2)
	  /*buf[0] = REG_Pressure;
	  ret = HAL_I2C_Master_Transmit(&hi2c1, Pressure_ADDR, buf, 1, HAL_MAX_DELAY);
	  if ( ret != HAL_OK ) {
		  strcpy((char*)buf, "Error Tx\r\n");
	  } else {
		  HAL_Delay(100);
		  ret = HAL_I2C_Master_Receive(&hi2c1, Pressure_ADDR, buf, 6, HAL_MAX_DELAY);
		  if ( ret != HAL_OK ) {
			  strcpy((char*)buf, "Error Rx\r\n");
		  } else {
			  pressure_raw = (buf[2] << 16) | (buf[1] << 8) | buf[0];
			  pressure_temp_raw = (buf[5] << 16) | (buf[4] << 8) | buf[3];
		  }
	  }

	  ret = HAL_I2C_Master_Transmit(&hi2c1, Pressure_ADDR, buf, 1, HAL_MAX_DELAY);
	  if ( ret != HAL_OK ) {
		  strcpy((char*)buf, "Error Tx\r\n");
	  } else {
		  HAL_Delay(100);
		  ret = HAL_I2C_Master_Receive(&hi2c1, Pressure_ADDR, buf, 6, HAL_MAX_DELAY);
		  if ( ret != HAL_OK ) {
			  strcpy((char*)buf, "Error Rx\r\n");
		  } else {
			  pressure_raw = (buf[2] << 16) | (buf[1] << 8) | buf[0];
			  pressure_temp_raw = (buf[5] << 16) | (buf[4] << 8) | buf[3];
		  }
	  }*/

	  //Convert values
	  convert(CO2_raw, VOC_raw, temp_raw, hum_raw, light_raw, pressure_raw, pressure_temp_raw, &CO2_real, &VOC_real, &temp_real, &hum_real, &light_real, &pressure_real, &pressure_temp_real);

	  sprintf((char*)buf, "%u,%u,%u,%u,%u\r\n", (unsigned int)CO2_real, (unsigned int)VOC_real, (unsigned int)temp_real, (unsigned int)hum_real, (unsigned int)light_real);//, (unsigned int)pressure_real), (unsigned int)pressure_temp_real;
	  HAL_UART_Transmit(&huart1, buf, strlen((char*)buf), HAL_MAX_DELAY);
	  HAL_UART_Transmit(&huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
	  HAL_Delay(500);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

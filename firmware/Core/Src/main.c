/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "filter.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_BUF_SZ 1
#define MSG_BUF_SZ 50
#define UART_BUF_SZ 100

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char uart_buffer[UART_BUF_SZ];
uint8_t uart_txcplt;

uint32_t adc_buffer[ADC_BUF_SZ];
uint32_t aktueller_wert;
uint8_t current_byte;
uint8_t msg[MSG_BUF_SZ];
uint32_t msg_pointer;

FSK_Filter filter1;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart){
	uart_txcplt = 1;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart){

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	static uint8_t prev_sig_det;

	if (htim->Instance == htim1.Instance){
		float newVal =
				__LL_ADC_CALC_DATA_TO_VOLTAGE(3000, adc_buffer[0],
				ADC_RESOLUTION12b)
				/ 1000.0f;

		if (uart_txcplt){
			uart_txcplt = 0;

			uart_buffer[0] = 0xCC;
			uart_buffer[1] = 0xDD;
			uart_buffer[2] = 0xCC;
			uart_buffer[3] = 0xDD;
			memcpy(&uart_buffer[4], (uint16_t*)&adc_buffer[0], 2);
			memcpy(&uart_buffer[6], &filter1.y0, 4);
			memcpy(&uart_buffer[10], &filter1.y1, 4);
			HAL_UART_Transmit_IT(&huart2, (uint8_t*)uart_buffer, 14);
		}

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
		FSK_Filter_update(&filter1, newVal - 1.5f);
		if (FSK_Filter_isByteFinished(&filter1)) {
			if ((&filter1)->byte == 0b10101010){
				for (uint32_t i = 0; i < MSG_BUF_SZ; ++i){
					msg[i] = '\0';
			    }
			}
			else {
				msg[msg_pointer++] = (&filter1)->byte;
			}
			filter1.byte = 0;
		}
		// Signal detected falling edge -> Message End
		if (prev_sig_det != filter1.signal_detected &&
				filter1.signal_detected == 0){
			msg_pointer = 0;
			uart_buffer[0] = 0xAA;
			uart_buffer[1] = 0xBB;
			uart_buffer[2] = 0xAA;
			uart_buffer[3] = 0xBB;
			memcpy((char*)&uart_buffer[4], (char*)msg, strlen((char*)msg));
			uart_buffer[4 + strlen((char*)msg)] = '\n';
			uart_buffer[4 + strlen((char*)msg) + 1] = '\0';
			while(!uart_txcplt); // wait for uart to be free
			HAL_UART_Transmit(&huart2, (uint8_t*)uart_buffer, strlen(uart_buffer), 100);
		}

		prev_sig_det = filter1.signal_detected;

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_buffer, ADC_BUF_SZ);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  FSK_Filter_init(&filter1);

  uart_txcplt = 1;

  HAL_TIM_Base_Start_IT(&htim1);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&adc_buffer, ADC_BUF_SZ);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//HAL_UART_Transmit(&huart2, (uint8_t*)"Hello World\n\r", strlen("Hello World\n\r"), 100);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		HAL_Delay(100);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
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

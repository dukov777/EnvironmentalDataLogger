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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>

#include "dht22.h"

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

/* USER CODE BEGIN PV */
#define DHT22_TIMINGS_BUFFER_SIZE 40
uint16_t dht22_captured_timinings[DHT22_TIMINGS_BUFFER_SIZE];
uint16_t timinings[DHT22_TIMINGS_BUFFER_SIZE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define SPI_BUFFER_SIZE 20
uint8_t transmit_buffer[SPI_BUFFER_SIZE] = {1,2};
uint8_t receive_buffer[SPI_BUFFER_SIZE];

typedef union{
	uint32_t data;
	struct {
		uint32_t uart_rx_event:1;
		uint32_t uart_tx_event:1;
		uint32_t timer1_event:1;
		uint32_t dht22_reading_complete;

	};
}Event_t;

#define DTH22_DATA_SIZE 5
typedef union{
	uint8_t buffer[DTH22_DATA_SIZE];
	struct {
		// The humidity received
		uint16_t humidity;
		// The temperature received
		uint16_t temperature;
	};
}DTH22_Data_t;

Event_t events;
DTH22_Data_t dth22_data;

uint8_t bytes[5], checksum;


/// States for the interrupt driven state machine
enum STATES
{
    STATE_INIT = 0,
    STATE_START,
    STATE_RESPONSE1,  //First falling flank
    STATE_RESPONSE2,  //second falling flank
    STATE_DATA,
    STATE_IDLE,
    STATE_ERROR
};

/// The state variable for the interrupt driven state machine
volatile uint32_t dht22_state = STATE_INIT;

/// Trigger the reading of the DHT22 data, this is a non blocking call
void DHT22_StartReading()
{
//  TIM_OCStructInit(&TIM_OCInitStructure);
//  TIM_ICStructInit(&TIM_ICInitStructure);
//
//  TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3, DISABLE);
//  TIM_ClearITPendingBit(TIM4, TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3);

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = DHT22_Data_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT22_Data_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(DHT22_Data_GPIO_Port, DHT22_Data_Pin, GPIO_PIN_RESET);

//
//  DMA_Cmd(DMA1_Channel5, DISABLE);
//  DMA_DeInit(DMA1_Channel5);
//
//  TIM_DeInit(TIM4);
//  TIM_InitStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1; //1�s per clock
//  TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
//  TIM_InitStructure.TIM_Period = 10000;    //10ms timeout
//  TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
//  TIM_TimeBaseInit(TIM4, &TIM_InitStructure);
//  TIM_Cmd(TIM4, ENABLE);
//
//
//  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Active;
//  TIM_OCInitStructure.TIM_Pulse = 1000;   //1ms
//  TIM_OC2Init(TIM4, &TIM_OCInitStructure);
//
//  TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
//  TIM_ICInitStructure.TIM_ICFilter = 0;
//  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
//  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
//  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
//
//  TIM_ICInit(TIM4, &TIM_ICInitStructure);
//
//  TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
//  TIM4->CNT = 0;
//  TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
//  TIM_ITConfig(TIM4, TIM_IT_CC3, DISABLE);  //No capture interrupts yet
//  TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);
//
//  DMA1_Channel5->CNDTR = 40;
//  DMA_Init(DMA1_Channel5, &DMA_InitStructure);
//  DMA_ClearITPendingBit(DMA1_IT_TC5);
//  DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, DISABLE);
//
  dht22_state = STATE_START;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	events.uart_rx_event = 1;
	// initiate DTH22 reading sequence
	// wakeup the DTH22
	// set data pin to 0
	// start timer for 1 ms
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	events.uart_tx_event = 1;
	HAL_UART_Receive_IT(&huart2, receive_buffer, 1);

}

/// General function to handle an error state and reset all peripherals
/// Uncomment if printf is available and the debug output is needed
void DHT22_Error() {
//	if (dht22_state == STATE_DATA) {
//		//printf("DMA transfer timeout, number of Transfers: %d\r\n", DMA_GetCurrDataCounter(DMA1_Channel5));
//		if (DMA_GetFlagStatus(DMA1_FLAG_TE5) != RESET) {
//			//printf("DMA Error!\r\n");
//			DMA_ClearFlag(DMA1_FLAG_TE5);
//		}
//		if (TIM_GetFlagStatus(TIM4, TIM_FLAG_CC3OF) != RESET) {
//			//printf("Timer overcapture flag!\r\n");
//			TIM_GetCapture3(TIM4);
//		}
//	} else if (state == STATE_RESPONSE1) {
//		//printf("First Response timeout\r\n");
//	} else if (state == STATE_RESPONSE2) {
//		//printf("Second Response timeout\r\n");
//	} else if (state == STATE_START) {
//		//printf("Start Timeout\r\n");
//	} else {
//		//printf("Other timeout with state %d\r\n", state);
//	}
//	//printf("T1: %x T2: %x\r\n", _t1, _t2);
//	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//	TIM_ITConfig(TIM4, TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3, DISABLE);
//	DMA_Cmd(DMA1_Channel5, DISABLE);
//
//	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, DISABLE);
//	///maybe set an error flag here
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	UNUSED(htim);

	HAL_TIM_ActiveChannel channel = htim->Channel;
	if(channel == HAL_TIM_ACTIVE_CHANNEL_2) {
		HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_2);

		HAL_GPIO_WritePin(DHT22_Data_GPIO_Port, DHT22_Data_Pin, GPIO_PIN_SET);

		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = DHT22_Data_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(DHT22_Data_GPIO_Port, &GPIO_InitStruct);

		// Initiate waiting for falling edge on DHT22_Data_Pin
		// DHT22_Data_Pin is CH1 default capture compare pin
		HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);

		return;
	}
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	events.timer1_event = 1;

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		if(dht22_state == STATE_RESPONSE1){
			uint16_t ellapsed_time = __HAL_TIM_GET_COUNTER(htim);
			if(ellapsed_time < 50){
				dht22_state = STATE_RESPONSE2;
				__HAL_TIM_SET_COUNTER(htim, 0);
				HAL_TIM_IC_Start_IT(htim, TIM_CHANNEL_1);
			}else{
				DHT22_Error();
			}
		} else if(dht22_state == STATE_RESPONSE2){
			uint16_t ellapsed_time = __HAL_TIM_GET_COUNTER(htim);
			if(ellapsed_time < 200){
				dht22_state = STATE_DATA;

				__HAL_TIM_SET_COUNTER(htim, 0);
				HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, dht22_captured_timinings, DHT22_TIMINGS_BUFFER_SIZE);
			}else{
				DHT22_Error();
			}
		} else if(dht22_state == STATE_DATA) {
			events.dht22_reading_complete = 1;

			// convert the captured dht22 time series into Rh and Temp
		    bytes[0] = (dht22_captured_timinings[0] > 100) ? 1:0;
		    for(int i = 1; i < DTH22_DATA_SIZE * 8; i++){
		        bytes[i/8] <<= 1;
		        bytes[i/8] |= ((dht22_captured_timinings[i]-dht22_captured_timinings[i-1]) > 100) ? 1:0;      //Saved length should be 50us + 26us for a 0 and 50us + 70us for a one
		    }

		    checksum = (bytes[0] + bytes[1] + bytes[2] + bytes[3]);
		    // convert only if checksum is correct, otherwise we waste CPU cycles and power
		    if(bytes[4] == checksum){
		        dht22_state = STATE_IDLE;

		        dth22_data.buffer[0] = bytes[1];
		    	dth22_data.buffer[1] = bytes[0];

		    	dth22_data.buffer[2] = bytes[3];
		    	dth22_data.buffer[3] = bytes[2];
		    	// transmit received humidity and temperature in little endian order.
		    	HAL_StatusTypeDef hal_status = HAL_UART_Transmit_DMA(&huart2, dth22_data.buffer, DTH22_DATA_SIZE);
		    	if(hal_status != HAL_OK){
		    		// Unexpected UART State. It has to be free for transmission.
		    		Error_Handler();
		    	}
		    }else{
				DHT22_Error();
		    }
		} else {
			// Unexpected state. Something is wrong with uC. Do not proceed further.
			Error_Handler();
		}
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
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

//  HAL_DMA_RegisterCallback(&hdma_usart2_tx, HAL_DMA_XFER_CPLT_CB_ID, DMA_Transmit_Callback);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UART_Receive_IT(&huart2, receive_buffer, 1);

  TIM1->CCR1 = 50;
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  __HAL_TIM_SET_COUNTER(&htim2, 0);
  HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);
  DHT22_StartReading();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(events.uart_rx_event){
		  const char string[] = "blaaa";
		  transmit_buffer[0] = strlen(string);
		  strcpy((char*)&transmit_buffer[1], string);
		  HAL_UART_Transmit_DMA(&huart2, transmit_buffer, strlen(string) + 1);
	  }

	  if(events.uart_tx_event){
	  }

	  if(events.timer1_event) {

	  }

	  events.data = 0;

//	  HAL_SuspendTick();
//	  HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
//	  HAL_ResumeTick();

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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

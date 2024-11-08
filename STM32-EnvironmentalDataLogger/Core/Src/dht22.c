#include "tim.h"
#include "gpio.h"

#include "dht22.h"

enum STATES
{
    STATE_INIT = 0,
    STATE_START,
    STATE_RESPONSE1,  //First falling flank
    STATE_RESPONSE2,  //second falling flank
    STATE_DATA,
    STATE_IDLE,
    STATE_STOP
};

#define DHT22_TIMINGS_BUFFER_SIZE 40
#define DTH22_DATA_SIZE 5

#define DHT22_PULL_UP_TIME 50000 // 50us
#define DHT22_PULL_SENSOR_RESPONCE_TIME 200 // 200us
#define DHT22_PULL_BIT_SET_TIME 100 // 100us

#define DHT22_IS_BIT_SET(time) (time > DHT22_PULL_BIT_SET_TIME)

// The state variable for the interrupt driven state machine
uint32_t dht22_state = STATE_INIT;
uint16_t dht22_captured_timinings[DHT22_TIMINGS_BUFFER_SIZE];
uint16_t humidity;
int16_t temperature;
uint8_t bytes[DTH22_DATA_SIZE];
uint8_t checksum;


// Trigger the reading of the DHT22 data, this is a non blocking call
void DHT22_Start()
{
	DHT22_Stop();

	// Start the timer peripherial
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = DHT22_Data_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT22_Data_GPIO_Port, &GPIO_InitStruct);
	HAL_GPIO_WritePin(DHT22_Data_GPIO_Port, DHT22_Data_Pin, GPIO_PIN_RESET);
	
	// Clear counter and start the timer
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	
	// Wakeup the sensor.
    HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);

	// Start timeout timer
    HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_3);

	dht22_state = STATE_START;
}

// Stop DHT22 interrupts. This is a blocking call
void DHT22_Stop(){
	dht22_state = STATE_STOP;

	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_3);
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
	HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
	HAL_TIM_Base_Stop_IT(&htim2);
}

// Handles DHT22 the error state, initiate public dht22 state and reset all peripherals to initialization state.

static void DHT22_Error(DHT22_ErrorCodes_t error) {
	DHT22_Stop();

	// Tell the listener that an error has occured
	DHT22_ErrorCallback(error);
}

// Dedicated error handlers for each error type

static void DHT22_TimeoutError() {
	DHT22_Error(DHT22_ERROR_TIMEOUT);
}


static void DHT22_ChecksumError() {
	DHT22_Error(DHT22_ERROR_CHECKSUM);
}


static void DHT22_FramingError() {
	DHT22_Error(DHT22_ERROR_FRAMING);
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {
	if(htim == &htim2){
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
			dht22_state = STATE_RESPONSE1;
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
		} else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
			HAL_TIM_OC_Stop_IT(htim, TIM_CHANNEL_3);
			DHT22_TimeoutError();
		}
	}
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if(htim == &htim2){
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
			if(dht22_state == STATE_RESPONSE1){
				uint16_t ellapsed_time = __HAL_TIM_GET_COUNTER(htim);
				if(ellapsed_time < DHT22_PULL_UP_TIME){
					dht22_state = STATE_RESPONSE2;
					__HAL_TIM_SET_COUNTER(htim, 0);
				}else{
					DHT22_FramingError();
				}
			} else if(dht22_state == STATE_RESPONSE2){
				HAL_TIM_IC_Stop_IT(htim, TIM_CHANNEL_1);
				uint16_t ellapsed_time = __HAL_TIM_GET_COUNTER(htim);
				if(ellapsed_time < DHT22_PULL_SENSOR_RESPONCE_TIME){
					dht22_state = STATE_DATA;

					//clear CNT to avoid overflow during the data capture
					__HAL_TIM_SET_COUNTER(htim, 0);
					HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)dht22_captured_timinings, DHT22_TIMINGS_BUFFER_SIZE);
				}else{
					DHT22_FramingError();
				}
			} else if(dht22_state == STATE_DATA) {
				HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);
				// convert the captured dht22 time series into Rh and Temp
				bytes[0] = DHT22_IS_BIT_SET(dht22_captured_timinings[0]) ? 1:0;
				for(int i = 1; i < DTH22_DATA_SIZE * 8; i++){
					bytes[i/8] <<= 1;

					uint16_t time_diff = dht22_captured_timinings[i] - dht22_captured_timinings[i-1];
					bytes[i/8] |= DHT22_IS_BIT_SET(time_diff) ? 1:0;      //Saved length should be 50us + 26us for a 0 and 50us + 70us for a one
				}

				checksum = (bytes[0] + bytes[1] + bytes[2] + bytes[3]);
				// convert only if checksum is correct, otherwise we waste CPU cycles and power
				if(bytes[4] == checksum){
					dht22_state = STATE_IDLE;

					//convert the data without the sign bit
					temperature = (((uint16_t)bytes[2] & 0x7F)<<8) | bytes[3];
					
					// add the sign
					if (bytes[2] & 0x80) {
						temperature *= -1;
					}

					humidity = ((int16_t)bytes[0] << 8) | bytes[1];

					DHT22_Stop();
					// Notify the listener that the data is ready
					DHT22_DataReadyCallback(humidity, temperature);
				}else{
					DHT22_ChecksumError();
				}
			} else {
				// Unexpected state. Something is wrong with uC. Do not proceed further.
				Error_Handler();
			}
		}
	}
}


__weak void DHT22_ErrorCallback(DHT22_ErrorCodes_t error){
	UNUSED(error);

}


__weak void DHT22_DataReadyCallback(uint16_t humidity, int16_t temperature){
	UNUSED(humidity);
	UNUSED(temperature);
}

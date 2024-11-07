/*
 * dht22.h
 *
 *  Created on: Nov 4, 2024
 *      Author: petarlalov
 */

#ifndef _INC_DHT22_H_
#define _INC_DHT22_H_
#include <stdint.h>

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

typedef enum
{
    DHT22_ERROR_NO_ERROR = 0,
    DHT22_ERROR_FRAMING,
    DHT22_ERROR_TIMEOUT,
    DHT22_ERROR_CHECKSUM
}DHT22_ErrorCodes_t;

void DHT22_Init();
void DHT22_DeInit();

// Start reading the sensor
void DHT22_StartReading();

// Callbacks on Data ready and error
void DHT22_DataReadyCallback(uint16_t humidity, int16_t temperature);
void DHT22_ErrorCallback(DHT22_ErrorCodes_t error);

#endif /* _INC_DHT22_H_ */

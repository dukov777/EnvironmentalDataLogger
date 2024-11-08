#ifndef _INC_DHT22_H_
#define _INC_DHT22_H_
#include <stdint.h>

typedef enum {
    DHT22_ERROR_NO_ERROR = 0,
    DHT22_ERROR_FRAMING,
    DHT22_ERROR_TIMEOUT,
    DHT22_ERROR_CHECKSUM
}DHT22_ErrorCodes_t;

void DHT22_Start();
void DHT22_Stop();

// Callbacks on Data ready and error
void DHT22_DataReadyCallback(uint16_t humidity, int16_t temperature);
void DHT22_ErrorCallback(DHT22_ErrorCodes_t error);

#endif /* _INC_DHT22_H_ */

# Status
:white_check_mark: Done
1. The project has been tested with DHT11 -> Nucleo F103 -> ESP32 Devkit->ThingSpeak

:negative_squared_cross_mark:	 Not Done
1. No defensive programming
2. No HAL API error handling
3. Consistent formatting and naming conventions across STM32 and ESP projects :(

# Design
The selected approach for communication and measurement is the request->measure->reply. By doing this, ESP32 triggers the measurement and waits until the measured data is returned or timeouts. 
To reduce the power used, I decided to minimize resources and time executions, so the STM32 code is designed in SLEEPONEXIT mode, where only the interrupts are called on activity on certain peripherals.
Particularly, once the STM32 receives the UART RX interrupt, it initiates the DHT wakeup process by starting TIM2 in CO mode for 5000 uS. On the TIM CO interrupt, we pull up the data pin and start the TIM2 CI to capture the input change. Once the DHT sync has been received, the TIM CI is started again to receive the remaining data bit stream.
Completing the data receiving, we convert bitstream to data and initiate UART TX in DMA mode.
In cases of DHT bitstream error, we send the Error information to ESP32 and cut the execution, thus preserving the power.

# Implementation

The STM32 to ESP32 data format is little-endian packed in 5 bytes. This is the simplest format, but it could cause issues if the compiler has issues with packing. This structure has been copy-pasted in ESP32 and STM32, which is bad practice since changing one place does not guarantee we did all over the rest.

``` C
typedef union{
	uint8_t buffer[ENVIRONMENT_DATA_REPLY_SIZE];
	// packet structure
  struct {
    uint8_t   status;
		uint16_t  humidity;
    int16_t temperature;
  } __attribute__((packed));
  ;
}EnvironmentDataReply_t;
```

STM32 configuration has been done with STM32CubeMX, and some of the timers initialize values are "hidden". This makes it hard to understand why some decisions are made by reading code only.

ESP32 reading from STM32 and writing to ThingSpeak is straightforward without any power management.


# Project
https://github.com/users/dukov777/projects/4


# Wiring

https://github.com/dukov777/EnvironmentalDataLogger/blob/main/Docs/Schematics.pdf
https://github.com/dukov777/EnvironmentalDataLogger/tree/main/Hardware

# Setup
## Prepare Environment for ESP32

Create a file `secrets.h` in the `ESP32-EnvironmentalDataLogger` folder with the following content:

```bash
// Use this file to store all of the private credentials 
// and connection details

#define SECRET_SSID "MySSID"		// replace MySSID with your WiFi network name
#define SECRET_PASS "MyPassword"	// replace MyPassword with your WiFi password

#define SECRET_CH_ID 000000			// replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "XYZ"   // replace XYZ with your channel write API Key
```

# Tests
### 1. Install python and dependencies

```bash
python3 -m venv .venv
source .venv/bin/activate  # On macOS/Linux
pip install -r requirements.txt
```

### 2. Test STM32 communication channel

Read DHT22 data from STM32F103
```bash
python communicator.py --port /dev/tty.usbserial-0001
```
replace /dev/tty.usbserial-0001 with your port

The reply should be like this:
```bash
Status:  OK
Humidity:  36
Temperature:  23
```

# Resources
https://embedded-systems-design.github.io/overview-of-the-esp32-devkit-doit-v1/

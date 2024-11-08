
## Project
https://github.com/users/dukov777/projects/4


## Resources
https://embedded-systems-design.github.io/overview-of-the-esp32-devkit-doit-v1/

## Wiring

https://github.com/dukov777/EnvironmentalDataLogger/tree/main/Hardware

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


## Test
### 1. Install python and dependencies

```bash
python3 -m venv .venv
source .venv/bin/activate  # On macOS/Linux
pip install -r requirements.txt
```

### 2. Run the test

Read DHT22 data from STM32F103
```bash
python communicator.py --port /dev/tty.usbserial-0001
```
replace /dev/tty.usbserial-0001 with your port

Reply should be like this:
```bash
Status:  OK
Humidity:  36
Temperature:  23
```

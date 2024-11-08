


## Project
https://github.com/users/dukov777/projects/4


## Resources
https://embedded-systems-design.github.io/overview-of-the-esp32-devkit-doit-v1/

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

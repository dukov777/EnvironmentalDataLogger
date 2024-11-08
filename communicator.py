import serial
import argparse


def error_code_to_string(error_code):
    if error_code == 0:
        return "OK"
    elif error_code == 1:
        return "DHT22_ERROR_FRAMING"
    elif error_code == 2:
        return "DHT22_ERROR_TIMEOUT"
    elif error_code == 3:
        return "DHT22_ERROR_CHECKSUM"
    else:
        return "UNKNOWN ERROR CODE"


def __main():
    parser = argparse.ArgumentParser(description='Communicate with DHT22 sensor.')
    parser.add_argument('--port', type=str, help='Serial port name')
    parser.add_argument('--baudrate', type=int, help='Baudrate', default=115200)
    parser.add_argument('--timeout', type=int, help='Timeout', default=1)
    args = parser.parse_args()

    with serial.Serial(args.port, baudrate=args.baudrate, timeout=args.timeout) as ser:
        ser.write(b'\x01')
        
        status = ser.read(1)
        humidity = ser.read(2)
        temperature = ser.read(2)
        
        print("Status: ", error_code_to_string(int.from_bytes(status, byteorder='little')))
        humidity = humidity[1] + humidity[0]*256
        print("Humidity: ", humidity)
        temperature = temperature[1] + temperature[0]*256
        print("Temperature: ", temperature)
    

if __name__ == '__main__':
    __main()
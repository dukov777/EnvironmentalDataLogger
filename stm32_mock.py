import serial
import argparse
import struct

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
    parser.add_argument('--timeout', type=int, help='Timeout', default=300)
    args = parser.parse_args()

    humidity = 0
    temperature = 0

    with serial.Serial(args.port, baudrate=args.baudrate, timeout=args.timeout) as ser:
        while True:
            command = ser.read(1)
            print("Command: ", command)
            # encoded in little-endian format
            # byte 0: status
            # byte 1-2: humidity
            # byte 3-4: temperature
            humidity = humidity + 1
            temperature = temperature + 1
            
            buffer = struct.pack('<BHH', 0, humidity, temperature)
            ser.write(buffer)


if __name__ == '__main__':
    __main()
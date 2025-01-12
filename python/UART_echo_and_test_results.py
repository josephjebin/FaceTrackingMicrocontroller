import serial
import time
import sys

ser = serial.Serial(port="COM5", baudrate=9600, timeout=1)

try:
    while True:
        # if there is data to read
        if ser.in_waiting > 0:  
            data = ser.read()

            # echo next frame 
            if data == b'~': 
                print("E")
                sent = False
                while not sent: 
                    # if there is data to read
                    if ser.in_waiting > 0:
                        data = ser.read()
                        print(data)
                        ser.write(data) 
                        sent = True
            # else display data 
            else: 
                try: 
                    decoded_data = data.decode('utf-8')
                    print(decoded_data, end="")
                    sys.stdout.flush()  
                except UnicodeDecodeError: 
                    print(data)
                    print("Received malformed test results")

except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()  
import serial
import time

# input: 1 frame of UART data to echo back followed by
#        test results. 

ser = serial.Serial(port="COM5", baudrate=9600, timeout=1)

try:
    while True:
        if ser.in_waiting > 0:  # if there is data to read
            data = ser.read(ser.in_waiting)
            print("sending back: ")
            print(data)
            time.sleep(3) 
            ser.write(data) 

            while ser.in_waiting == 0: {}
                
            # hil tests will use null as terminating character for printing test results
            malformed_test_results = False
            while not malformed_test_results and data != 0x7E:
                try: 
                    data = ser.read(ser.in_waiting)
                    decoded_data = data.decode('utf-8')
                    print(decoded_data, end="")
                except UnicodeDecodeError:
                    print("Received malformed test results:")
                    malformed_test_results = True
            
except KeyboardInterrupt:
    print("Exiting...")
finally:
    ser.close()  


# try:
#     while True:
#         if ser.in_waiting > 0:
#             data = ser.read(ser.in_waiting)
#             print(data)
            
# except KeyboardInterrupt:
#     print("Exiting...")
# finally:
#     ser.close()  
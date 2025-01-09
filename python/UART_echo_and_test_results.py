import serial

# input: 1 frame of UART data to echo back followed by
#        test results. 

ser = serial.Serial(port="COM5", baudrate=9600, timeout=1)

try:
    while True:
        if ser.in_waiting > 0:  # if there is data to read
            data = ser.read(ser.in_waiting)
            print("sending back: ")
            print(data)
            ser.write(data) 

            while not ser.in_waiting: {
                # wait for test results
            }
                
            # hil tests will use null as terminating character for printing test results
            malformed_test_results = False
            while not malformed_test_results and data == '\0':
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
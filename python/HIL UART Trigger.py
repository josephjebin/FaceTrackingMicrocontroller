import serial

ser = serial.Serial(port="COM3", baudrate=9600, timeout=1)

def send_scan_command():
    data = bytes([0x06])
    ser.write(data) 
    print(f"Sending data: 0x{data.hex()}")

send_scan_command()
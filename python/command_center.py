import cv2
import time 
import serial
import pyaudio
import pygame
import numpy as np
from enum import Enum
import random

class MinionState(Enum):
    TRACKING = 1
    SCANNING = 2
    SLEEPING = 3
current_state = MinionState.SLEEPING

# most significant nibble will control horizontal servo 
# least significant nibble will control vertical servo 
# x and y will be [0 and 6]
def send_position(x, y):
    if not (0 <= x <= 6) or not (0 <= y <= 6):
        raise ValueError("Inputs must be integers between 0 and 6 inclusive.")
    
    data = bytes([(x << 4) | y])
    ser.write(data)
    print(f"Sending data: x={x}, y={y} -> byte=0x{data.hex()}")

def send_scan_command():
    data = bytes([0x80])
    ser.write(data) 
    print(f"Sending data: 0x{data.hex()}")

# Load the pre-trained model
model_path = "opencv_face_detector_uint8.pb"
config_path = "opencv_face_detector.pbtxt"

# frame size and center
FRAME_WIDTH = 640
FRAME_CENTER_X = 320
FRAME_HEIGHT = 480
FRAME_CENTER_Y = 240

# Threshold for significant deviation of user's face from center (in pixels)
TOLERANCE = 20

# operating speed is .2 seconds / 60 degrees 
# UART delay ~1.05ms = .00105 seconds
# LARGE_COMPARE_INCREMENT = 24 degrees = .08 seconds
# LARGE_DELAY = 0.08
LARGE_DELAY = 0.5

# MEDIUM_COMPARE_INCREMENT = 9.6 degrees = .032 seconds
# MEDIUM_DELAY = .06
MEDIUM_DELAY = .45
# SMALL_COMPARE_INCREMENT = .96 degrees = .0032 seconds
# SMALL_DELAY = .01
SMALL_DELAY = .1

# delay for triggering minion to scan for faces because no faces are currently detected
DELAY_BEFORE_SCANNING = 5
DELAY_BEFORE_SLEEPING = 15

face_last_detected_time = time.time()  

sound_last_played_time = time.time()
sound_duration = 0
sound_next_play_time = time.time()

# variableS used to control when a command can be sent to prevent sending a command while the servos are still positioning 
horiztonal_command_next_send_time = time.time()
vertical_command_next_send_time = time.time()

# Initialize serial communication
ser = serial.Serial(port="COM5", baudrate=9600, timeout=1)

# while True: 
#     for i in range(20):
#         send_position(4, 0)
#         time.sleep(SMALL_DELAY) 

#     time.sleep(.3)

#     for i in range(40): 
        # send_position(1, 0)
        # time.sleep(SMALL_DELAY)

# Load the network using OpenCV's DNN module
net = cv2.dnn.readNetFromTensorflow(model_path, config_path)

# Open the webcam
cap = cv2.VideoCapture(0)  # Use 0 for the default webcam; change index for external cameras
# cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
# cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

# Initialize the microphone input
p = pyaudio.PyAudio()
chunk = 1024  # Size of each audio chunk (number of frames per buffer)
device_index = 1  # Index of your microphone device, adjust as necessary
stream = p.open(format=pyaudio.paInt16,
                channels=1,
                rate=16000,
                output=False,
                input=True, 
                input_device_index=device_index,
                frames_per_buffer=chunk)  

# intialize audio output 
pygame.mixer.pre_init(frequency=22050, size=-16, channels=1, buffer=512)
pygame.init()
huh = pygame.mixer.Sound("Huh sound effect.wav")
snore = pygame.mixer.Sound("snoring.wav")
fart = pygame.mixer.Sound("fart.wav")
song = pygame.mixer.Sound("Naruto Theme - The Raising Fighting Spirit.wav")

while True:
    if current_state == MinionState.SLEEPING: 
        audio = np.frombuffer(stream.read(chunk), dtype=np.int16)
        volume = np.linalg.norm(audio) 
        # first condition makes sure minion doesn't wake itself up from its own noises 
        # through testing, I found my base volume is ~2900-31000
        if time.time() >= (sound_last_played_time + sound_duration) and (volume > 60000):  
            print(volume)
            send_scan_command()
            huh.play()
            current_state = MinionState.SCANNING
            face_last_detected_time = time.time()  
            time.sleep(.099)
        elif time.time() > sound_next_play_time: 
            if(random.randint(0, 1)): 
                snore.play()
                sound_duration = 1
            else: 
                fart.play()
                sound_duration = .5
            sound_last_played_time = time.time()
            sound_next_play_time = sound_last_played_time + random.randint(3, 6)

    else: 
        # Capture frame-by-frame
        ret, frame = cap.read()
        if not ret:
            print("Error: Could not read frame.")
            break

        # Get the height and width of the frame
        h, w = frame.shape[:2]

        # Prepare the image for the DNN module
        # Resize to the network's expected input size and normalize pixel values
        blob = cv2.dnn.blobFromImage(frame, scalefactor=1.0, size=(300, 300), mean=(104.0, 177.0, 123.0))

        # Set the blob as input to the network
        net.setInput(blob)

        # Perform a forward pass to get detections
        detections = net.forward()

        # boolean used to indicate if work for a face has been done
        # this is to handle multiple faces and prevent the servo from bouncing between faces
        sentFacePosition = False

        # Iterate over detections
        for i in range(detections.shape[2]):
            if(sentFacePosition): break
            confidence = detections[0, 0, i, 2]  # Confidence level of the detection

            # Only consider detections with confidence above a certain threshold
            if confidence > 0.98:
                if not pygame.mixer.get_busy():  # Check if the song is already playing
                    song.play(loops=-1)  
                current_state = MinionState.TRACKING
                face_last_detected_time = time.time()  
                # Get bounding box coordinates
                box = detections[0, 0, i, 3:7] * [w, h, w, h]
                (x1, y1, x2, y2) = box.astype("int")

                # Draw a bounding box around the face
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                label = f"Face: {confidence * 100:.2f}%"
                cv2.putText(frame, label, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

                # distance values
                    # face moves left, on screen moves right: distance goes up
                    # face moves right: distance goes down and becomes more negative
                    # face goes up: distance goes down 
                    # face goes down: distance goes up

                needToSend = False
                output_x = 0
                output_y = 0
                # Calculate the center of the face
                if time.time() >= horiztonal_command_next_send_time: 
                    face_center_x = (x1 + x2) // 2
                    distance_x = face_center_x - FRAME_CENTER_X
                    abs_distance_x = abs(distance_x)
                    # value used for determining small, medium, or large jump
                    fraction_x = abs_distance_x/FRAME_WIDTH

                    if abs_distance_x > TOLERANCE: 
                        needToSend = True
                        if(fraction_x > .25):
                            output_x += 3
                            horiztonal_command_next_send_time = time.time() + LARGE_DELAY
                        elif(fraction_x > .15): 
                            output_x += 2
                            horiztonal_command_next_send_time = time.time() + MEDIUM_DELAY
                        else: 
                            output_x += 1
                            horiztonal_command_next_send_time = time.time() + SMALL_DELAY
                            
                        # IRL moved right -> send positive jump
                        if(distance_x < 0): 
                            output_x += 3
                
                if time.time() >= vertical_command_next_send_time: 
                    face_center_y = (y1 + y2) // 2
                    distance_y = face_center_y - FRAME_CENTER_Y
                    abs_distance_y = abs(distance_y)
                    # value used for determining small, medium, or large jump
                    fraction_y = abs_distance_y/FRAME_HEIGHT
                
                    if abs_distance_y > TOLERANCE: 
                        needToSend = True
                        if(fraction_y > .25): 
                            output_y += 3
                            vertical_command_next_send_time = time.time() + LARGE_DELAY
                        elif(fraction_y > .15): 
                            output_y += 2
                            vertical_command_next_send_time = time.time() + MEDIUM_DELAY
                        else: 
                            output_y += 1
                            vertical_command_next_send_time = time.time() + SMALL_DELAY

                        # IRL moved up, send positive jump
                        if(distance_y < 0): 
                            output_y += 3

                if(needToSend): 
                    send_position(output_x, output_y)
                    needToSend = False 
                    sentFacePosition = True  

        if current_state == MinionState.TRACKING and (time.time() - face_last_detected_time > DELAY_BEFORE_SCANNING): 
            pygame.mixer.stop()  
            print('lost face')
            send_scan_command()
            current_state = MinionState.SCANNING
        elif current_state == MinionState.SCANNING and time.time() - face_last_detected_time > DELAY_BEFORE_SLEEPING: 
            print('NIGHT NIGHT')
            current_state = MinionState.SLEEPING
            # don't want the minion waking itself up
            sound_last_played_time = time.time()
            sound_next_play_time = sound_last_played_time + 1
        # Display the frame
        cv2.imshow("Face Detection", frame)

        # Break the loop when 'q' is pressed
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# Release the webcam and close the window
cap.release()
cv2.destroyAllWindows()
ser.close()

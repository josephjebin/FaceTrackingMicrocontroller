Overview Video! <br />
https://drive.google.com/file/d/1DXGHi_vd6GrTkzMNU0J7Osyh93HIYfZo/view?usp=drive_link
<br /> <br /> 

Application Dependencies: 

This project uses a TM4C123GXL to control the servos. The BSP for the TM4C along with system, startup, and register files are located in the tm4c directory. <br /> 
This project uses a python program for face detection and to control the servos. 

Testing Dependencies:

This project uses Unity and CMock for validating and mocking. <br /> 
To run the make commands, you will need the GNU make utility, which I obtained from the QTools collection (https://github.com/QuantumLeaps/qtools). <br /> 
You will also need ruby to generate mocks. <br /> 
This project uses a python program for UART communication for HIL tests. <br />
<br />
Running the application: 
1. Connect the servos to the microcontroller. The horizontal panning servo connects to PB6, and the vertical tilting servo connects to PB4. Make sure the grounds of the servos, battery packs, and microcontroller are connected.
2. Flash the microcontroller, and run cyber_minion_runner.c
3. Connect the webcam to your computer.
4. Run command_center.py
Don't make a sound, or the minion might wake up!  
<br />
How to test: 

Unit tests
1. Generate mocks by stepping into test\unit\ and changing paths to files as necessary (cmock.rb is in the lib folder of cmock) and running:
ruby cmock.rb src/bsp.h
2. Run the makefile in the unit test directory by running: 
make

Hardware in loop (HIL) tests
1. Run the python program UART_echo_and_test_results.py, which is a program that will echo UART data from the microcontroller back to the microcontroller. This program will also print the test results. 
2. Run the makefile in the hil test directory by running: 
make
3. Press the reset button on the microcontroller to start testing. Test results will show up in the python program's console.

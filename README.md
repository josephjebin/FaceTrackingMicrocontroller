Overview Video!

https://drive.google.com/file/d/1DXGHi_vd6GrTkzMNU0J7Osyh93HIYfZo/view?usp=drive_link


Dependencies: 

Application

This project uses a TM4C123GXL to control the servos. The BSP for the TM4C along with system, startup, and register files are located in the tm4c directory.

Testing

I used Unity and CMock for validating and mocking. To run the make commands, you will need the GNU make utility, which I obtained from the QTools collection (https://github.com/QuantumLeaps/qtools).


Running the application: 
1. Connect the servos to the microcontroller. The horizontal panning servo connects to PB6, and the vertical tilting servo connects to PB4. Make sure the grounds of the servos, battery packs, and microcontroller are connected.
2. Flash the microcontroller, and run cyber_minion_runner.c
3. Connect the webcam to your computer.
4. Run command_center.py
Don't make a sound, or the minion might wake up!  


How to test: 

Unit tests
1. Run the makefile in the unit test directory by typing: 
make

Hardware in loop (HIL) tests
1. Launch a serial communication terminal like PuTTY, and connect it to your microcontroller. Ensure the COM port and baud rate match the microcontroller's. This project uses a baud rate of 9600. 
2. Run the makefile by in the hil test directory by typing: 
make
3. Press the reset button on the microcontroller to start testing. Test results will show up in the serial terminal. 

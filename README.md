# Helmet  Electronics Details
## Theory of Operation
The basic gameplay is to open a sliding door, add cushioning materials, close the door and a ‘hammer’ drops to update data on a monitor. An accelerometer is attached to the center of the falling ‘hammer.’ During the fall acceleration & deceleration is recorded to an array and sent to the processing app via serial. Processing parses the data and plots it to the graph. Based on success levels processing will update images, colors, and play a sound. 

Processing controls the timing for the gameplay while the uController waits to receive signals via serial. The uController has several sensors to ensure safety and proper alignment. To sense the door’s close/open state it uses both a reed (NO) & a button trigger(NO). The reed confirms if the door has been reopened allowing for the next attempt.  

Once the door is closed a maglock engages preventing the door to be opened during the hammer drop. Indicating LEDs show if the lock is engaged or not. During startup the uController uses an IR sensor to find the ‘home’ location of the hammer. This is done to ensure repeatability and easy setup. 

## Updating software

The software is built using platform.io. You can use Arduino IDE by copying the src folder and renaming it to Helmet and renaming the main.cpp file to ‘Helmet.ino’. This will allow you to use Arduino to make modifications. You will need to install any missing libraries through the Arduino Library manager.

## Project File Organization

	docs : documentation and pictures
	src : source code
	eagle : pcb files
	examples_tests : N/A
	processing: location of computer app
	processing/data: assets used for the processing app
	readme.md : basic project info its the file you are reading
	resources:Helmet_Drop BOM.xls
	
	some other directories are used by platformio and vscode

#### Hardware

**Phoenix connectors:**

2-pin: (4 qty)

	1. 24V
	2. Safety Switch
	3. Accel Sensor (1 pair)
	
3-pin: (3 qty)

	1. IR In
	2. Reed In
	3. 12v In
	
4-pin: (2 qty)

	1. Stepper Motor
	2. Maglock
	
6-pin: (1 qty)

	1.Lock Light
	2.Unlock Light

Various Components:

	3 mosfets (SMD 7460AAW44K)
	3 10K SMS Resistors
	32U4 - 5v itsyBitsy
	Stepper Motor
	Stepper Driver
	IR Sensor (CPB765WZ)
	Happ Switch
	Reed Switch
	Maglock
	24V PSU
	12V PSU
	5V across USB

#### Sensor Board
IR sensor board - used to set IR with resistors as well as a troubleshooting board.

## Audio
They are Mp3 files stored /data/sound/ and can be updated simply by following the naming scheme and replacing old files. It is recommended making a backup before replacing any files. 
Capitalization matters. 

##### Naming scheme:

	Failure: red.mp3
	Average: orange.mp3
	Success: green.mp3

## Images/video

The images are saved as PNG and the video is MP4, both are stored /data/images.  by following the naming scheme and replacing old files. It is recommended making a backup before replacing any files. Capitalization matters. 


##### Naming scheme:

	Arrow Failure: redArrow.png
	Arrow Average: yellowArrow.mp3
	Arrow Success: greenArrow.mp3
	Head Failure: 3_en.png
	Head Average:  2_en.png
	Head Success:  1_en.png
	Video: attractor.mp4

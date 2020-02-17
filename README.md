# The SpeAr: a Spectrophotometer with Arduino
[![Build Status](https://travis-ci.com/massimofrassetto/SpeAr.svg?branch=master)](https://travis-ci.com/massimofrassetto/Spear)

## Welcome to this repository
**Hey Folks!** My name is Massimo Frassetto and this is a repository where you can find a simple firmware for my final Exam project on 2015. I was really excited of this project so i decided to continue modify and upgrade it.
**I hope you enjoy it! :)**

## Before you continue...
Yes.. I must warn you... **I'm not an expert programmer!** I program (and study) in my spare time so surely there will be mistakes or I will not have chosen the correct solution. I simply ask you, if you find something that you think should be modified, to write to me at the following email address: frassetto.massimoo@gmail.com
If you wish you can also send me some material or other documentation that I will be happy to read. You can send me modify suggest write by you too!

`Thank you for understanding!`

## Documentation and Wiki
All the documentation sections are still "Work In Progress". For the moment i can only give my thesis (in Italian).
* https://drive.google.com/open?id=1wMpfqE4ke-pRUaybfkaCi_CKb389sP8Q

The description of the various functions can be found directly in the source. There is not yet a purpose-built library.
In the future I will also insert the procedure and the wiring diagram so that you can build your own copy. I repeat: if you find something wrong or that can be improved do not be afraid to write to me!!! (If you are impatient you can easily deduce the various connections by reading the code and looking at the datasheets of the individual components)
## Coding Section

### Libraris you may need
* Default (already present on Arduino IDE)
	* LiquidCrystal
	* Wire
	* SPI
	* SD
* Custom
	* From Mark Stanley, Alexander Brevig:
		* Keypad
			* Arduinio Playround reference : 	https://playground.arduino.cc/Code/Keypad/
			* Direct Github link: 				https://github.com/Chris--A/Keypad
	* From Adafruit Industries (https://www.adafruit.com/):
		* Light Sensor:
			* Adafruit_TSL2591: 				https://github.com/adafruit/Adafruit_TSL2591_Library
			* Adafruit_Sensor:					https://github.com/adafruit/Adafruit_Sensor
		* MotorShield:
			* Adafruit_MotorShield: 			https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library

## Materials Section

### Component list
* TSL2591 High Dynamic Range Digital Light Sensor: https://www.adafruit.com/product/1980
* Motor/Stepper/Servo Shield: https://www.adafruit.com/product/1438
* Arduino Mega 2560: https://store.arduino.cc/arduino-mega-2560-rev3
* Simple rele: https://www.amazon.it/gp/product/B00CEQBCSW/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1
* DS1307 Real: Time Clock https://www.adafruit.com/product/3296
* Keypad: https://www.amazon.it/gp/product/B00K6Y02BW/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1
* Buzzer: https://www.amazon.it/gp/product/B00UREMX9U/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1
* SD card reader: https://www.amazon.it/gp/product/B00PC9I3M6/ref=ppx_yo_dt_b_asin_title_o05_s00?ie=UTF8&psc=1
* Dupont: https://www.amazon.it/gp/product/B00H3IHBQS/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
* Arduino Header: https://www.amazon.it/gp/product/B00Q6Y74DA/ref=ppx_yo_dt_b_asin_title_o03_s00?ie=UTF8&psc=1
* Breadboard: https://www.amazon.it/gp/product/B00CFFFF26/ref=ppx_yo_dt_b_asin_title_o02_s00?ie=UTF8&psc=1
* Stepper + Book: https://www.amazon.it/gp/product/B00DHIX0I6/ref=ppx_yo_dt_b_asin_title_o08_s00?ie=UTF8&psc=1

<!-- Comment Syntax -->



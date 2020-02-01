#ifndef COSTANTS.h
#define COSTANTS.h

#define MODEL_VERSION						1.0
#define FIRMWARE_VERSION					11

#define SERIAL_BAUDRATE						115200
#define PIN_LCD_RS 							8
#define PIN_LCD_ENABLE 						9
#define PIN_LCD_D4 							10
#define PIN_LCD_D5 							11
#define PIN_LCD_D6 							12
#define PIN_LCD_D7 							13

#define LCD_COLS							16
#define LCD_ROWS							2

#define ADAFRUIT_SENSOR_IDENTIFIER 			2519

#define MOTOR_PORT 							2
#define MOTOR_STEPS_PER_REVOLUTION 			200
#define MOTOR_SPEED_RPM						50
#define MOTOR_GRATINGLIMIT_LOW				850
#define MOTOR_GRATINGLIMIT_HIGH 			1250		

#define KEYPAD_ROWS 						4
#define KEYPAD_COLS 						3
#define PIN_KEYPAD_ROW_0 					38
#define PIN_KEYPAD_ROW_1 					37
#define PIN_KEYPAD_ROW_2 					36
#define PIN_KEYPAD_ROW_3 					35
#define PIN_KEYPAD_COLS_0 					34
#define PIN_KEYPAD_COLS_1 					33
#define PIN_KEYPAD_COLS_2 					32

#define PIN_PIEZO 							46

#define PIN_LAMP_SWITCH 					25
#define PIN_LAMP_CHECKINGSENSOR 			A9
#define LAMP_CHECKINGSENSOR_THRESHOLD 		500				//ms
#define LAMP_CHECKING_TIMEBREAK 			1000			//ms

#define PIN_MOTOR_POSITIONSENSOR 			A8
#define MOTOR_POSITIONSENSOR_THRESHOLD		470

#define	PIN_BUTTON_BACK 					3
#define PIN_BUTTON_NEXT						4
#define PIN_BUTTON_UP 						5
#define PIN_BUTTON_DOWN 					6
#define PIN_BUTTON_OK 						7

#define CHIPSELECT 							53

#define ANALYSISMODE_SIMPLEREAD				'1'
#define ANALYSISMODE_ALLSPECTRUM			'2'
#define ANALYSISMODE_CONCANALYSIS			'3'

#define SPECTRALIMIT_LOW					380
#define SPECTRALIMIT_HIGH					780
#define MIN_REPLICATES						5

#define ALLSPECTRUM_FILENAME				"allspect.txt"

#endif
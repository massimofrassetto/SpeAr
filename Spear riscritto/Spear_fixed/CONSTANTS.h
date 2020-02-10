// FILE PER RAGGRUPPARE TUTTE LE COSTANTI 

#ifndef COSTANTS_h
#define COSTANTS_h

// Informazioni interne della macchina

#define MODEL_VERSION						1.0
#define FIRMWARE_VERSION					13.3


// velocità della comunicazione seriale
#define SERIAL_BAUDRATE						250000

// ======== PARAMETRI PER GESTIRE IL SENSORE TSL ========
#define TSL_CONNECTION_DONE					0
#define TSL_CONNECTION_FAILED				1

#define TSL_READTYPE_VISIBLE				0
#define TSL_READTYPE_FULLSPECTRUM			1
#define TSL_READTYPE_INFRARED				2

// ======== PARAMETRI PER GESTIRE LO SCHERMO LCD ========
// Dimensioni dello schermo
#define LCD_COLS							16
#define LCD_ROWS							2
// Pin per il controllo
#define PIN_LCD_RS 							8
#define PIN_LCD_ENABLE 						9
#define PIN_LCD_D4 							10
#define PIN_LCD_D5 							11
#define PIN_LCD_D6 							12
#define PIN_LCD_D7 							13

// Codice identificativo del sensore dell'adafruit
#define ADAFRUIT_SENSOR_IDENTIFIER 			2519

// ======== PARAMETRI PER GESTIRE IL MOTORE STEPPER ======== 
// Parametri inizializzazione driver adafruit
#define MOTOR_PORT 							2
#define MOTOR_STEPS_PER_REVOLUTION 			200					//[STEP]
#define MOTOR_SPEED_RPM						50					//[RPM]
#define MOTOR_STEPTYPE						SINGLE
// #define MOTOR_STEPTYPE						DOUBLE
// #define MOTOR_STEPTYPE						INTERLEAVE
// #define MOTOR_STEPTYPE						MICROSTEP
// Parametri per far combaciare gli estremi del range delle lunghezze d'onda le posizioni di riferimento
#define MOTOR_STEPS_GRATINGLIMIT_LOW		850					//[STEP] corrisponde a SPECTRALIMIT_LOW
#define MOTOR_STEPS_GRATINGLIMIT_HIGH 		1250				//[STEP] corrisponde a SPECTRALIMIT_HIGH
#define MOTOR_STEPS_MACHINEZERO		 		1050				//[STEP]Zero macchina. Questi sono gli step che servono per portare la normale della superficia di rifrazione normale alla direzione dei raggi della sorgente luminosa.
#define MOTOR_POSITIONSENSOR_THRESHOLD		470					//[VALORE ANALOGICO 0%1024]valore sotto il quale considero il sensore ostruito e quindi non in zero 
#define PIN_MOTOR_POSITIONSENSOR 			A8

// ======== PARAMETRI PER GESTIRE IL TASTIERINO NUMERICO ========  
#define KEYPAD_ROWS 						4
#define KEYPAD_COLS 						3
#define PIN_KEYPAD_ROW_0 					38
#define PIN_KEYPAD_ROW_1 					37
#define PIN_KEYPAD_ROW_2 					36
#define PIN_KEYPAD_ROW_3 					35
#define PIN_KEYPAD_COLS_0 					34
#define PIN_KEYPAD_COLS_1 					33
#define PIN_KEYPAD_COLS_2 					32

// Pin del piezo/Buzzer
#define PIN_BUZZER 							46

// ======== PARAMETRI PER GESTIRE LA LAMPADA ========  
#define PIN_LAMP_SWITCH 					25
#define PIN_LAMP_CHECKINGSENSOR 			A9
#define LAMP_CHECKINGSENSOR_THRESHOLD 		460					//[VALORE ANALOGICO 0%1024] valore sotto il quale considero la lampada spenta
#define LAMP_CHECKING_TIMEBREAK 			250					//[MILLISECONDI] tempo di attesa da un cambio di stato e l'altro per dare alla resistenza il tempo di scaricarsi
#define LAMP_CHECK_PASSED					0
#define LAMP_CHECK_ERROR_PHASE_1			1
#define LAMP_CHECK_ERROR_PHASE_2			2
#define LAMP_CHECK_ERROR_PHASE_3			4

// ======== PARAMETRI PER GESTIRE I TASTI DIREZIONALI ========  
#define	PIN_BUTTON_BACK 					3
#define PIN_BUTTON_NEXT						4
#define PIN_BUTTON_UP 						5
#define PIN_BUTTON_DOWN 					6
#define PIN_BUTTON_OK 						7

// ======================== SD CARD	==========================
// https://www.arduino.cc/en/reference/SD
	// [...] The communication between the microcontroller and the SD card uses SPI,
	// which takes place on digital pins 11, 12, and 13 (on most Arduino boards) or 50, 51, and 52 (Arduino Mega).
	// Additionally, another pin must be used to select the SD card.
	// This can be the hardware SS pin - pin 10 (on most Arduino boards)
	// or pin 53 (on the Mega) - or another pin specified in the call to SD.begin().
	// Note that even if you don't use the hardware SS pin, it must be left as an output or the SD library won't work. [...]

#define CHIPSELECT 							53					

// Numero totale delle analisi attualmente gestite
#define ANALYSISMODE_NUMBER 				3
// Codici identificativi delle singole modalità
#define ANALYSISMODE_SIMPLEREAD				0
#define ANALYSISMODE_ALLSPECTRUM			1
#define ANALYSISMODE_CONCANALYSIS			2

// limiti dello speettro di analisi per il momento sono sul visibile e sono abbastanza empirici
#define SPECTRALIMIT_LOW					380					//[NANOMETRI] corrisponde a MOTOR_STEPS_GRATINGLIMIT_LOW
#define SPECTRALIMIT_HIGH					780					//[NANOMETRI] corrisponde a MOTOR_STEPS_GRATINGLIMIT_HIGH
#define MIN_REPLICATES						5					//Numero minimo di letture per considerare affidabile la media.

// Semplici costanati per gestire quanti decimali mostrare a seconda dei casi 
#define DECIMAL_LCD_TRASMITTANCE			6
#define DECIMAL_SERIAL_TRASMITTANCE			8
#define DECIMAL_LCD_ABSORBANCE				6
#define DECIMAL_SERIAL_ABSORBANCE			8
#endif
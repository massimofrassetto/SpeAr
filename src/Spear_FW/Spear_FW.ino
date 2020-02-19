/* PROGRAMMA PRINCIPALE */

#define DEBUG_SERIAL
// #define SERIAL_OUTPUT

#include <LiquidCrystal.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

#include <Adafruit_MotorShield.h>

#include <RTClib.h>

#include <SPI.h>
#include <SD.h>

#include <Keypad.h>

#include "CONSTANTS.h"
#include "MSG.h"
#include "FUNCTIONS.h"

Adafruit_TSL2591 m_tsl = Adafruit_TSL2591(ADAFRUIT_SENSOR_IDENTIFIER); // pass in a number for the sensor identifier (for your use later)

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *m_grtMotor = AFMS.getStepper(MOTOR_STEPS_PER_REVOLUTION, MOTOR_PORT);

RTC_DS1307 m_rtc;

LiquidCrystal m_lcd(LCD_PIN_RS, LCD_PIN_ENABLE, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);

File m_allSpectrumFile;
File m_traceLog;

const byte ROWS = KEYPAD_ROWS;
const byte COLS = KEYPAD_COLS;
char hexaKeys[ROWS][COLS] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};
byte rowPins[ROWS] = {KEYPAD_PIN_ROW_0, KEYPAD_PIN_ROW_1, KEYPAD_PIN_ROW_2, KEYPAD_PIN_ROW_3};
byte colPins[COLS] = {KEYPAD_PIN_COLS_0, KEYPAD_PIN_COLS_1, KEYPAD_PIN_COLS_2};
Keypad m_customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char m_customKey;
String m_keyPadString;

String m_analysisModeLcdString[ANALYSISMODE_NUMBER]={ANALYSISMODE_LCDSTRING_MOD_1, ANALYSISMODE_LCDSTRING_MOD_2, ANALYSISMODE_LCDSTRING_MOD_3};
bool m_refreshScreen=true;

int m_gratingMotorFutureSteps;
int m_gratingMotorCurrentSteps;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;

int m_lambdaMin;
int m_lambdaMax;
int m_lambdaRequested;
int m_lambdaCorrected;
float m_lambdaCaptured;
int m_nReadsRequested;
int m_nReadsCorrected;
int m_nReadsCaptured;
int m_readsVal=0;
float m_sumBackgroundReadsVal=0;
float m_sumSampleReadsVal=0;

int m_backVal=LOW;
int m_nextVal=LOW;
int m_upVal=LOW;
int m_downVal=LOW;
int m_okVal=LOW;

int m_lampSwitchVal;
int m_lampCheckingResult=0;
int m_indexAnalysisMode=0;
int m_backgroundSensorVal;
int m_readSensorVal;
int m_allSpectrumScanID=0;


void setup(void){
	tone(BUZZER_PIN, 500, 100); delay(100);
	tone(BUZZER_PIN, 300, 200); delay(100);
	tone(BUZZER_PIN, 500, 400); delay(4300);
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial) {
		; 												// wait for serial port to connect. Needed for native USB port only
	}
	// serialTest("TEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEST\n");
	#ifdef DEBUG_SERIAL
		Serial.println("[===================================================================]");
		Serial.println("|========================= STARTING SYSTEM =========================|");
		Serial.println("|===================================================================|");
		serialSendInstrumentDetalis();
		Serial.println("[===================================================================]");
		Serial.println("=================== Initializing Connected Devices ==================");
	#endif
	m_lcd.begin(LCD_COLS, LCD_ROWS);
	#ifdef DEBUG_SERIAL
		Serial.println(">> LCD Inizializated;");
	#endif
	m_lcd.setCursor(0, 0); m_lcd.print("Spe.Ar. Project");
	m_lcd.setCursor(0, 1); m_lcd.print("V "); m_lcd.print(MODEL_VERSION); m_lcd.print(" SW "); m_lcd.print(FIRMWARE_VERSION);
	pinMode(BUTTON_PIN_OK, 				INPUT);
	pinMode(BUTTON_PIN_BACK, 			INPUT);
	pinMode(BUTTON_PIN_NEXT, 			INPUT);
	pinMode(BUTTON_PIN_UP, 				INPUT);
	pinMode(BUTTON_PIN_DOWN, 			INPUT);
	pinMode(LAMP_PIN_CHECKINGSENSOR, 	INPUT);
	pinMode(LAMP_PIN_SWITCH, 			OUTPUT);
	digitalWrite(LAMP_PIN_SWITCH, 		HIGH);
	#ifdef DEBUG_SERIAL
		Serial.println(">> I/O Inizializated;");
	#endif
	#ifdef DEBUG_SERIAL
		Serial.print(">> RTC Connection...\t\t\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Initializing");
	m_lcd.setCursor(0, 1); m_lcd.print("RTC...");
	if(m_rtc.begin()){
		m_lcd.print("OK!");
		#ifdef DEBUG_SERIAL
			Serial.println("Established");
		#endif
		if (!m_rtc.isrunning()) {
			#ifdef DEBUG_SERIAL
				Serial.println(">>\t#RTC is NOT running!");
				Serial.println(">>\t#Build timestamp will be set!");
			#endif
			m_rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // following line sets the RTC to the date & time this sketch was compiled
			// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));  // This line sets the RTC with an explicit date & time, for example to set January 21, 2014 at 3am you would call:
		}
		else{
			// m_rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Imposta l'orario in modo rapido scommentando questa riga
			now = m_rtc.now();
			#ifdef DEBUG_SERIAL
				Serial.print(">>\t#Current Time:\t\t\t");
				Serial.print(now.year(), DEC);
				Serial.print('/');
				Serial.print(now.month(), DEC);
				Serial.print('/');
				Serial.print(now.day(), DEC);
				Serial.print(" (");
				Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
				Serial.print(") ");
				Serial.print(now.hour(), DEC);
				Serial.print(':');
				Serial.print(now.minute(), DEC);
				Serial.print(':');
				Serial.println(now.second(), DEC);
			#endif
		}
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Failed");
		#endif
		m_lcd.print("ERR!");
		printWiringError(BUZZER_PIN, m_lcd);
		waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
		waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	}
	AFMS.begin();
	#ifdef DEBUG_SERIAL
		Serial.println(">> Motor Inizializated;");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Starting");
	m_lcd.setCursor(0, 1); m_lcd.print("Instrument...");
	delay(500);
	#ifdef DEBUG_SERIAL
		Serial.print(">> Creating SD Card connection...\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Initializing");
	m_lcd.setCursor(0, 1); m_lcd.print("SD card...");
	// if(SDCardChecking(SD_CHIPSELECT)==SD_CONNECTION_DONE){
	if(SD.begin(SD_CHIPSELECT)){
		m_lcd.print("OK!");
		#ifdef DEBUG_SERIAL
			Serial.println("Established");
			SDinfo();
		#endif
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Failed");
		#endif
		m_lcd.print("ERR!");
		printWiringError(BUZZER_PIN, m_lcd);
		waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
		waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	}
	#ifdef DEBUG_SERIAL
		Serial.print(">> Creating TSL Sensor connection...\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Init.ing TSL...");
	// if(tslSensorInitializationConnection(&m_tsl, m_lcd)==TSL_CONNECTION_DONE){
	if(m_tsl.begin()){
		#ifdef DEBUG_SERIAL
			Serial.println("Established");
		#endif
		m_lcd.setCursor(0, 1); m_lcd.print("	OK!");
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Failed");
		#endif
		m_lcd.setCursor(0, 1); m_lcd.print("	FAILED!");
		printWiringError(BUZZER_PIN, m_lcd);
	}
	serialDisplaySensorDetails(&m_tsl);
	#ifdef DEBUG_SERIAL
		Serial.println("=================== Configuring Connected Devices ===================");
	#endif
	#ifdef DEBUG_SERIAL
		Serial.print(">> Speed Motor Configured:\t\t"); Serial.print(MOTOR_SPEED_RPM); Serial.println("rpm");
	#endif
	m_grtMotor->setSpeed(MOTOR_SPEED_RPM);
	#ifdef DEBUG_SERIAL
		Serial.print(">> Checking Lamp...\t\t\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Lamp");
	m_lcd.setCursor(0, 1); m_lcd.print("Checking");
	m_lampCheckingResult=lampChecking(LAMP_PIN_CHECKINGSENSOR, LAMP_PIN_SWITCH, m_lampSwitchVal, BUZZER_PIN, m_lcd);
	if(m_lampCheckingResult==LAMP_CHECK_PASSED){
		#ifdef DEBUG_SERIAL
			Serial.println("WORKING");
		#endif
		m_lcd.setCursor(0, 1); m_lcd.print("is OK!!         ");
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.print("Error!!:\t"); Serial.println(m_lampCheckingResult);
		#endif
		m_lcd.clear();
		m_lcd.setCursor(0, 0); m_lcd.print("!!ERRORE!!");
		m_lcd.setCursor(0, 1); m_lcd.print("Bin code: "); m_lcd.print(String(m_lampCheckingResult));
		tone(BUZZER_PIN, 400, 1000); delay(500);
		tone(BUZZER_PIN, 400, 1000);
	}
	#ifdef DEBUG_SERIAL
		Serial.println(">> Configuring TSL Sensor...");
	#endif
	tslConfigureSensor(&m_tsl, TSL_GAIN_MAX, TSL_INTEGRATIONTIME_300ms);
	#ifdef DEBUG_SERIAL
		Serial.println(">> Configuring Keypad...");
		Serial.print(">>\t#Debounce Filter:\t\t\t"); 			Serial.println(KEYPAD_ANTIDEBOUNCEFILTER_TIME);
	#endif
	m_customKeypad.setDebounceTime(KEYPAD_ANTIDEBOUNCEFILTER_TIME);
	#ifdef DEBUG_SERIAL
		Serial.println("===================================================================");
		Serial.println("===================== Initialization COMPLETED ====================");
		Serial.print("============================= "); Serial.print(millis()/1000.0, 3); Serial.println("s ==============================");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Instrument");
	m_lcd.setCursor(0, 1); m_lcd.print("is Ready!!!!");			//Migliorare l'effetto felice, ora sembra che semplicemente funzioni male
	tone(BUZZER_PIN, 600, 200); delay(200);
	tone(BUZZER_PIN, 600, 200); delay(100);
	tone(BUZZER_PIN, 900, 400); delay(1000);
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Press 'OK'");
	m_lcd.setCursor(0, 1); m_lcd.print("to Start...");
	while(!m_okVal){
		m_okVal=digitalRead(BUTTON_PIN_OK);
	}
	waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	#ifdef DEBUG_SERIAL
		Serial.println(">> Entering 'Selection Mode' Menu");
	#endif
	delay(200);
}

void loop(void){
	m_customKey=0;
	if(m_refreshScreen){
		m_lcd.clear();
		m_lcd.setCursor(0, 0); m_lcd.print("Sel. Analysis:");
	}
	/*MODES:
		1	-	SIMPLE READ
		2	-	ALL SPECTRUM
		3	-	CONCANALYSIS
	*/
	while(!m_okVal && !m_backVal){
		m_okVal=	digitalRead(BUTTON_PIN_OK);
		m_backVal=	digitalRead(BUTTON_PIN_BACK);
		m_upVal=	digitalRead(BUTTON_PIN_UP);
		m_downVal=	digitalRead(BUTTON_PIN_DOWN);
		if(!m_upVal&&m_downVal){
			if(m_indexAnalysisMode>=2){
				m_indexAnalysisMode=0;
			}
			else{
				m_indexAnalysisMode++;
			}
			while(m_downVal){
				m_downVal=digitalRead(BUTTON_PIN_DOWN);
			}
			m_refreshScreen=true;
		}
		else if(m_upVal&&!m_downVal){
			if(m_indexAnalysisMode<=0){
				m_indexAnalysisMode=2;
			}
			else{
				m_indexAnalysisMode--;
			}
			while(m_upVal){
				m_upVal=digitalRead(BUTTON_PIN_UP);
			}
			m_refreshScreen=true;
		}
		if(m_refreshScreen){
			m_lcd.setCursor(0, 1); m_lcd.print("                ");
			m_lcd.setCursor(0, 1); m_lcd.print(String(m_indexAnalysisMode+1) + "-" + m_analysisModeLcdString[m_indexAnalysisMode]);
			m_refreshScreen=false;
		}
	}
	waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	// m_okVal=0;
	delay(200);
	#ifdef DEBUG_SERIAL
		Serial.print(">> Mode Selected:\t\t"); Serial.print(m_indexAnalysisMode); Serial.print(" - "); Serial.println(m_analysisModeLcdString[m_indexAnalysisMode]);
		Serial.println(">> Waiting confirm...");
	#endif
	switch(m_indexAnalysisMode){
		case(ANALYSISMODE_SIMPLEREAD):{
			m_lcd.clear();
			m_lcd.setCursor(0, 0); m_lcd.print("Simple Read");
			m_lcd.setCursor(0, 1); m_lcd.print("selected?");
			while(!m_okVal && !m_backVal){
				m_okVal=digitalRead(BUTTON_PIN_OK);
				m_backVal=digitalRead(BUTTON_PIN_BACK);
				if(m_okVal){
					#ifdef DEBUG_SERIAL
						Serial.println(">> Simple Read Selected!");
					#endif
					waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Lambda (nm): ");
					m_lcd.setCursor(0, 1); m_lcd.print("Set Val: ");
					#ifdef DEBUG_SERIAL
						Serial.print(">> Lambda requested(nm):\t\t");
					#endif
					m_keyPadString="";
					while(!m_okVal){
						m_okVal=digitalRead(BUTTON_PIN_OK);
						m_customKey=0;
						m_customKey=m_customKeypad.getKey();
						// delay(KEYPAD_ANTIDEBUNCEFILTER);
						if(m_customKey){
							m_keyPadString+=m_customKey;
							m_lcd.setCursor(0, 1); m_lcd.print("Set Val: " + m_keyPadString);
							#ifdef DEBUG_SERIAL
								Serial.print(m_customKey);
							#endif
						}
					}
					waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
					#ifdef DEBUG_SERIAL
						Serial.println();
					#endif
					m_lambdaRequested=m_keyPadString.toInt();
					delay(200);
					m_lambdaCorrected=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Lambda: ");
					m_lcd.setCursor(0, 1); m_lcd.print(String(m_lambdaCorrected) + "nm");
					#ifdef DEBUG_SERIAL
						if(m_lambdaCorrected<m_lambdaRequested){
							Serial.print(">> Lambda can't be lower then "); Serial.print(SPECTRALIMIT_LOW); Serial.print("nm. Corrected to that value. Requested was "); Serial.println(m_lambdaRequested);
						}
						else if (m_lambdaCorrected>m_lambdaRequested){
							Serial.print(">> Lambda can't be higher then "); Serial.print(SPECTRALIMIT_HIGH); Serial.print("nm. Corrected to that value. Requested was "); Serial.println(m_lambdaRequested); 
						}
						else{
							Serial.println(">> - Lambda Acceptded!!");
						}
					#endif
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Set Replicates");
					m_lcd.setCursor(0, 1); m_lcd.print("n: ");
					#ifdef DEBUG_SERIAL
						Serial.print(">> Replicates requested:\t\t");
					#endif
					m_keyPadString="";
					while(!m_okVal){
						m_okVal=digitalRead(BUTTON_PIN_OK);
						m_customKey=0;
						m_customKey = m_customKeypad.getKey();
						// delay(100);
						if(m_customKey){
							m_keyPadString+=m_customKey;
							m_lcd.setCursor(0, 1); m_lcd.print("n: " + m_keyPadString);
							#ifdef DEBUG_SERIAL
								Serial.print(m_customKey);
							#endif
						}
					}
					waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
					#ifdef DEBUG_SERIAL
						Serial.println();
					#endif
					m_nReadsRequested=m_keyPadString.toInt();
					if(m_nReadsRequested<MIN_REPLICATES){
						m_nReadsCorrected=MIN_REPLICATES;
						#ifdef DEBUG_SERIAL
							Serial.print(">> Replicates can't be lower then "); Serial.print(MIN_REPLICATES); Serial.print(". Corrected to that value. Requested was "); Serial.print(m_nReadsRequested);
						#endif
					}
					else{
						m_nReadsCorrected=m_nReadsRequested;
						#ifdef DEBUG_SERIAL
							Serial.println(">> - Replicates Acceptded!!");
						#endif
					}
					#ifdef DEBUG_SERIAL
						Serial.println(">> Positioning motor...");
					#endif
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Replicates: " + String(m_nReadsCorrected));
					m_lcd.setCursor(0, 1); m_lcd.print("Pos.ing motor...");
					m_gratingMotorFutureSteps=map(m_lambdaCorrected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_LOW);
					gratingMotorZeroPoint(MOTOR_PIN_POSITIONSENSOR, BUZZER_PIN, m_lcd, m_grtMotor);			//serve davvero?
					m_grtMotor->step(m_gratingMotorFutureSteps, FORWARD, MOTOR_STEPTYPE);
					m_grtMotor->release();
					#ifdef DEBUG_SERIAL
						Serial.println(">> Positioning motor done! Waiting confirm to proceed...");
					#endif
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Press 'OK' to");
					m_lcd.setCursor(0, 1); m_lcd.print("AutoZero");
					//rivedere
					while(!m_okVal){
						m_okVal=digitalRead(BUTTON_PIN_OK);
					}
					m_okVal=LOW;
					#ifdef DEBUG_SERIAL
						Serial.println(">> Reading blank...");
					#endif
					m_nReadsCaptured=0;
					m_sumBackgroundReadsVal=0;
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Reading...");
					m_lcd.setCursor(0, 1); m_lcd.print("Index: ");
					for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
						m_lcd.setCursor(7, 1); m_lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
						// simpleRead(tsl);
						m_readsVal=simpleRead(&m_tsl, TSL_READTYPE_VISIBLE);
						m_sumBackgroundReadsVal+=m_readsVal;
						#ifdef DEBUG_SERIAL
							Serial.print(">> "); Serial.print(m_readsVal);
						#endif
					}
					#ifdef DEBUG_SERIAL
						Serial.println();
					#endif
					m_nReadsCaptured=0;
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Load Sample!");
					m_lcd.setCursor(0, 1); m_lcd.print("'OK' to read...");
					#ifdef DEBUG_SERIAL
						Serial.println(">> Blank reads done! Waiting confirm to proceed to Sample...");
					#endif
					delay(200);
					while(!m_okVal && !m_backVal){
						m_okVal=digitalRead(BUTTON_PIN_OK);
						m_backVal=digitalRead(BUTTON_PIN_BACK);
						if(m_okVal){
							m_okVal=LOW;
							#ifdef DEBUG_SERIAL
								Serial.println(">> Reading Sample...");
							#endif
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("Reading...");
							m_lcd.setCursor(0, 1); m_lcd.print("Index: ");
							m_nReadsCaptured=0;
							m_sumSampleReadsVal=0;
							for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
								m_lcd.setCursor(7, 1); m_lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
								// simpleRead();
								m_readsVal=simpleRead(&m_tsl, TSL_READTYPE_VISIBLE);
								m_sumSampleReadsVal+=m_readsVal;
								#ifdef DEBUG_SERIAL
									Serial.print(">> "); Serial.print(m_readsVal);
								#endif
							}
							#ifdef DEBUG_SERIAL
								Serial.println();
							#endif
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("--LOADING DATA-");
							m_lcd.setCursor(0, 1); m_lcd.print("...PLEAS WAIT...");
							#ifdef DEBUG_SERIAL
								Serial.println(">> Sample reads done! Calculating results...");
							#endif
							//float iSource=m_sumBackgroundReadsVal/(float)m_nReadsCorrected;
							//float iSample=((m_sumSampleReadsVal/(float)m_nReadsCorrected));
							//float trasmittance=iSample/iSource;
							//float Abs=log10(1/(iSample/iSource))*1000;
							float trasmittance	=	(m_sumSampleReadsVal/(float)m_nReadsCorrected)/(m_sumBackgroundReadsVal/(float)m_nReadsCorrected);
							float absorbance	=	log10(1/((((m_sumSampleReadsVal/(float)m_nReadsCorrected)/(m_sumBackgroundReadsVal/(float)m_nReadsCorrected))))); //verificare il 1000
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("Abs Sample:");
							m_lcd.setCursor(0, 1); m_lcd.print(String(absorbance,DECIMAL_LCD_ABSORBANCE));
							#ifdef DEBUG_SERIAL
								Serial.println(">> [---------------------------------------------------]");
								Serial.print(">> [ Number of reads:\t\t");	Serial.println(m_nReadsCorrected);
								Serial.print(">> [ Background Average:\t");	Serial.println((m_sumBackgroundReadsVal)/(m_nReadsCorrected));
								Serial.print(">> [ Sample Average:\t\t");	Serial.println((m_sumSampleReadsVal)/(m_nReadsCorrected));
								Serial.print(">> [ Trasmittance:\t\t");		Serial.println(trasmittance,DECIMAL_SERIAL_TRASMITTANCE);
								Serial.print(">> [ Absorbance:\t\t");		Serial.println(absorbance,DECIMAL_SERIAL_ABSORBANCE);
								//Serial.print(">> [ iSource:\t\t\t");		Serial.println(iSource);
								//Serial.print(">> [ iSample:\t\t\t");		Serial.println(iSample);
								//Serial.print(">> [ Abs:\t\t\t");			Serial.print(Abs); 										Serial.println("*10^-3.");
								Serial.println(">> [---------------------------------------------------]");
							#endif
						}
						//m_okVal=LOW;
					}
					m_backVal=LOW;
				}
			}
			m_backVal=LOW;
			m_refreshScreen=true;
			break;
		}
		case(ANALYSISMODE_ALLSPECTRUM):{
			m_lcd.clear();
			m_lcd.setCursor(0, 0); m_lcd.print("All Spectrum");
			m_lcd.setCursor(0, 1); m_lcd.print("selected?");
			while(!m_okVal && !m_backVal){
				m_okVal=digitalRead(BUTTON_PIN_OK);
				m_backVal=digitalRead(BUTTON_PIN_BACK);
				if(m_okVal){
					m_okVal=LOW;
					delay(1000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Spectrum range");
					m_lcd.setCursor(0, 1); m_lcd.print("Set MIN:  ");
					m_keyPadString="";
					while(m_okVal==LOW){
						m_customKey=0;
						m_customKey = m_customKeypad.getKey();
						// delay(100);
						if(m_customKey){
							m_keyPadString=m_keyPadString+m_customKey;
							m_lcd.setCursor(0, 1); m_lcd.print("Set MIN:  " + m_keyPadString);
						}
						m_okVal=digitalRead(BUTTON_PIN_OK);
					}
					m_lambdaRequested=m_keyPadString.toInt();
					m_lambdaMin=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					m_okVal=LOW;
					delay(1000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Spectrum range");
					m_lcd.setCursor(0, 1); m_lcd.print("Set MAX:  ");
					m_keyPadString="";
					while(m_okVal==LOW){
						m_customKey=0;
						m_customKey = m_customKeypad.getKey();
						// delay(100);
						if(m_customKey){
							m_keyPadString=m_keyPadString+m_customKey;
							m_lcd.setCursor(0, 1); m_lcd.print("Set MAX:  " + m_keyPadString);
						}
						m_okVal=digitalRead(BUTTON_PIN_OK);
					}
					m_lambdaRequested=m_keyPadString.toInt();
					m_lambdaMax=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					m_okVal=LOW;
					delay(1000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("MIN:  "); m_lcd.print(m_lambdaMin);
					m_lcd.setCursor(0, 1); m_lcd.print("MAX:  "); m_lcd.print(m_lambdaMax);
					delay(1000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Grating Motor");
					m_lcd.setCursor(0, 1); m_lcd.print("Zero setting...");
					gratingMotorZeroPoint(MOTOR_PIN_POSITIONSENSOR, BUZZER_PIN, m_lcd, m_grtMotor);
					delay(1000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Set Replicates");
					m_lcd.setCursor(0, 1); m_lcd.print("n:  ");
					m_keyPadString="";
					while(m_okVal==LOW){
						m_customKey=0;
						m_customKey = m_customKeypad.getKey();
						// delay(100);
						if(m_customKey){
							m_keyPadString=m_keyPadString+m_customKey;
							m_lcd.setCursor(0, 1); m_lcd.print("n:  " + m_keyPadString);
						}
						m_okVal=digitalRead(BUTTON_PIN_OK);
					}
					m_nReadsCorrected=m_keyPadString.toInt();
					m_okVal=LOW;
					delay(1000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Replicates:  ");
					m_lcd.setCursor(0, 1); m_lcd.print(m_nReadsCorrected);
					delay(3000);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Press 'OK' to");
					m_lcd.setCursor(0, 1); m_lcd.print("AutoZero");
					while(m_okVal==LOW){
						m_okVal=digitalRead(BUTTON_PIN_OK);
					}
					m_okVal=LOW;
					backgroundSensor(m_lcd, m_allSpectrumFile);
					m_lcd.clear();
					m_lcd.setCursor(0, 0); m_lcd.print("Load Sample!");
					m_lcd.setCursor(0, 1); m_lcd.print("'OK' to read...");
					m_allSpectrumScanID=0;
					delay(1000);
					while(m_backVal==LOW){
						m_okVal=digitalRead(BUTTON_PIN_OK);
						m_backVal=digitalRead(BUTTON_PIN_BACK);
						if(m_okVal==HIGH){
							m_okVal=LOW;
							m_allSpectrumScanID++;
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("Reading...");
							delay(1000);
							//	>> ======================= TODO ======================= << 
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("Spectrum " + m_allSpectrumScanID);
							m_lcd.setCursor(0, 1); m_lcd.print("Saved!");
						}
					}
					m_backVal=LOW;
				}
			}
			m_backVal=LOW;
			m_refreshScreen=true;
			break;				
		}
		case(ANALYSISMODE_CONCANALYSIS):{
			m_lcd.clear();
			m_lcd.setCursor(0, 0); m_lcd.print("Conc. Analysis");
			m_lcd.setCursor(0, 1); m_lcd.print("selected?");
			while((m_okVal==LOW) && (m_backVal==LOW)){
				m_okVal=digitalRead(BUTTON_PIN_OK);
				m_backVal=digitalRead(BUTTON_PIN_BACK);
				if(m_okVal){
					m_okVal=LOW;
					//	>> ======================= TODO ======================= << 
				}
			}
			m_backVal=LOW;
			m_refreshScreen=true;
			break;
		}
		
	}
	#ifdef DEBUG_SERIAL
		Serial.println(">> Returning to the selection mode menu");
	#endif
}

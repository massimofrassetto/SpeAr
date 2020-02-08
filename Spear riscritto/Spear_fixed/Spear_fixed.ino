/* PROGRAMMA PRINCIPALE */

#define DEBUG

#include <LiquidCrystal.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include <Adafruit_MotorShield.h>

#include <SPI.h>
#include <SD.h>

#include <Keypad.h>

#include "CONSTANTS.h"
#include "MSG.h"

Adafruit_TSL2591 tsl = Adafruit_TSL2591(ADAFRUIT_SENSOR_IDENTIFIER); // pass in a number for the sensor identifier (for your use later)

LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_ENABLE, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(MOTOR_STEPS_PER_REVOLUTION, MOTOR_PORT);

File allSpectrumFile;

const byte ROWS = KEYPAD_ROWS;
const byte COLS = KEYPAD_COLS;
char hexaKeys[ROWS][COLS] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};
byte rowPins[ROWS] = {PIN_KEYPAD_ROW_0, PIN_KEYPAD_ROW_1, PIN_KEYPAD_ROW_2, PIN_KEYPAD_ROW_3};
byte colPins[COLS] = {PIN_KEYPAD_COLS_0, PIN_KEYPAD_COLS_1, PIN_KEYPAD_COLS_2};
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char m_customKey;
String m_keyPadString;

String m_analysisModeLcdString[ANALYSISMODE_NUMBER]={ANALYSISMODE_LCDSTRING_MOD_1, ANALYSISMODE_LCDSTRING_MOD_2, ANALYSISMODE_LCDSTRING_MOD_3};
bool m_refreshScreen=true;

int m_gratingMotorFutureSteps;
int m_gratingMotorCurrentSteps;
int m_lambdaMin;
int m_lambdaMax;
int m_lambdaRequested;
int m_lambdaCorrected;
float m_lambdaCaptured;
int m_nReadsRequested;
int m_nReadsCorrected;
int m_nReadsCaptured;
int m_readsVal=0;
int m_sumReadsVal=0;
float m_sumBackgroundReadsVal=0;
float m_sumSampleReadsVal=0;

int m_backVal=LOW;
int m_nextVal=LOW;
int m_upVal=LOW;
int m_downVal=LOW;
int m_okVal=LOW;

int m_indexAnalysisMode=0;
int m_lampSwitchVal;
int m_backgroundSensorVal;
int m_readSensorVal;
int m_allSpectrumScanID=0;

#include "FUNCTIONS.h"

void setup(void){
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}
	#ifdef DEBUG
		Serial.println("=====================================");
		Serial.println("========== STARTING SYSTEM ==========");
		Serial.println("=====================================");
		Serial.print("========== Model:\t"); 						Serial.println(MODEL_VERSION);
		Serial.print("========== Firmware:\t"); 					Serial.println(FIRMWARE_VERSION);
		Serial.println("=====================================");
	#endif
	AFMS.begin();
	myMotor->setSpeed(MOTOR_SPEED_RPM);
	#ifdef DEBUG
		Serial.println(">> Motor Inizializated;");
	#endif
	pinMode(PIN_BUTTON_OK, 				INPUT);
	pinMode(PIN_BUTTON_BACK, 			INPUT);
	pinMode(PIN_BUTTON_NEXT, 			INPUT);
	pinMode(PIN_BUTTON_UP, 				INPUT);
	pinMode(PIN_BUTTON_DOWN, 			INPUT);
	pinMode(PIN_LAMP_CHECKINGSENSOR, 	INPUT);
	pinMode(PIN_LAMP_SWITCH, 			OUTPUT);
	digitalWrite(PIN_LAMP_SWITCH, 		HIGH);
	#ifdef DEBUG
		Serial.println(">> I/O Inizializated;");
	#endif
	lcd.begin(LCD_COLS, LCD_ROWS);
	#ifdef DEBUG
		Serial.println(">> LCD Inizializated;");
	#endif
	lcd.setCursor(0, 0); lcd.print("Spe.Ar. Project");
	lcd.setCursor(0, 1); lcd.print("V "); lcd.print(MODEL_VERSION); lcd.print(" SW "); lcd.print(FIRMWARE_VERSION);
	#ifdef DEBUG
		Serial.println(">> Motor Inizializated;");
	#endif
	tone(PIN_BUZZER, 500, 100); delay(100);
	tone(PIN_BUZZER, 300, 200); delay(100);
	tone(PIN_BUZZER, 500, 400); delay(4300);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Starting");
	lcd.setCursor(0, 1); lcd.print("Instrument...");
	#ifdef DEBUG
		Serial.println("======= Starting Instrument =======");
	#endif
	#ifdef DEBUG
		Serial.println(">> Checking Lamp...\t\t");
	#endif
	lampChecking(PIN_LAMP_CHECKINGSENSOR, PIN_LAMP_SWITCH, &m_lampSwitchVal, PIN_BUZZER);
	#ifdef DEBUG
		Serial.println(">> Checking TSL Sensor...\t");
	#endif
	tslSensorChecking();
	#ifdef DEBUG
		Serial.println(">> SD Card Checking...\t");
	#endif
	SDCardChecking(CHIPSELECT);
	#ifdef DEBUG
		Serial.println(">> Homing Grating...\t\t");
	#endif
	gratingMotorChecking(PIN_MOTOR_POSITIONSENSOR, PIN_BUZZER);
	m_gratingMotorCurrentSteps=0;
	#ifdef DEBUG
		Serial.println("======= Initialization COMPLETED =======");
		Serial.print("================ "); Serial.print(millis()/1000.0); Serial.println("s =================");
	#endif
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Instrument");
	lcd.setCursor(0, 1); lcd.print("is Ready!!!!");
	tone(PIN_BUZZER, 600, 200); delay(200);
	tone(PIN_BUZZER, 600, 200); delay(100);
	tone(PIN_BUZZER, 900, 400); delay(1000);
	lcd.noDisplay();	delay(500);
	lcd.display();		delay(2000);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Press 'OK'");
	lcd.setCursor(0, 1); lcd.print("to Start...");
	while(!m_okVal){
		m_okVal=digitalRead(PIN_BUTTON_OK);
	}
	m_okVal=LOW;
	#ifdef DEBUG
		Serial.println(">> Entering Selection Mode Menu");
	#endif
	delay(200);
}

void loop(void){
	m_customKey=0;
	if(m_refreshScreen){
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("Sel. Analysis:");
	}
	/*MODES:
		1	-	SIMPLE READ
		2	-	ALL SPECTRUM
		3	-	CONCANALYSIS
	*/
	while(!m_okVal && !m_backVal){
		m_okVal	=digitalRead(PIN_BUTTON_OK);
		m_backVal	=digitalRead(PIN_BUTTON_BACK);
		m_upVal	=digitalRead(PIN_BUTTON_UP);
		m_downVal	=digitalRead(PIN_BUTTON_DOWN);
		if(!m_upVal&&m_downVal){
			if(m_indexAnalysisMode>=2){
				m_indexAnalysisMode=0;
			}
			else{
				m_indexAnalysisMode++;
			}
			while(m_downVal){
				m_downVal=digitalRead(PIN_BUTTON_DOWN);
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
				m_upVal=digitalRead(PIN_BUTTON_UP);
			}
			m_refreshScreen=true;
		}
		if(m_refreshScreen){
			lcd.setCursor(0, 1); lcd.print("                ");
			lcd.setCursor(0, 1); lcd.print(String(m_indexAnalysisMode+1) + "-" + m_analysisModeLcdString[m_indexAnalysisMode]);
			m_refreshScreen=false;
		}
	}
	m_okVal=0;
	delay(200);
	#ifdef DEBUG
		Serial.print(">> Mode Selected: "); Serial.print(m_indexAnalysisMode); Serial.print(" - "); Serial.println(m_analysisModeLcdString[m_indexAnalysisMode]);
		Serial.println(">> Waiting confirm...");
	#endif
	switch(m_indexAnalysisMode){
		case(ANALYSISMODE_SIMPLEREAD):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("Simple Read");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while(!m_okVal && !m_backVal){
				m_okVal=digitalRead(PIN_BUTTON_OK);
				m_backVal=digitalRead(PIN_BUTTON_BACK);
				if(m_okVal){
					#ifdef DEBUG
						Serial.println(">> Simple Read Selected!");
					#endif
					m_okVal=LOW;
					delay(100);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Lambda (nm): ");
					lcd.setCursor(0, 1); lcd.print("Set Val: ");
					#ifdef DEBUG
						Serial.print(">> Lambda requested:\t\t");
					#endif
					m_keyPadString="";
					while(!m_okVal){
						m_okVal=digitalRead(PIN_BUTTON_OK);
						m_customKey=0;
						m_customKey=customKeypad.getKey();
						delay(200);
						if(m_customKey){
							m_keyPadString+=m_customKey;
							lcd.setCursor(0, 1); lcd.print("Set Val: " + m_keyPadString + "nm");
							#ifdef DEBUG
								Serial.print(m_customKey);
							#endif
						}
					}
					m_okVal=LOW;
					#ifdef DEBUG
						Serial.println();
					#endif
					m_lambdaRequested=m_keyPadString.toInt();
					delay(200);
					m_lambdaCorrected=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Lambda: ");
					lcd.setCursor(0, 1); lcd.print(String(m_lambdaCorrected) + "nm");
					#ifdef DEBUG
						if(m_lambdaCorrected<m_lambdaRequested){
							Serial.print(">> Lambda can't be lower then "); Serial.print(SPECTRALIMIT_LOW); Serial.print("nm. Corrected to that value. Requested was "); Serial.print(m_lambdaRequested);
						}
						else if (m_lambdaCorrected>m_lambdaRequested){
							Serial.print(">> Lambda can't be higher then "); Serial.print(SPECTRALIMIT_HIGH); Serial.print("nm. Corrected to that value. Requested was "); Serial.print(m_lambdaRequested); 
						}
						else{
							Serial.println(">> Lambda Acceptded!!");
						}
					#endif
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Set Replicates");
					lcd.setCursor(0, 1); lcd.print("n: ");
					#ifdef DEBUG
						Serial.print(">> Replicates requested:\t");
					#endif
					m_keyPadString="";
					while(!m_okVal){
						m_okVal=digitalRead(PIN_BUTTON_OK);
						m_customKey=0;
						m_customKey = customKeypad.getKey();
						delay(100);
						if(m_customKey){
							m_keyPadString+=m_customKey;
							lcd.setCursor(0, 1); lcd.print("n: " + m_keyPadString);
							#ifdef DEBUG
								Serial.print(m_customKey);
							#endif
						}
					}
					m_okVal=LOW;
					#ifdef DEBUG
						Serial.println(".");
					#endif
					m_nReadsRequested=m_keyPadString.toInt();
					if(m_nReadsRequested<MIN_REPLICATES){
						m_nReadsCorrected=MIN_REPLICATES;
						#ifdef DEBUG
							Serial.print(">> Replicates can't be lower then "); Serial.print(MIN_REPLICATES); Serial.print(". Corrected to that value. Requested was "); Serial.print(m_nReadsRequested);
						#endif
					}
					else{
						m_nReadsCorrected=m_nReadsRequested;
						#ifdef DEBUG
							Serial.println(">> Replicates Acceptded!!");
						#endif
					}
					#ifdef DEBUG
						Serial.println(">> Positioning motor...");
					#endif
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Replicates: " + String(m_nReadsCorrected));
					lcd.setCursor(0, 1); lcd.print("Pos.ing motor...");
					m_gratingMotorFutureSteps=map(m_lambdaCorrected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_LOW);
					gratingMotorChecking(PIN_MOTOR_POSITIONSENSOR, PIN_BUZZER);			//serve davvero?
					myMotor->step(m_gratingMotorFutureSteps, FORWARD, SINGLE); 
					#ifdef DEBUG
						Serial.println(">> Positioning motor done! Waiting confirm to proceed...");
					#endif
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Press 'OK' to");
					lcd.setCursor(0, 1); lcd.print("AutoZero");
					//rivedere
					while(!m_okVal){
						m_okVal=digitalRead(PIN_BUTTON_OK);
					}
					m_okVal=LOW;
					#ifdef DEBUG
						Serial.println(">> Reading blank...");
					#endif
					m_nReadsCaptured=0;
					m_sumBackgroundReadsVal=0;
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Reading...");
					lcd.setCursor(0, 1); lcd.print("Index: ");
					for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
						lcd.setCursor(7, 1); lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
						// simpleRead();
						m_readsVal=simpleRead(TSL_READTYPE_VISIBLE);
						m_sumBackgroundReadsVal+=m_readsVal;
						Serial.print(">> "); Serial.println(m_readsVal);
					}
					m_nReadsCaptured=0;
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Load Sample!");
					lcd.setCursor(0, 1); lcd.print("'OK' to read...");
					#ifdef DEBUG
						Serial.println(">> Blank reads done! Waiting confirm to proceed to Sample...");
					#endif
					delay(200);
					while(!m_okVal && !m_backVal){
						m_okVal=digitalRead(PIN_BUTTON_OK);
						m_backVal=digitalRead(PIN_BUTTON_BACK);
						if(m_okVal){
							m_okVal=LOW;
							#ifdef DEBUG
								Serial.println(">> Reading Sample...");
							#endif
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Reading...");
							lcd.setCursor(0, 1); lcd.print("Index: ");
							m_nReadsCaptured=0;
							m_sumSampleReadsVal=0;
							for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
								lcd.setCursor(7, 1); lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
								// simpleRead();
								m_readsVal=simpleRead(TSL_READTYPE_VISIBLE);
								m_sumBackgroundReadsVal+=m_readsVal;
								Serial.print(">> "); Serial.println(m_readsVal);
							}
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("--LOADING DATA-");
							lcd.setCursor(0, 1); lcd.print("...PLEAS WAIT...");
							#ifdef DEBUG
								Serial.println(">> Sample reads done! Calculating results...");
							#endif
							//float iSource=m_sumBackgroundReadsVal/(float)m_nReadsCorrected;
							//float iSample=((m_sumSampleReadsVal/(float)m_nReadsCorrected));
							//float trasmittance=iSample/iSource;
							//float Abs=log10(1/(iSample/iSource))*1000;
							float trasmittance	=	(m_sumSampleReadsVal/(float)m_nReadsCorrected)/(m_sumBackgroundReadsVal/(float)m_nReadsCorrected);
							float absorbance	=	log10(1/((((m_sumSampleReadsVal/(float)m_nReadsCorrected)/(m_sumBackgroundReadsVal/(float)m_nReadsCorrected))))); //verificare il 1000
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Abs Sample:");
							lcd.setCursor(0, 1); lcd.print(String(absorbance,DECIMAL_LCD_ABSORBANCE));
							#ifdef DEBUG
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
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("All Spectrum");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while(!m_okVal && !m_backVal){
				m_okVal=digitalRead(PIN_BUTTON_OK);
				m_backVal=digitalRead(PIN_BUTTON_BACK);
				if(m_okVal){
					m_okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Spectrum range");
					lcd.setCursor(0, 1); lcd.print("Set MIN:  ");
					m_keyPadString="";
					while(m_okVal==LOW){
						m_customKey=0;
						m_customKey = customKeypad.getKey();
						delay(100);
						if(m_customKey){
							m_keyPadString=m_keyPadString+m_customKey;
							lcd.setCursor(0, 1); lcd.print("Set MIN:  " + m_keyPadString);
						}
						m_okVal=digitalRead(PIN_BUTTON_OK);
					}
					m_lambdaRequested=m_keyPadString.toInt();
					m_lambdaMin=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					m_okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Spectrum range");
					lcd.setCursor(0, 1); lcd.print("Set MAX:  ");
					m_keyPadString="";
					while(m_okVal==LOW){
						m_customKey=0;
						m_customKey = customKeypad.getKey();
						delay(100);
						if(m_customKey){
							m_keyPadString=m_keyPadString+m_customKey;
							lcd.setCursor(0, 1); lcd.print("Set MAX:  " + m_keyPadString);
						}
						m_okVal=digitalRead(PIN_BUTTON_OK);
					}
					m_lambdaRequested=m_keyPadString.toInt();
					m_lambdaMax=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					m_okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("MIN:  "); lcd.print(m_lambdaMin);
					lcd.setCursor(0, 1); lcd.print("MAX:  "); lcd.print(m_lambdaMax);
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Grating Motor");
					lcd.setCursor(0, 1); lcd.print("Zero setting...");
					gratingMotorZeroPoint(PIN_MOTOR_POSITIONSENSOR, PIN_BUZZER);
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Set Replicates");
					lcd.setCursor(0, 1); lcd.print("n:  ");
					m_keyPadString="";
					while(m_okVal==LOW){
						m_customKey=0;
						m_customKey = customKeypad.getKey();
						delay(100);
						if(m_customKey){
							m_keyPadString=m_keyPadString+m_customKey;
							lcd.setCursor(0, 1); lcd.print("n:  " + m_keyPadString);
						}
						m_okVal=digitalRead(PIN_BUTTON_OK);
					}
					m_nReadsCorrected=m_keyPadString.toInt();
					m_okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Replicates:  ");
					lcd.setCursor(0, 1); lcd.print(m_nReadsCorrected);
					delay(3000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Press 'OK' to");
					lcd.setCursor(0, 1); lcd.print("AutoZero");
					while(m_okVal==LOW){
						m_okVal=digitalRead(PIN_BUTTON_OK);
					}
					m_okVal=LOW;
					backgroundSensor();
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Load Sample!");
					lcd.setCursor(0, 1); lcd.print("'OK' to read...");
					m_allSpectrumScanID=0;
					delay(1000);
					while(m_backVal==LOW){
						m_okVal=digitalRead(PIN_BUTTON_OK);
						m_backVal=digitalRead(PIN_BUTTON_BACK);
						if(m_okVal==HIGH){
							m_okVal=LOW;
							m_allSpectrumScanID++;
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Reading...");
							delay(1000);
							//	>> ======================= TODO ======================= << 
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Spectrum " + m_allSpectrumScanID);
							lcd.setCursor(0, 1); lcd.print("Saved!");
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
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("Conc. Analysis");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while((m_okVal==LOW) && (m_backVal==LOW)){
				m_okVal=digitalRead(PIN_BUTTON_OK);
				m_backVal=digitalRead(PIN_BUTTON_BACK);
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
	#ifdef DEBUG
		Serial.println(">> Returning to the selection mode menu");
	#endif
}

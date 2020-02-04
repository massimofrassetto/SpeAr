/* Da usar in futuro:
	https://github.com/cubiwan/LinearRegressino
*/

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
char customKey;
String keyPadString;

String analysisModeLcdString[ANALYSISMODE_NUMBER]={ANALYSISMODE_LCDSTRING_MOD_1, ANALYSISMODE_LCDSTRING_MOD_2, ANALYSISMODE_LCDSTRING_MOD_3};
bool refreshScreen=true;

int gratingMotorFutureSteps;
int gratingMotorCurrentSteps;
int lambdaMin;
int lambdaMax;
int lambdaSelected;
int lambdaCorrected;
float lambdaCaptured;
int nReplicates;
int nReads;
int readsVal=0;
int sumReadsVal=0;
float sumBackgroundReadsVal=0;
float sumSampleReadsVal=0;

int backVal=LOW;
int nextVal=LOW;
int upVal=LOW;
int downVal=LOW;
int okVal=LOW;
int lampSwitchVal;
int backgroundSensorVal;
int readSensorVal;
int x=0;	//definire funzione...
int indexAnalysisMode=0;

#include "FUNCTIONS.h"

void setup(void){
	Serial.begin(SERIAL_BAUDRATE);
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
	tone(PIN_PIEZO, 500, 100); delay(100);
	tone(PIN_PIEZO, 300, 200); delay(100);
	tone(PIN_PIEZO, 500, 400); delay(4300);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Starting");
	lcd.setCursor(0, 1); lcd.print("Instrument...");
	#ifdef DEBUG
		Serial.println("======= Starting Instrument =======");
	#endif
	#ifdef DEBUG
		Serial.println(">> Checking Lamp...\t\t");
	#endif
	lampChecking(PIN_LAMP_CHECKINGSENSOR, PIN_LAMP_SWITCH, &lampSwitchVal, PIN_PIEZO);
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
	gratingMotorChecking(PIN_MOTOR_POSITIONSENSOR, PIN_PIEZO);
	gratingMotorCurrentSteps=0;
	#ifdef DEBUG
		Serial.println("======= Initialization COMPLETED =======");
		Serial.print("================ "); Serial.print(millis()/1000); Serial.println("s =================");
	#endif
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Instrument");
	lcd.setCursor(0, 1); lcd.print("is Ready!!!!");
	tone(PIN_PIEZO, 600, 200); delay(200);
	tone(PIN_PIEZO, 600, 200); delay(100);
	tone(PIN_PIEZO, 900, 400); delay(1000);
	lcd.noDisplay();	delay(500);
	lcd.display();		delay(2000);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Press 'OK'");
	lcd.setCursor(0, 1); lcd.print("to Start...");
	while(!okVal){
		okVal=digitalRead(PIN_BUTTON_OK);
	}
	okVal=LOW;
	#ifdef DEBUG
		Serial.println(">> Entering Selection Mode Menu");
	#endif
	delay(200);
}

void loop(void){
	customKey=0;
	if(refreshScreen){
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("Sel. Analysis:");
	}
	/*MODES:
		1	-	SIMPLE READ
		2	-	ALL SPECTRUM
		3	-	CONCANALYSIS
	*/
	while(!okVal && !backVal){
		okVal	=digitalRead(PIN_BUTTON_OK);
		backVal	=digitalRead(PIN_BUTTON_BACK);
		upVal	=digitalRead(PIN_BUTTON_UP);
		downVal	=digitalRead(PIN_BUTTON_DOWN);
		if(!upVal&&downVal){
			if(indexAnalysisMode>=2){
				indexAnalysisMode=0;
			}
			else{
				indexAnalysisMode++;
			}
			while(downVal){
				downVal=digitalRead(PIN_BUTTON_DOWN);
			}
			refreshScreen=true;
		}
		else if(upVal&&!downVal){
			if(indexAnalysisMode<=0){
				indexAnalysisMode=2;
			}
			else{
				indexAnalysisMode--;
			}
			while(upVal){
				upVal=digitalRead(PIN_BUTTON_UP);
			}
			refreshScreen=true;
		}
		if(refreshScreen){
			lcd.setCursor(0, 1); lcd.print("                ");
			lcd.setCursor(0, 1); lcd.print(String(indexAnalysisMode+1) + "-" + analysisModeLcdString[indexAnalysisMode]);
			refreshScreen=false;
		}
	}
	okVal=0;
	delay(200);
	#ifdef DEBUG
		Serial.print(">> Mode Selected: "); Serial.print(indexAnalysisMode); Serial.print(" - "); Serial.println(analysisModeLcdString[indexAnalysisMode]);
		Serial.println(">> Waiting confirm...");
	#endif
	switch(indexAnalysisMode){
		case(ANALYSISMODE_SIMPLEREAD):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("Simple Read");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while(!okVal && !backVal){
				okVal=digitalRead(PIN_BUTTON_OK);
				backVal=digitalRead(PIN_BUTTON_BACK);
				if(okVal){
					#ifdef DEBUG
						Serial.println(">> Simple Read Selected!");
					#endif
					okVal=LOW;
					delay(100);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Lambda (nm): ");
					lcd.setCursor(0, 1); lcd.print("Set Val: ");
					#ifdef DEBUG
						Serial.print(">> Lambda requested:\t\t");
					#endif
					keyPadString="";
					while(!okVal){
						okVal=digitalRead(PIN_BUTTON_OK);
						customKey=0;
						customKey=customKeypad.getKey();
						delay(200);
						if(customKey){
							keyPadString+=customKey;
							lcd.setCursor(0, 1); lcd.print("Set Val: " + keyPadString + "nm");
							#ifdef DEBUG
								Serial.print(customKey);
							#endif
						}
					}
					okVal=LOW;
					#ifdef DEBUG
						Serial.println();
					#endif
					lambdaSelected=keyPadString.toInt();
					delay(200);
					lambdaCorrected=constrain(lambdaSelected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Lambda: ");
					lcd.setCursor(0, 1); lcd.print(String(lambdaCorrected) + "nm");
					#ifdef DEBUG
						if(lambdaCorrected<lambdaSelected){
							Serial.print(">> Lambda Corrected to ["); Serial.print(lambdaCorrected); Serial.print("] couse it's lower then the lower limit of "); Serial.print(SPECTRALIMIT_LOW); Serial.println("nm.");
						}
						else if (lambdaCorrected>lambdaSelected){
							Serial.print(">> Lambda Corrected to ["); Serial.print(lambdaCorrected); Serial.print("] couse it's greater then the higher limit of "); Serial.print(SPECTRALIMIT_HIGH); Serial.println("nm.");
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
					keyPadString="";
					while(!okVal){
						okVal=digitalRead(PIN_BUTTON_OK);
						customKey=0;
						customKey = customKeypad.getKey();
						delay(100);
						if(customKey){
							keyPadString+=customKey;
							lcd.setCursor(0, 1); lcd.print("n: " + keyPadString);
							#ifdef DEBUG
								Serial.print(customKey);
							#endif
						}
					}
					okVal=LOW;
					#ifdef DEBUG
						Serial.println("nm.");
					#endif
					nReplicates=keyPadString.toInt();
					if(nReplicates<MIN_REPLICATES){
						nReplicates=MIN_REPLICATES;
						#ifdef DEBUG
							Serial.print(">> Replicates Corrected to ["); Serial.print(nReplicates); Serial.print("] couse it's lower then the lower limit of "); Serial.print(MIN_REPLICATES); Serial.println(".");
						#endif
					}
					else{
						#ifdef DEBUG
							Serial.println(">> Replicates Acceptded!!");
						#endif
					}
					#ifdef DEBUG
						Serial.println(">> Positioning motor...");
					#endif
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Replicates: " + String(nReplicates));
					lcd.setCursor(0, 1); lcd.print("Pos.ing motor...");
					gratingMotorFutureSteps=map(lambdaCorrected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH, MOTOR_GRATINGLIMIT_HIGH, MOTOR_GRATINGLIMIT_LOW);
					gratingMotorChecking(PIN_MOTOR_POSITIONSENSOR, PIN_PIEZO);			//serve davvero?
					myMotor->step(gratingMotorFutureSteps, FORWARD, SINGLE); 
					#ifdef DEBUG
						Serial.println(">> Positioning motor done! Waiting confirm to proceed...");
					#endif
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Press 'OK' to");
					lcd.setCursor(0, 1); lcd.print("AutoZero");
					//rivedere
					while(!okVal){
						okVal=digitalRead(PIN_BUTTON_OK);
					}
					okVal=LOW;
					#ifdef DEBUG
						Serial.println(">> Reading blank...");
					#endif
					nReads=0;
					sumBackgroundReadsVal=0;
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Reading...");
					lcd.setCursor(0, 1); lcd.print("Index: ");
					for(nReads; nReads<nReplicates; nReads++){
						lcd.setCursor(7, 1); lcd.print(String(nReads) + "/" + String(nReplicates));
						simpleRead();
						sumBackgroundReadsVal+=readsVal;
					}
					nReads=0;
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Load Sample!");
					lcd.setCursor(0, 1); lcd.print("'OK' to read...");
					#ifdef DEBUG
						Serial.println(">> Blank reads done! Waiting confirm to proceed to Sample...");
					#endif
					delay(200);
					while(!okVal && !backVal){
						okVal=digitalRead(PIN_BUTTON_OK);
						backVal=digitalRead(PIN_BUTTON_BACK);
						if(okVal){
							okVal=LOW;
							#ifdef DEBUG
								Serial.println(">> Reading Sample...");
							#endif
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Reading...");
							lcd.setCursor(0, 1); lcd.print("Index: ");
							nReads=0;
							sumSampleReadsVal=0;
							for(nReads; nReads<nReplicates; nReads++){
								lcd.setCursor(7, 1); lcd.print(String(nReads) + "/" + String(nReplicates));
								simpleRead();
								sumSampleReadsVal+=readsVal;
							}
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("--LOADING DATA-");
							lcd.setCursor(0, 1); lcd.print("...PLEAS WAIT...");
							#ifdef DEBUG
								Serial.println(">> Sample reads done! Calculating results...");
							#endif
							float iSource=sumBackgroundReadsVal/(float)nReplicates;
							float iSample=((sumSampleReadsVal/(float)nReplicates));
							float trasmittance=iSample/iSource;
							float absorbance=log10(1/((((sumSampleReadsVal/(float)nReplicates)/(sumBackgroundReadsVal/(float)nReplicates))))); //verificare il 1000
							//float Abs=log10(1/(iSample/iSource))*1000;
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Abs Sample:");
							lcd.setCursor(0, 1); lcd.print(String(absorbance,DECIMAL_LCD_ABSORBANCE));
							#ifdef DEBUG
								Serial.println(">> [---------------------------------------------------]");
								Serial.print(">> [ Number of reads:\t\t");	Serial.println(nReplicates);
								Serial.print(">> [ Background Average:\t");	Serial.println((sumBackgroundReadsVal)/(nReplicates));
								Serial.print(">> [ Sample Average:\t\t");	Serial.println((sumSampleReadsVal)/(nReplicates));
								//Serial.print(">> [ iSource:\t\t\t");		Serial.println(iSource);
								//Serial.print(">> [ iSample:\t\t\t");		Serial.println(iSample);
								Serial.print(">> [ Trasmittance:\t\t");		Serial.println(trasmittance,DECIMAL_SERIAL_TRASMITTANCE);
								Serial.print(">> [ Absorbance:\t\t");		Serial.println(absorbance,DECIMAL_SERIAL_ABSORBANCE);
								//Serial.print(">> [ Abs:\t\t\t");			Serial.print(Abs); 										Serial.println("*10^-3.");
								Serial.println(">> [---------------------------------------------------]");
							#endif
						}
						//okVal=LOW;
					}
					backVal=LOW;
				}
			}
			backVal=LOW;
			refreshScreen=true;
			break;
		}
		case(ANALYSISMODE_ALLSPECTRUM):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("All Spectrum");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while(!okVal && !backVal){
				okVal=digitalRead(PIN_BUTTON_OK);
				backVal=digitalRead(PIN_BUTTON_BACK);
				if(okVal){
					okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Spectrum range");
					lcd.setCursor(0, 1); lcd.print("Set MIN:  ");
					keyPadString="";
					while(okVal==LOW){
						customKey=0;
						customKey = customKeypad.getKey();
						delay(100);
						if(customKey){
							keyPadString=keyPadString+customKey;
							lcd.setCursor(0, 1); lcd.print("Set MIN:  " + keyPadString);
						}
						okVal=digitalRead(PIN_BUTTON_OK);
					}
					lambdaSelected=keyPadString.toInt();
					lambdaMin=constrain(lambdaSelected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Spectrum range");
					lcd.setCursor(0, 1); lcd.print("Set MAX:  ");
					keyPadString="";
					while(okVal==LOW){
						customKey=0;
						customKey = customKeypad.getKey();
						delay(100);
						if(customKey){
							keyPadString=keyPadString+customKey;
							lcd.setCursor(0, 1); lcd.print("Set MAX:  " + keyPadString);
						}
						okVal=digitalRead(PIN_BUTTON_OK);
					}
					lambdaSelected=keyPadString.toInt();
					lambdaMax=constrain(lambdaSelected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
					okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("MIN:  "); lcd.print(lambdaMin);
					lcd.setCursor(0, 1); lcd.print("MAX:  "); lcd.print(lambdaMax);
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Grating Motor");
					lcd.setCursor(0, 1); lcd.print("Zero setting...");
					gratingMotorZeroPoint(PIN_MOTOR_POSITIONSENSOR, PIN_PIEZO);
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Set Replicates");
					lcd.setCursor(0, 1); lcd.print("n:  ");
					keyPadString="";
					while(okVal==LOW){
						customKey=0;
						customKey = customKeypad.getKey();
						delay(100);
						if(customKey){
							keyPadString=keyPadString+customKey;
							lcd.setCursor(0, 1); lcd.print("n:  " + keyPadString);
						}
						okVal=digitalRead(PIN_BUTTON_OK);
					}
					nReplicates=keyPadString.toInt();
					okVal=LOW;
					delay(1000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Replicates:  ");
					lcd.setCursor(0, 1); lcd.print(nReplicates);
					delay(3000);
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Press 'OK' to");
					lcd.setCursor(0, 1); lcd.print("AutoZero");
					while(okVal==LOW){
						okVal=digitalRead(PIN_BUTTON_OK);
					}
					okVal=LOW;
					backgroundSensor();
					lcd.clear();
					lcd.setCursor(0, 0); lcd.print("Load Sample!");
					lcd.setCursor(0, 1); lcd.print("'OK' to read...");
					x=0;
					delay(1000);
					while(backVal==LOW){
						okVal=digitalRead(PIN_BUTTON_OK);
						backVal=digitalRead(PIN_BUTTON_BACK);
						if(okVal==HIGH){
							okVal=LOW;
							x++;
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Reading...");
							delay(1000);
							//	>> ======================= TODO ======================= << 
							lcd.clear();
							lcd.setCursor(0, 0); lcd.print("Spectrum " + x);
							lcd.setCursor(0, 1); lcd.print("Saved!");
						}
					}
					backVal=LOW;
				}
			}
			backVal=LOW;
			refreshScreen=true;
			break;				
		}
		case(ANALYSISMODE_CONCANALYSIS):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("Conc. Analysis");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while((okVal==LOW) && (backVal==LOW)){
				okVal=digitalRead(PIN_BUTTON_OK);
				backVal=digitalRead(PIN_BUTTON_BACK);
				if(okVal){
					okVal=LOW;
					//	>> ======================= TODO ======================= << 
				}
			}
			backVal=LOW;
			refreshScreen=true;
			break;
		}
		
	}
	#ifdef DEBUG
		Serial.println(">> Returning to the selection mode menu");
	#endif
}

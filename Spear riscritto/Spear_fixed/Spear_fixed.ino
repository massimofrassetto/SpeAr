#include <LiquidCrystal.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include <Adafruit_MotorShield.h>

#include <SPI.h>
#include <SD.h>

#include <Keypad.h>

#include "CONSTANTS.h"

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
//byte colPins[COLS] = {PIN_KEYPAD_COLS_0, PIN_KEYPAD_COLS_1, PIN_KEYPAD_COLS_2,};
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char customKey;
String keyPadString;

int gratingMotorSteps;
int lambdaMin;
int lambdaMax;
int lambdaSelected;
int lambdaCorrected;
float lambdaCaptured;
int nReplicates;
int nReads;
int readsVal=0;
int sumReadsVal=0;
long sumBackgroundReadsVal=0;
long sumSampleReadsVal=0;

int backVal=LOW;
int nextVal=LOW;
int upVal=LOW;
int downVal=LOW;
int okVal=LOW;
int lampSwitchVal;
int backgroundSensorVal;
int readSensorVal;
int x=0;	//definire funzione...

#include "FUNCTIONS.h"

void setup(void){
	Serial.begin(SERIAL_BAUDRATE);
	AFMS.begin();
	myMotor->setSpeed(MOTOR_SPEED_RPM);
	pinMode(PIN_BUTTON_OK, 				INPUT);
	pinMode(PIN_BUTTON_BACK, 			INPUT);
	pinMode(PIN_BUTTON_NEXT, 			INPUT);
	pinMode(PIN_BUTTON_UP, 				INPUT);
	pinMode(PIN_BUTTON_DOWN, 			INPUT);
	pinMode(PIN_LAMP_CHECKINGSENSOR, 	INPUT);
	pinMode(PIN_LAMP_SWITCH, 			OUTPUT);
	digitalWrite(PIN_LAMP_SWITCH, 		HIGH);
	lcd.begin(LCD_COLS, LCD_ROWS);
	//delay(1000);
	//Il primo setCursor in 0,0 è da verificare se funziona, attualmente sto scrivendo senza testare 2020/01/31
	lcd.setCursor(0, 0); lcd.print("Spe.Ar. Project");
	lcd.setCursor(0, 1); lcd.print("V "); lcd.print(MODEL_VERSION); lcd.print(" SW "); lcd.print(FIRMWARE_VERSION);
	//lcd.setCursor(0, 1); lcd.print("V 1.0 SW 10");
	//lcd.setCursor(0, 1); lcd.print("V " + MODEL_VERSION + "SW" + FIRMWARE_VERSION);
	tone(PIN_PIEZO, 500, 100); delay(100);
	tone(PIN_PIEZO, 300, 200); delay(100);
	tone(PIN_PIEZO, 500, 400); delay(4300);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Starting");
	lcd.setCursor(0, 1); lcd.print("Instrument...");
	//delay(2500);
	
	lampChecking(PIN_LAMP_CHECKINGSENSOR, PIN_LAMP_SWITCH, &lampSwitchVal, PIN_PIEZO);
	tslSensorChecking();
	gratingMotorChecking(PIN_MOTOR_POSITIONSENSOR, PIN_PIEZO);
	SDCardChecking(CHIPSELECT);
	
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
	while(okVal==LOW){
		okVal=digitalRead(PIN_BUTTON_OK);
	}
	okVal=LOW;
}

void loop(void){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Analysis:");
	lcd.setCursor(0, 1); lcd.print(" 1  2  3");
	/*MODES:
		1	-	SIMPLE READ
		2	-	ALL SPECTRUM
		3	-	CONCANALYSIS
	*/
	customKey=0;
	while(customKey==0){ 
		customKey = customKeypad.getKey();
		delay(100);
	}
	switch(customKey){
		case(ANALYSISMODE_SIMPLEREAD):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("Simple Read");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while((okVal==LOW) && (backVal==LOW)){
				okVal=digitalRead(PIN_BUTTON_OK);
				backVal=digitalRead(PIN_BUTTON_BACK);
			}
			if(okVal==HIGH){
				okVal=LOW;
				delay(100);
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Lambda (nm): ");
				lcd.setCursor(0, 1); lcd.print("Set Val: ");
				keyPadString="";
				while(okVal==LOW){
					customKey=0;
					customKey = customKeypad.getKey();
					delay(100);
					if(customKey){
						keyPadString=keyPadString+customKey;
						lcd.setCursor(0, 1); lcd.print("Set Val:	" + keyPadString);
					}
					okVal=digitalRead(PIN_BUTTON_OK);
				}
				lambdaSelected=keyPadString.toInt();
				okVal=LOW;
				delay(1000);
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Lambda: ");
				lcd.setCursor(0, 1);
				lambdaCorrected=constrain(lambdaSelected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
				delay(200);
				gratingMotorSteps=map(lambdaCorrected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH, MOTOR_GRATINGLIMIT_HIGH, MOTOR_GRATINGLIMIT_LOW);
				lcd.print(lambdaCorrected); lcd.print("nm");
				delay(1000);
				gratingMotorChecking(PIN_MOTOR_POSITIONSENSOR, PIN_PIEZO);
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Set Replicates");
				lcd.setCursor(0, 1); lcd.print("n: ");
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
				okVal=LOW;
				nReplicates=keyPadString.toInt();
				if(nReplicates<MIN_REPLICATES){
					nReplicates=MIN_REPLICATES;
				}
				//delay(1000);
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Replicates:  ");
				lcd.setCursor(0, 1); lcd.print(nReplicates);
				delay(3000);
				myMotor->step(gratingMotorSteps, FORWARD, SINGLE); 
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Press 'OK' to");
				lcd.setCursor(0, 1); lcd.print("AutoZero");
				while(okVal==LOW){
					okVal=digitalRead(PIN_BUTTON_OK);
				}
				okVal=LOW;
				delay(1000);
				nReads=0;
				sumBackgroundReadsVal=0;
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Reading...");
				delay(1000);
				//for(nReads; nReads<((nReplicates) + MIN_REPLICATES); nReads++){
				for(nReads; nReads<nReplicates; nReads++){
					simpleRead();
					sumBackgroundReadsVal+=readsVal;
				}
				nReads=0;
				lcd.clear();
				lcd.setCursor(0, 0); lcd.print("Load Sample!");
				lcd.setCursor(0, 1); lcd.print("'OK' to read...");
				delay(1000);
				while(backVal==LOW){	
					okVal=digitalRead(PIN_BUTTON_OK);
					backVal=digitalRead(PIN_BUTTON_BACK);
					if(okVal==HIGH){
						okVal=LOW;
						lcd.clear();
						lcd.setCursor(0, 0); lcd.print("Reading...");
						//delay(1000);
						nReads=0;
						sumSampleReadsVal=0;
						for(nReads; nReads<nReplicates; nReads++){
							simpleRead();
							sumSampleReadsVal+=readsVal;
						}
						lcd.clear();
						lcd.setCursor(0, 0); lcd.print("Abs Sample:");
						Serial.print((sumSampleReadsVal)/(nReplicates));
						Serial.print("///");
						//Serial.println((sumBackgroundReadsVal)/((nReplicates)+MIN_REPLICATES));
						Serial.println((sumBackgroundReadsVal)/(nReplicates));
						Serial.print(nReplicates);
						Serial.print("///");
						//Serial.println(((nReplicates)+MIN_REPLICATES));
						Serial.println(nReplicates);
						//long iSource=((sumBackgroundReadsVal)/((nReplicates)+MIN_REPLICATES));
						long iSource=((sumBackgroundReadsVal)/(nReplicates));
						long iSample=((sumSampleReadsVal)/(nReplicates));
						long trasmittance=(1000*((iSample)/(iSource)));
						long absorbance=log10(1/((iSample)/(iSource)))*1000;
						Serial.print(iSource); Serial.print("-"); Serial.print(iSample); Serial.print("-"); Serial.print(trasmittance); Serial.print("-"); Serial.println(absorbance);
						//Abs=(log10(1/((sumSampleReadsVal)/(nReplicates)/(sumBackgroundReadsVal)/((nReplicates)+MIN_REPLICATES))))*1000;
						long Abs=(log10(1/((sumSampleReadsVal)/(nReplicates)/(sumBackgroundReadsVal)/(nReplicates))))*1000;
						Serial.println(Abs);
						lcd.setCursor(0, 1); lcd.print(Abs);
					}
					okVal=LOW;
				}
				backVal=LOW;
			} 
			break;
		}
		case(ANALYSISMODE_ALLSPECTRUM):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("All Spectrum");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while((okVal==LOW) && (backVal==LOW)){
				okVal=digitalRead(PIN_BUTTON_OK);
				backVal=digitalRead(PIN_BUTTON_BACK);
			}
			if(okVal==HIGH){
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
				break;				
			}
		}
		case(ANALYSISMODE_CONCANALYSIS):{
			lcd.clear();
			lcd.setCursor(0, 0); lcd.print("Conc. Analysis");
			lcd.setCursor(0, 1); lcd.print("selected?");
			while((okVal==LOW) && (backVal==LOW)){
				okVal=digitalRead(PIN_BUTTON_OK);
				backVal=digitalRead(PIN_BUTTON_BACK);
			}
			if(okVal==HIGH){
				okVal=LOW;
				//	>> ======================= TODO ======================= << 
			}
			okVal=LOW;
			break;
		}
		
	}
}

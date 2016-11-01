#include <LiquidCrystal.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include <Adafruit_MotorShield.h>

#include <SPI.h>
#include <SD.h>

#include <Keypad.h>

LiquidCrystal lcd(8,9,10,11,12,13);

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

const int piezoPin=46;
const int lampCheckingSensorPin=A9;
int lampCheckingSensorVal=0;
const int lampSwitchPin=25;
int lampSwitchVal;
const int positionMotorSensorPin=A8;
int positionMotorSensorVal=0;

const int chipSelect=53;

File dataFile;

const byte ROWS = 4;
const byte COLS = 3;
char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {38,37, 36, 35}; 
byte colPins[COLS] = {34, 33, 32,};
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char customKey;
String keyPadString;
int lowSpectraLimit=380;
int highSpectraLimit=780;
int lowGratingMotorLimit=850;
int highGratingMotorLimit=1250;
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
long Abs;

const int backPin=3;
const int nextPin=4;
const int upPin=5;
const int downPin=6;
const int okPin=7;
int backVal=LOW;
int nextVal=LOW;
int upVal=LOW;
int downVal=LOW;
int okVal=LOW;
int backgroundSensorVal;
int readSensorVal;
int x=0;

void configureSensor(void){
  //tsl.setGain(TSL2591_GAIN_LOW);      // 1x gain (bright light)
  //tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);     // 428x gain
  tsl.setGain(TSL2591_GAIN_MAX);        // 9876x (extremely low light)
 
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
 
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print  ("Gain:");
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain){
    case TSL2591_GAIN_LOW:
      lcd.print("1x(Low)");
      break;
    case TSL2591_GAIN_MED:
      lcd.print("25x(Medium)");
      break;
    case TSL2591_GAIN_HIGH:
      lcd.print("428x(High)");
      break;
    case TSL2591_GAIN_MAX:
      lcd.print("9876x (Max)");
      break;
  }
  lcd.setCursor(0, 1);
  lcd.print ("Timing:");
  lcd.print((tsl.getTiming() + 1) * 100, DEC); 
  lcd.print("ms");
  delay(3000);
}

void tslSensorChecking (void){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("TSL2591");
  lcd.setCursor(0, 1);
  lcd.print("Checking..."); 
  delay(2500);
  if (tsl.begin()){
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("TSL2591 Sensor");
    lcd.setCursor(0, 1);
    lcd.print("OK");
  } 
  else{
    printWiringError();
  }
  delay(2500);
  configureSensor();
  delay(3000);
}

void gratingMotorZeroPoint (void){
  positionMotorSensorVal=analogRead(positionMotorSensorPin);
  while(positionMotorSensorVal<=470){
    positionMotorSensorVal=analogRead(positionMotorSensorPin);
    myMotor->step(1, BACKWARD, SINGLE); 
  }
  myMotor->step(1050, FORWARD, SINGLE); 
  tone(46,400,500);
}


void gratingMotorChecking (void){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Grating Motor");
  lcd.setCursor(0, 1);
  lcd.print("Positioning...");
  gratingMotorZeroPoint();
  lcd.setCursor(0, 1);
  lcd.print("Motor Dgr.= 0'");
  delay(3000);
}

void SDCardChecking (void){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Initializing");
  lcd.setCursor(0, 1);
  lcd.print("SD card...");
  delay(2000);
  if (!SD.begin(53)) {
    printWiringError();
    delay(2500);
    return;
  }
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("Initialization");
    lcd.setCursor(0, 1);
    lcd.print("DONE!!!");
    delay(3000);
}

void printWiringError(){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("OH-oh..");
  tone(piezoPin, 400, 1000);
  delay(500);
  tone(piezoPin, 400, 1000);
  lcd.setCursor(0, 1);
  lcd.print("check wiring?");
}

void lampChecking (void){
  //int lampOkArray[3]={1,0,1};
  int lampChecingkArray[3]={0,0,0};
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Lamp");
  lcd.setCursor(0, 1);
  lcd.print("Checking");
  delay(2500);
  lampCheckingSensorVal=analogRead(lampCheckingSensorPin);
  lcd.print(".");
  if(lampCheckingSensorVal>500){
    lampChecingkArray[0]=1;
  }
  lampSwitchVal=LOW;
  digitalWrite(lampSwitchPin, lampSwitchVal);
  delay(1000);
  lampCheckingSensorVal=analogRead(lampCheckingSensorPin);
  lcd.print(".");
  if(lampCheckingSensorVal<500){
    lampChecingkArray[1]=0;
  }
  lampSwitchVal=HIGH;
  digitalWrite(lampSwitchPin, lampSwitchVal);
  delay(1000);
  lampCheckingSensorVal=analogRead(lampCheckingSensorPin);
  lcd.print(".");
  if(lampCheckingSensorVal>500){
    lampChecingkArray[2]=1;
  }
  delay(1000);
  /*controllare correttezza sintassi
   */
  if(lampChecingkArray[0]&&(!lampChecingkArray[1])&&lampChecingkArray[2]){
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("Lamp ");
    lcd.setCursor(0, 1);
    lcd.print("is OK!!");
  }
  else{
    printWiringError();
  }
  delay(2500);
}

void setup(void){
  
}

void loop(void){
  
}


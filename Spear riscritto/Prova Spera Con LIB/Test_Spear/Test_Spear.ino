#include <SPEAR.h>
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
//int lampCheckingSensorVal=0;
const int lampSwitchPin=25;
int lampSwitchVal;
const int positionMotorSensorPin=A8;
//int positionMotorSensorVal=0;

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

void setup(void){
  Serial.begin(9600);
  AFMS.begin();
  myMotor->setSpeed(50);  //rpm
  pinMode(okPin, INPUT);
  pinMode(backPin, INPUT);
  pinMode(nextPin, INPUT);
  pinMode(upPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(lampCheckingSensorPin, INPUT);
  pinMode(lampSwitchPin, OUTPUT);
  digitalWrite(lampSwitchPin, HIGH);
  lcd.begin(16, 2);
  delay(1000);
  lcd.print("Spe.Ar. Project");
  lcd.setCursor(0, 1);
  lcd.print("V 1.0  SW 10");
  tone(piezoPin, 500, 100);
  delay(100);
  tone(piezoPin, 300, 200);
  delay(100);
  tone(piezoPin, 500, 400);
  delay(4300);
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Starting");
  lcd.setCursor(0, 1);
  lcd.print("Instrument...");
  delay(2500);
  
  SPEAR.lampChecking(lampCheckingSensorPin, lampSwitchPin, &lampSwitchVal, piezoPin);
  SPEAR.tslSensorChecking();
  SPEAR.gratingMotorChecking(positionMotorSensorPin, piezoPin);
  SPEAR.SDCardChecking(chipSelect);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Instrument");
  lcd.setCursor(0, 1);
  lcd.print("is Ready!!!!");
  tone(piezoPin, 600, 200);
  delay(200);
  tone(piezoPin, 600, 200);
  delay(100);
  tone(piezoPin, 900, 400);
  delay(1000);
  lcd.noDisplay();
  delay(500);
  lcd.display();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press 'OK'");
  lcd.setCursor(0, 1);
  lcd.print("to Start...");
  while(okVal==LOW){
    okVal=digitalRead(okPin);
  }
  okVal=LOW;
}

void loop(void){
  
}


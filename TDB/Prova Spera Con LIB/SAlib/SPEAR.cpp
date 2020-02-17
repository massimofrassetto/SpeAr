#include "Arduino.h"
#include "SPEAR.h"

void SPEAR::configureSensor(void){
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

void SPEAR::tslSensorChecking (void){
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
    printWiringError(piezoPin);
  }
  delay(2500);
  configureSensor();
  delay(3000);
}

void SPEAR::gratingMotorZeroPoint (const int sensMotPin, const int piezoZeroPin){
  int positionMotorSensorVal=analogRead(sensMotPin);
  while(positionMotorSensorVal<=470){
    positionMotorSensorVal=analogRead(sensMotPin);
    myMotor->step(1, BACKWARD, SINGLE); 
  }
  myMotor->step(1050, FORWARD, SINGLE); 
  tone(piezoZeroPin, 400, 500);
}

void SPEAR::gratingMotorChecking (const int sensMotCheckPin, const int piezoCheckPin){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Grating Motor");
  lcd.setCursor(0, 1);
  lcd.print("Positioning...");
  gratingMotorZeroPoint(sensMotCheckPin, piezoCheckPin);
  lcd.setCursor(0, 1);
  lcd.print("Motor Dgr.= 0'");
  delay(3000);
}

void SPEAR::SDCardChecking (const int chipSel){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Initializing");
  lcd.setCursor(0, 1);
  lcd.print("SD card...");
  delay(2000);
  if (!SD.begin(chipSel)) {
    printWiringError(piezoPin);
    delay(2500);
    return;
  }
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("Initialization");
    lcd.setCursor(0, 1);
    lcd.print("DONE!!!");
    delay(3000);
}

void SPEAR::printWiringError (const int allarmPin){
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("OH-oh..");
  tone(allarmPin, 400, 1000);
  delay(500);
  tone(allarmPin, 400, 1000);
  lcd.setCursor(0, 1);
  lcd.print("check wiring?");
}

void SPEAR::lampChecking (const int lampSensPin, const int lampStatePin, int* lampState, const int piezoLCPin){
  int lampCheckingSensorVal=0;
  int lampChecingkArray[3]={0,0,0};
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Lamp");
  lcd.setCursor(0, 1);
  lcd.print("Checking");
  delay(2500);
  lampCheckingSensorVal=analogRead(lampSensPin);
  lcd.print(".");
  if(lampCheckingSensorVal>500){
    lampChecingkArray[0]=1;
  }
  (*lampState)=LOW;
  digitalWrite(lampStatePin, (*lampState));
  delay(1000);
  lampCheckingSensorVal=analogRead(lampSensPin);
  lcd.print(".");
  if(lampCheckingSensorVal<500){
    lampChecingkArray[1]=0;
  }
  (*lampState)=HIGH;
  digitalWrite(lampStatePin, (*lampState));
  delay(1000);
  lampCheckingSensorVal=analogRead(lampSensPin);
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
    printWiringError(piezoLCPin);
  }
  delay(2500);
}


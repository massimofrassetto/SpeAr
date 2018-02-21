/*IMPORTAZIONE LIBRERIE*/
//lcd
#include <LiquidCrystal.h> 
#include <Wire.h> //this library allows you to communicate with I2C / TWI devices
//sensore
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
//driver motore (nel caso si farà una seconda versione prevedo di inserire un driver stile 3d printer)
#include <Adafruit_MotorShield.h>
//sd control
#include <SPI.h>
#include <SD.h>
//paddino numerico
#include <Keypad.h>

LiquidCrystal lcd(8,9,10,11,12,13);

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

const int piezoPin=46;  //pin del cicalino
const int lampCheckingSensorPin=A9; //pin della resistenza per vedere se la lampada si accende
int lampCheckingSensorVal=0; //valore di verifica (credo si possa eliminare usando una semplice variabile di uscita da funzione)
const int lampSwitchPin=25; //pin per spegnere e accendere la lampada
int lampSwitchVal; //val per memorizzare se accesa o spenta
const int positionMotorSensorPin=A8; //pin del sensore per il posizionamento del reticolo
int positionMotorSensorVal=0;   //valore di rotazione del reticolo

const int chipSelect=53; //per controllare l'sd

File dataFile;
//variabili per il controllo del paddino
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
//fine variabili paddino
int lowSpectraLimit=380; //frequenza minore
int highSpectraLimit=780; //frequenza maggiore
int lowGratingMotorLimit=850; //passi che corrispondono alla low
int highGratingMotorLimit=1250; //passi che corrispondo alla high
int gratingMotorSteps; //passi che deve compiere lo stepper
//2 frequenze estreme nel caso si voglia analizzare un range
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

void configureSensor(void)
{
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
  switch(gain)
  {
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
  if (tsl.begin()) 
  {
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("TSL2591 Sensor");
    lcd.setCursor(0, 1);
    lcd.print("OK");
  } 
  else 
  {
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("OH-oh..");
    tone(piezoPin, 400, 1000);
    delay(500);
    tone(piezoPin, 400, 1000);
    lcd.setCursor(0, 1);
    lcd.print("check wiring?");
  }
  delay(2500);
  configureSensor();
  delay(3000);
}

void gratingMotorZeroPoint (void)
{
  positionMotorSensorVal=analogRead(positionMotorSensorPin);
  while(positionMotorSensorVal<=470)
  {
    positionMotorSensorVal=analogRead(positionMotorSensorPin);
    myMotor->step(1, BACKWARD, SINGLE); 
  }
  myMotor->step(1050, FORWARD, SINGLE); 
  tone(46,400,500);
}


void gratingMotorChecking (void)
{
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Grating Motor");
  lcd.setCursor(0, 1);
  lcd.print("Positioning...");
  gratingMotorZeroPoint();
  lcd.setCursor(0, 1);
  lcd.print("Motor Dgr.= 0'");
  delay(3000);
}

void SDCardChecking (void)
{
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Initializing");
  lcd.setCursor(0, 1);
  lcd.print("SD card...");
  delay(2000);
  //-----------------------------------------------
  if (!SD.begin(53)) {
    lcd.print("OH-oh..");
    tone(piezoPin, 400, 1000);
    delay(500);
    tone(piezoPin, 400, 1000);
    lcd.setCursor(0, 1);
    lcd.print("check wiring?");
    delay(2500);
    return;
  }
  
  
  //-----------------------------------------------
    /*if (!SD.begin(53))
    {
      lcd.clear(); lcd.setCursor(0, 0);
      lcd.print("Initialization");
      lcd.setCursor(0, 1);
      lcd.print("FAILED!!!");
      delay(3000);
      return;
    }*/
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("Initialization");
    lcd.setCursor(0, 1);
    lcd.print("DONE!!!");
    delay(3000);
}

void lampChecking (void)
{
   lcd.clear(); lcd.setCursor(0, 0);
   lcd.print("Lamp");
   lcd.setCursor(0, 1);
   lcd.print("Checking... ");
   delay(2500);
   int i=0;
   lampCheckingSensorVal=analogRead(lampCheckingSensorPin);
   if(lampCheckingSensorVal>500)
   {
     i++;
   }
   lampSwitchVal=LOW;
   digitalWrite(lampSwitchPin, lampSwitchVal);
   delay(1000);
   lampCheckingSensorVal=analogRead(lampCheckingSensorPin);
   if(lampCheckingSensorVal<500)
   {
     i++;
   }
   lampSwitchVal=HIGH;
   digitalWrite(lampSwitchPin, lampSwitchVal);
   delay(1000);
   lampCheckingSensorVal=analogRead(lampCheckingSensorPin);
   if(lampCheckingSensorVal>500)
   {
     i++;
   }
   delay(1000);
   lcd.clear(); lcd.setCursor(0, 0);
   if(i==3)
   {
     lcd.print("Lamp ");
     lcd.setCursor(0, 1);
     lcd.print("is OK!!");
   }
   else
   {
     lcd.print("OH-oh..");
     tone(piezoPin, 400, 1000);
     delay(500);
     tone(piezoPin, 400, 1000);
     lcd.setCursor(0, 1);
     lcd.print("check wiring?");
   }
   delay(2500);
}

void setup(void) 
{
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
  lcd.print("V 1.0");
  tone(46, 500, 100);
  delay(100);
  tone(46, 300, 200);
  delay(100);
  tone(46, 500, 400);
  delay(4300);
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Starting");
  lcd.setCursor(0, 1);
  lcd.print("Instrument...");
  delay(2500);
  
 // lampChecking();
  tslSensorChecking();
  //gratingMotorChecking();
  SDCardChecking();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Instrument");
  lcd.setCursor(0, 1);
  lcd.print("is Ready!!!!");
  tone(46, 600, 200);
  delay(200);
  tone(46, 600, 200);
  delay(100);
  tone(46, 900, 400);
  delay(1000);
  lcd.noDisplay();
  delay(500);
  lcd.display();
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press 'OK'");
  lcd.setCursor(0, 1);
  lcd.print("to continue...");
  while(okVal==LOW)
  {
    okVal=digitalRead(okPin);
  }
  okVal=LOW;
}

int simpleRead(void)
{
  // Simple data read example. Just read the infrared, fullspecrtrum diode 
  // or 'visible' (difference between the two) channels.
  // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
  uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
  //uint16_t x = tsl.getLuminosity(TSL2561_FULLSPECTRUM);
  //uint16_t x = tsl.getLuminosity(TSL2561_INFRARED);

  Serial.println(x, DEC);
  readsVal=x;
}

/*void advancedRead(void)
{
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  Serial.print("[ "); Serial.print(millis()); Serial.print(" ms ] ");
  Serial.print("IR: "); Serial.print(ir);  Serial.print("  ");
  Serial.print("Full: "); Serial.print(full); Serial.print("  ");
  Serial.print("Visible: "); Serial.print(full - ir); Serial.print("  ");
  Serial.print("Lux: "); Serial.println(tsl.calculateLux(full, ir));
}

/**************************************************************************/
/*
    Performs a read using the Adafruit Unified Sensor API.
*/
/**************************************************************************/
/*void unifiedSensorAPIRead(void)
{
  //sensors_event_t event;
  tsl.getEvent(&event);
 
  
  Serial.print("[ "); Serial.print(event.timestamp); Serial.print(" ms ] ");
  if ((event.light == 0) |
      (event.light > 4294966000.0) | 
      (event.light <-4294966000.0))
  {
    Serial.println("Invalid data (adjust gain or timing)");
  }
  else
  {
    Serial.print(event.light); Serial.println(" lux");
  }
}*/

void backgroundSensor (void)
{
  delay(1000);
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Opening...");
  dataFile = SD.open("allspect.txt", FILE_WRITE);
  
  if (dataFile)
  {
    lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("writing...");
    delay(2500);
    dataFile.println("All Specrtum Analysis Data");
    dataFile.print("SerialNumber:  ");
    dataFile.println("cacca banana");
    dataFile.close();
    
         }
         else{
           lcd.clear(); lcd.setCursor(0, 0);
    lcd.print("problem...  ):");
    delay(2500);
           
         }
}

void loop(void) 
{ 
  lcd.clear(); lcd.setCursor(0, 0);
  lcd.print("Analysis:");
  lcd.setCursor(0, 1);
  lcd.print("   1   2   3");
  customKey=0;
  while(customKey==0)
  { 
    customKey = customKeypad.getKey();
    delay(100);
  }
  switch(customKey)
  {
    case('1'):
    {
      lcd.clear(); lcd.setCursor(0, 0);
      lcd.print("Simple Read");
      lcd.setCursor(0, 1);
      lcd.print("selected?");
      while((okVal==LOW) && (backVal==LOW))
      {
        okVal=digitalRead(okPin);
        backVal=digitalRead(backPin);
      }
      if(okVal==HIGH)
      {
        okVal=LOW;
        delay(1000);
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Lambda (nm):");
        lcd.setCursor(0, 1);
        lcd.print("Set Val:  ");
        keyPadString="";
        while(okVal==LOW)
        {
          customKey=0;
          customKey = customKeypad.getKey();
          delay(100);
          if(customKey)
          {
            keyPadString=keyPadString+customKey;
            lcd.setCursor(0, 1);
            lcd.print("Set Val:  "+ keyPadString);
          }
          okVal=digitalRead(okPin);
        }
        lambdaSelected=keyPadString.toInt();
        okVal=LOW;
        delay(1000);
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Lambda:");
        lcd.setCursor(0, 1);
        lambdaCorrected=constrain(lambdaSelected, lowSpectraLimit, highSpectraLimit);
        delay(200);
        gratingMotorSteps=map(lambdaCorrected, lowSpectraLimit, highSpectraLimit, highGratingMotorLimit, lowGratingMotorLimit);
        lcd.print(lambdaCorrected); lcd.print(" nm");
        delay(1000);
        gratingMotorChecking();
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Set Replicates");
        lcd.setCursor(0, 1);
        lcd.print("n:  ");
        keyPadString="";
        while(okVal==LOW)
        {
          customKey=0;
          customKey = customKeypad.getKey();
          delay(100);
          if(customKey)
          {
            keyPadString=keyPadString+customKey;
            lcd.setCursor(0, 1);
            lcd.print("n:  "+ keyPadString);
          }
          okVal=digitalRead(okPin);
        }
        nReplicates=keyPadString.toInt();
        okVal=LOW;
        delay(1000);
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Replicates:  ");
        lcd.setCursor(0, 1);
        lcd.print(nReplicates);
        delay(3000);
        myMotor->step(gratingMotorSteps, FORWARD, SINGLE); 
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Press 'OK' to");
        lcd.setCursor(0, 1);
        lcd.print("AutoZero");
        while(okVal==LOW)
        {
          okVal=digitalRead(okPin);
        }
        okVal=LOW;
        delay(1000);
        nReads=0;
        sumBackgroundReadsVal=0;
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Reading...");
        delay(1000);
        for(nReads; nReads<((nReplicates)+5); nReads++)
        {
          simpleRead();
          sumBackgroundReadsVal+=readsVal;
        }
        nReads=0;
        lcd.clear(); lcd.setCursor(0, 0);
        lcd.print("Load Sample!");
        lcd.setCursor(0, 1);
        lcd.print("'OK' to read...");
        delay(1000);
        while(backVal==LOW)
        {  
           okVal=digitalRead(okPin);
           backVal=digitalRead(backPin);
           if(okVal==HIGH)
           {
             okVal=LOW;
             lcd.clear(); lcd.setCursor(0, 0);
             lcd.print("Reading...");
             delay(1000);
             nReads=0;
             sumSampleReadsVal=0;
             for(nReads; nReads<nReplicates; nReads++)
               {
                 simpleRead();
                 sumSampleReadsVal+=readsVal;
               }
             lcd.clear(); lcd.setCursor(0, 0);
             lcd.print("Abs Sample:");
             lcd.setCursor(0, 1);
             Serial.print((sumSampleReadsVal)/(nReplicates));
             Serial.print("///");
             Serial.println((sumBackgroundReadsVal)/((nReplicates)+5));
             Serial.print(nReplicates);
             Serial.print("///");
             Serial.println(((nReplicates)+5));
             long iSource=((sumBackgroundReadsVal)/((nReplicates)+5));
             long iSample=((sumSampleReadsVal)/(nReplicates));
             long trasmittance=(1000*((iSample)/(iSource)));
             long absorbance=log10(1/((iSample)/(iSource)))*1000;
             Serial.print(iSource);
             Serial.print("-");
             Serial.print(iSample);
             Serial.print("-");
             Serial.print(trasmittance);
             Serial.print("-");
             Serial.println(absorbance);
             Abs=(log10(1/((sumSampleReadsVal)/(nReplicates)/(sumBackgroundReadsVal)/((nReplicates)+5))))*1000;
             Serial.println(Abs);
             lcd.print(Abs);
           }
           okVal=LOW;
          }
         backVal=LOW;
       } 
       break;
    }
    case('2'):
    {
      lcd.clear(); lcd.setCursor(0, 0);
      lcd.print("All Spectrum");
      lcd.setCursor(0, 1);
      lcd.print("selected?");
      while((okVal==LOW) && (backVal==LOW))
      {
        okVal=digitalRead(okPin);
        backVal=digitalRead(backPin);
      }
      if(okVal==HIGH)
      {
         okVal=LOW;
         delay(1000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Spectrum range");
         lcd.setCursor(0, 1);
         lcd.print("Set MIN:  ");
         keyPadString="";
         while(okVal==LOW)
         {
           customKey=0;
           customKey = customKeypad.getKey();
           delay(100);
           if(customKey)
           {
             keyPadString=keyPadString+customKey;
             lcd.setCursor(0, 1);
             lcd.print("Set MIN:  "+ keyPadString);
           }
           okVal=digitalRead(okPin);
         }
         lambdaSelected=keyPadString.toInt();
         lambdaMin=constrain(lambdaSelected, lowSpectraLimit, highSpectraLimit);
         okVal=LOW;
         delay(1000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Spectrum range");
         lcd.setCursor(0, 1);
         lcd.print("Set MAX:  ");
         keyPadString="";
         while(okVal==LOW)
         {
           customKey=0;
           customKey = customKeypad.getKey();
           delay(100);
           if(customKey)
           {
             keyPadString=keyPadString+customKey;
             lcd.setCursor(0, 1);
             lcd.print("Set MAX:  "+ keyPadString);
           }
           okVal=digitalRead(okPin);
         }
         lambdaSelected=keyPadString.toInt();
         lambdaMax=constrain(lambdaSelected, lowSpectraLimit, highSpectraLimit);
         okVal=LOW;
         delay(1000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("MIN:  ");
         lcd.print(lambdaMin);
         lcd.setCursor(0, 1);
         lcd.print("MAX:  ");
         lcd.print(lambdaMax);
         delay(1000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Grating Motor");
         lcd.setCursor(0, 1);
         lcd.print("Zero setting...");
         gratingMotorZeroPoint();
         delay(1000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Set Replicates");
         lcd.setCursor(0, 1);
         lcd.print("n:  ");
         keyPadString="";
         while(okVal==LOW)
         {
           customKey=0;
           customKey = customKeypad.getKey();
           delay(100);
           if(customKey)
           {
             keyPadString=keyPadString+customKey;
             lcd.setCursor(0, 1);
             lcd.print("n:  "+ keyPadString);
           }
           okVal=digitalRead(okPin);
         }
         nReplicates=keyPadString.toInt();
         okVal=LOW;
         delay(1000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Replicates:  ");
         lcd.setCursor(0, 1);
         lcd.print(nReplicates);
         delay(3000);
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Press 'OK' to");
         lcd.setCursor(0, 1);
         lcd.print("AutoZero");
         while(okVal==LOW)
         {
           okVal=digitalRead(okPin);
         }
         okVal=LOW;
         backgroundSensor();
         lcd.clear(); lcd.setCursor(0, 0);
         lcd.print("Load Sample!");
         lcd.setCursor(0, 1);
         lcd.print("'OK' to read...");
         x=0;
         delay(1000);
         while(backVal==LOW)
         {
          okVal=digitalRead(okPin);
          backVal=digitalRead(backPin);
          if(okVal==HIGH)
          {
            okVal=LOW;
            x++;
            lcd.clear(); lcd.setCursor(0, 0);
            lcd.print("Reading...");
            delay(1000);
            //*******************normal
            lcd.clear(); lcd.setCursor(0, 0);
            lcd.print("Spectrum "+ x);
            lcd.setCursor(0, 1);
            lcd.print("Saved!");
          }
        }
        backVal=LOW;
        break;        
      }
    }
    case('3'):
    {
      lcd.clear(); lcd.setCursor(0, 0);
      lcd.print("Conc. Analysis");
      lcd.setCursor(0, 1);
      lcd.print("selected?");
      while((okVal==LOW) && (backVal==LOW))
      {
        okVal=digitalRead(okPin);
        backVal=digitalRead(backPin);
      }
      if(okVal==HIGH)
      {
         okVal=LOW;

        
      }
      okVal=LOW;
      break;
    }
    
  }
}

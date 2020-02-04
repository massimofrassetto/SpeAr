#ifndef FUNCTIONS.h
#define FUNCTIONS.h

// =========================================================================================
// ======================================= PROTOTIPI =======================================
// =========================================================================================

void configureSensor(void);
void printWiringError(const int allarmPin);
void gratingMotorZeroPoint(const int sensMotPin, const int piezoPin);
void gratingMotorChecking(const int sensMotCheckPin, const int piezoCheckPin);
void SDCardChecking(const int chipSel);
void lampChecking(const int lampSensPin, const int lampStatePin, int* lampState, const int piezoLCPin);
int simpleRead(void);
void backgroundSensor(void);

// =========================================================================================
// ======================================= FUNZIONI ========================================
// =========================================================================================

void configureSensor(void){
	//tsl.setGain(TSL2591_GAIN_LOW);			// 1x gain (bright light)
	//tsl.setGain(TSL2591_GAIN_MED);			// 25x gain
	//tsl.setGain(TSL2591_GAIN_HIGH);		 // 428x gain
	tsl.setGain(TSL2591_GAIN_MAX);				// 9876x (extremely low light)
 
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);	// shortest integration time (bright light)
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
	tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);	// longest integration time (dim light)
 
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Gain:");
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
	lcd.setCursor(0, 1); lcd.print("Timing:"); lcd.print((tsl.getTiming() + 1) * 100, DEC); lcd.print("ms");
	delay(500);
}

void printWiringError (const int allarmPin){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("OH-oh..");
	tone(allarmPin, 400, 1000); delay(500);
	tone(allarmPin, 400, 1000);
	lcd.setCursor(0, 1); lcd.print("check wiring?");
}

void tslSensorChecking (void){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("TSL2591");
	lcd.setCursor(0, 1); lcd.print("Checking..."); 
	delay(500);
	if (tsl.begin()){
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("TSL2591 Sensor");
		lcd.setCursor(0, 1); lcd.print("OK");
	} 
	else{
		printWiringError(PIN_PIEZO);
	}
	delay(500);
	configureSensor();
	delay(500);
}

void gratingMotorZeroPoint (const int sensMotPin, const int piezoPin){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Grating Motor");
	lcd.setCursor(0, 1); lcd.print("Homing...");
	int positionMotorSensorVal=analogRead(sensMotPin);
	while(positionMotorSensorVal<=MOTOR_POSITIONSENSOR_THRESHOLD){
		positionMotorSensorVal=analogRead(sensMotPin);
		myMotor->step(1, BACKWARD, SINGLE); 
	}
	myMotor->step(1050, FORWARD, SINGLE); 													//COSTANTE da configurare!!!!!!!!!!!!!!!!
	tone(piezoPin, 400, 500);
}

void gratingMotorChecking (const int sensMotCheckPin, const int piezoCheckPin){
	gratingMotorZeroPoint(sensMotCheckPin, piezoCheckPin);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Motor Setted!");
	lcd.setCursor(0, 1); lcd.print("Dgr.= 0'");
	delay(500);
}

void SDCardChecking (const int chipSel){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Initializing");
	lcd.setCursor(0, 1); lcd.print("SD card...");
	delay(500);
	if (!SD.begin(chipSel)) {
		printWiringError(PIN_PIEZO);
		delay(2500);
		return;
	}
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Initialization");
	lcd.setCursor(0, 1); lcd.print("DONE!!!");
	delay(500);
}

void lampChecking (const int lampSensPin, const int lampStatePin, int* lampState, const int piezoLCPin){
	int lampCheckingSensorVal=0;
	int lampChecingkArray[3]={0,0,0};
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Lamp");
	lcd.setCursor(0, 1); lcd.print("Checking");
	//delay(2500);
	lampCheckingSensorVal=analogRead(lampSensPin);
	lcd.print(".");
	if(lampCheckingSensorVal>LAMP_CHECKINGSENSOR_THRESHOLD){
		lampChecingkArray[0]=1;
	}
	(*lampState)=LOW;
	digitalWrite(lampStatePin, (*lampState));
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	lcd.print(".");
	if(lampCheckingSensorVal<LAMP_CHECKINGSENSOR_THRESHOLD){
		lampChecingkArray[1]=0;
	}
	(*lampState)=HIGH;
	digitalWrite(lampStatePin, (*lampState));
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	lcd.print(".");
	if(lampCheckingSensorVal>LAMP_CHECKINGSENSOR_THRESHOLD){
		lampChecingkArray[2]=1;
	}
	delay(LAMP_CHECKING_TIMEBREAK);
	/*controllare correttezza sintassi
	 */
	if(lampChecingkArray[0]&&(!lampChecingkArray[1])&&lampChecingkArray[2]){
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("Lamp ");
		lcd.setCursor(0, 1); lcd.print("is OK!!");
	}
	else{
		printWiringError(piezoLCPin);
	}
	delay(2500);
}

int simpleRead(void){
	// Simple data read example. Just read the infrared, fullspecrtrum diode 
	// or 'visible' (difference between the two) channels.
	// This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
	uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
	//uint16_t x = tsl.getLuminosity(TSL2561_FULLSPECTRUM);
	//uint16_t x = tsl.getLuminosity(TSL2561_INFRARED);

	Serial.print(">> "); Serial.println(x, DEC);		//rimuovere dal di qui
	readsVal=x;
}

void backgroundSensor (void){
	delay(1000);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Opening...");
	allSpectrumFile = SD.open(ALLSPECTRUM_FILENAME, FILE_WRITE);
	if(allSpectrumFile){
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("writing...");
		delay(2500);
		allSpectrumFile.println("All Specrtum Analysis Data");
		allSpectrumFile.print("SerialNumber:  ");
		allSpectrumFile.println("[__________]");
		allSpectrumFile.close();
	}
	else{
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("problem...  ):");
		delay(2500);
	}
}

#endif
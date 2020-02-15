//	FILE CONTENENTE TUTTE LE FUNZIONI 

#ifndef FUNCTIONS_h
#define FUNCTIONS_h

// =========================================================================================
// ======================================= PROTOTIPI =======================================
// =========================================================================================

int tslSensorInitializationConnection(Adafruit_TSL2591 *myTsl, LiquidCrystal myLcd);
// void tslConfigureSensor(void);
void tslConfigureSensor(Adafruit_TSL2591 *myTsl, int gainMultiplier, int integrationTime);
void printWiringError(const int alarmPin, LiquidCrystal myLcd);
void gratingMotorZeroPoint(const int sensMotPin, const int buzzerPin, LiquidCrystal myLcd);
void gratingMotorChecking(const int sensMotCheckPin, const int buzzerCheckPin);
int SDCardChecking(const int chipSel, LiquidCrystal myLcd);
int lampChecking(const int lampSensPin, const int lampStatePin, int lampState, const int buzzerLCPin, LiquidCrystal myLcd);
uint16_t simpleRead(Adafruit_TSL2591 *myTsl, int tslReadType);
void backgroundSensor(LiquidCrystal myLcd, File myFile);
void serialDisplaySensorDetails(Adafruit_TSL2591 *myTsl);
void waitingButtonRelease(int pinButton, int* buttonVal);


// =========================================================================================
// ======================================= FUNZIONI ========================================
// =========================================================================================

void serialSendInstrumentDetalis(void){
	Serial.println("[===================================================================]");
	Serial.println("|========================= STARTING SYSTEM =========================|");
	Serial.println("|===================================================================|");
	Serial.print("|\t#Product Name:\t\t\t"); 			Serial.println(INSTRUMENT_NAME);
	Serial.print("|\t#Model:\t\t\t\t"); 				Serial.println(MODEL_VERSION);
	Serial.print("|\t#Firmware:\t\t\t"); 				Serial.println(FIRMWARE_VERSION);
	Serial.print("|\t#Autor Name:\t\t\t"); 			Serial.println(AUTOR_NAME);
	Serial.print("|\t#Email:\t\t\t\t"); 				Serial.println(AUTOR_EMAIL);
	Serial.println("[===================================================================]");
}

void waitingButtonRelease(int pinButton, int *buttonVal){
	while(digitalRead(pinButton)){
		;
	}
	(*buttonVal)=LOW;
}

// Funzione per decidere quale sensibilità ed il tempo di integrazione che il mio sensore deve avere.
// Ora il valore è scolpito in fase di compilazione.
// In un futuro bisogna informarsi per capire se è possibile modificare questi dati
// mentre lo spettrofotometro è acceso
void tslConfigureSensor(Adafruit_TSL2591 *myTsl, int gainMultiplier, int integrationTime){
	switch(gainMultiplier){
		case TSL_GAIN_LOW:
			(*myTsl).setGain(TSL2591_GAIN_LOW);						// 1x gain (bright light)
			break;
		case TSL_GAIN_MED:
			(*myTsl).setGain(TSL2591_GAIN_MED);						// 25x gain
			break;
		case TSL_GAIN_HIGH:
			(*myTsl).setGain(TSL2591_GAIN_HIGH);					// 428x gain
			break;
		case TSL_GAIN_MAX:
			(*myTsl).setGain(TSL2591_GAIN_MAX);						// 9876x (extremely low light)
			break;
	}
	switch(integrationTime){
		case TSL_INTEGRATIONTIME_100ms:
			(*myTsl).setTiming(TSL2591_INTEGRATIONTIME_100MS);		// shortest integration time (bright light)
			break;
		case TSL_INTEGRATIONTIME_200ms:
			(*myTsl).setTiming(TSL2591_INTEGRATIONTIME_200MS);
			break;
		case TSL_INTEGRATIONTIME_300ms:
			(*myTsl).setTiming(TSL2591_INTEGRATIONTIME_300MS);
			break;
		case TSL_INTEGRATIONTIME_400ms:
			(*myTsl).setTiming(TSL2591_INTEGRATIONTIME_400MS);
			break;
		case TSL_INTEGRATIONTIME_500ms:
			(*myTsl).setTiming(TSL2591_INTEGRATIONTIME_500MS);
			break;
		case TSL_INTEGRATIONTIME_600ms:
			(*myTsl).setTiming(TSL2591_INTEGRATIONTIME_600MS);		// longest integration time (dim light)
			break;
	}
	Serial.print(">>\t#Gain:\t\t\t\t");
	tsl2591Gain_t gain = (*myTsl).getGain();
	switch(gain){
		case TSL2591_GAIN_LOW:
			Serial.println("1x(Low)");
			break;
		case TSL2591_GAIN_MED:
			Serial.println("25x(Medium)");
			break;
		case TSL2591_GAIN_HIGH:
			Serial.println("428x(High)");
			break;
		case TSL2591_GAIN_MAX:
			Serial.println("9876x(Max)");
			break;
	}
	Serial.print(">> \t#Timing:\t\t\t"); Serial.print(((*myTsl).getTiming() + 1) * 100, DEC); Serial.println("ms");
	delay(1);
}

// Displays some basic information on this sensor from the unified
// sensor API sensor_t type (see Adafruit_Sensor for more information)
void serialDisplaySensorDetails(Adafruit_TSL2591 *myTsl){
	sensor_t sensor;
	(*myTsl).getSensor(&sensor);
	Serial.print (">>\t#Sensor: \t\t\t"); 		Serial.println(sensor.name);
	Serial.print (">>\t#Driver Ver: \t\t\t"); 	Serial.println(sensor.version);
	Serial.print (">>\t#Unique ID: \t\t\t"); 	Serial.println(sensor.sensor_id);
	Serial.print (">>\t#Max Value: \t\t\t"); 	Serial.print(sensor.max_value); 	Serial.println(" lux");
	Serial.print (">>\t#Min Value: \t\t\t"); 	Serial.print(sensor.min_value); 	Serial.println(" lux");
	Serial.print (">>\t#Resolution: \t\t\t"); 	Serial.print(sensor.resolution); 	Serial.println(" lux");
}


// In caso di errori durante il check questa è la funzione per avvisare l'utente che qualcosa non va.
void printWiringError (const int alarmPin, LiquidCrystal myLcd){
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("OH-oh..");
	myLcd.setCursor(0, 1); myLcd.print("check wiring?");
	tone(alarmPin, 400, 1000); delay(500);
	tone(alarmPin, 400, 1000);
}

// Verifico che la comunicazione con il sensore tsl sia funzionante
int tslSensorInitializationConnection(Adafruit_TSL2591 *myTsl, LiquidCrystal myLcd){
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("Checking TSL...");
	if ((*myTsl).begin()){
		myLcd.setCursor(0, 1); myLcd.print("	OK!");
		return TSL_CONNECTION_DONE;
	} 
	else{
		return TSL_CONNECTION_FAILED;
		// printWiringError(PIN_BUZZER);
	}
}

// Azzero la posizione della griglia di rifrazione
void gratingMotorZeroPoint (const int sensMotCheckPin, const int buzzerPin, LiquidCrystal myLcd, Adafruit_StepperMotor *myMotor){
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("Grating Motor");
	myLcd.setCursor(0, 1); myLcd.print("Homing...");
	// int positionMotorSensorVal=analogRead(sensMotPin);
	// while(positionMotorSensorVal<=MOTOR_POSITIONSENSOR_THRESHOLD){
		// positionMotorSensorVal=analogRead(sensMotPin);
		// myMotor->step(1, BACKWARD, SINGLE); 
	// }
	//proseguo fintanto che non incontro lo zero macchina (quando la fotocellula vede la fessura nella "ruota fonica")
	while(analogRead(sensMotCheckPin)<=MOTOR_POSITIONSENSOR_THRESHOLD){
		myMotor->step(1, BACKWARD, SINGLE); 
	}
	//a questo punto torno indietro fino a che la mia superfice rifrangente non è perpendicolare al raggio della sorgente luminosa
	myMotor->step(MOTOR_STEPS_MACHINEZERO, FORWARD, SINGLE);
	tone(buzzerPin, 400, 500);
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("Motor Setted!");
	myLcd.setCursor(0, 1); myLcd.print("Dgr.= 0'");
}

// funzione inutile se non per scrivere sullo schermo LCD. DA INCORPORARE CON QUELLA SOPRA
// void gratingMotorChecking (const int sensMotCheckPin, const int buzzerCheckPin){
	// gratingMotorZeroPoint(sensMotCheckPin, buzzerCheckPin);
	// myLcd.clear();
	// myLcd.setCursor(0, 0); myLcd.print("Motor Setted!");
	// myLcd.setCursor(0, 1); myLcd.print("Dgr.= 0'");
	// delay(500);
// }

// verifico la scheda SD
int SDCardChecking (const int chipSel, LiquidCrystal myLcd){
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("Initializing");
	myLcd.setCursor(0, 1); myLcd.print("SD card...");
	delay(500);
	if (SD.begin(chipSel)) {
		myLcd.clear();
		myLcd.setCursor(0, 0); myLcd.print("Initialization");
		myLcd.setCursor(0, 1); myLcd.print("DONE!!!");
		delay(500);
		return 0;
	}
	else{
		printWiringError(PIN_BUZZER, myLcd);
		return 1;
	}
}

// verifico che la lampada sia in funzione e che il relè cambi di stato.
// Bisognerebbe gestire anche il punto in viene generato l'errore visto che utilizzo un array
int lampChecking (const int lampSensPin, const int lampStatePin, int lampState, const int buzzerLCPin, LiquidCrystal myLcd){
	int lampCheckingErrorSum=0;
	int lampCheckingSensorVal=0;
	// int lampChecingkArray[3]={0,0,0};
	//delay(2500);
	/*
	la logica dietro questo test è la seguente:
		- rilevo se è gia accesa (valotare se accendere solo quando necessario) e poi la spegno;
		- rilevo se è spenta e poi riaccendo;
		- rilevo se è accesa.
	Ad ogni step vado a salvare l'esito su un array così avrò modo, in caso di problemi, di sapere dove è avvenuto.
	Tra uno stato e l'altro do il tempo alla resistenza della lampada di spegnersi del tutto per evitare false lettura (verificare se si può abbassare il tempo).
	*/
	lampCheckingSensorVal=analogRead(lampSensPin);
	myLcd.print(".");
	if(lampCheckingSensorVal<LAMP_CHECKINGSENSOR_THRESHOLD){
		// lampChecingkArray[0]=1;
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_1;
	}
	lampState=LOW;
	digitalWrite(lampStatePin, lampState);
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	myLcd.print(".");
	if(lampCheckingSensorVal>LAMP_CHECKINGSENSOR_THRESHOLD){
		// lampChecingkArray[1]=1;
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_2;
	}
	lampState=HIGH;
	digitalWrite(lampStatePin, lampState);
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	myLcd.print(".");
	if(lampCheckingSensorVal<LAMP_CHECKINGSENSOR_THRESHOLD){
		// lampChecingkArray[2]=1;
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_3;
	}
	if(lampCheckingErrorSum==LAMP_CHECK_PASSED){
		return LAMP_CHECK_PASSED;
	}
	else{
		return lampCheckingErrorSum;
		// printWiringError(buzzerLCPin);
	}
	delay(1500);
}

/*
	Punzione per eseguire una semplicissima lettura.
	La tengo per il momento ma in realtà è totalmente inuitle in quanto talmente semplice.
	Al più bisogna vedere come gestire il campo di lettura in base alle impostazioni di default
*/
	// // Simple data read example. Just read the infrared, fullspecrtrum diode 
	// // or 'visible' (difference between the two) channels.
	// // This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
// int simpleRead(Adafruit_TSL2591 *myTsl){
	// uint16_t x = (*myTsl).getLuminosity(TSL2591_VISIBLE);
	// //uint16_t x = (*myTsl).getLuminosity(TSL2561_FULLSPECTRUM);
	// //uint16_t x = (*myTsl).getLuminosity(TSL2561_INFRARED);

	// Serial.print(">> "); Serial.println(x, DEC);		//rimuovere dal di qui
	// m_readsVal=x;
// }
// int simpleRead(int tslReadType){
uint16_t simpleRead(Adafruit_TSL2591 *myTsl, int tslReadType){
	switch(tslReadType){
		case(TSL_READTYPE_VISIBLE):{
			return (*myTsl).getLuminosity(TSL2591_VISIBLE);
			break;
		}
		case(TSL_READTYPE_FULLSPECTRUM):{
			return (*myTsl).getLuminosity(TSL2591_FULLSPECTRUM);
			break;
		}
		case(TSL_READTYPE_INFRARED):{
			return (*myTsl).getLuminosity(TSL2591_INFRARED);
			break;
		}
	}
}

/*
	Scrivo sulla scheda tutti i valori dello zero rispetto ad ogni lunghezza d'onda
*/
void backgroundSensor(LiquidCrystal myLcd, File myFile){
	delay(1000);
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("Opening...");
	myFile = SD.open(ALLSPECTRUM_FILENAME, FILE_WRITE);
	if(myFile){
		myLcd.clear();
		myLcd.setCursor(0, 0); myLcd.print("writing...");
		delay(2500);
		myFile.println("All Specrtum Analysis Data");
		myFile.print("SerialNumber:  ");
		myFile.println("[__________]");
		myFile.close();
	}
	else{
		myLcd.clear();
		myLcd.setCursor(0, 0); myLcd.print("problem...  ):");
		delay(2500);
	}
}

// ========================================== OLD ==========================================
// int simpleRead(void);
// int simpleRead(int tslReadType);

#endif
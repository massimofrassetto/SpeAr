//	FILE CONTENENTE TUTTE LE FUNZIONI 

#ifndef FUNCTIONS_h
#define FUNCTIONS_h

// =========================================================================================
// ======================================= PROTOTIPI =======================================
// =========================================================================================

int tslSensorInitializationConnection(void);
void tslConfigureSensor(void);
void printWiringError(const int alarmPin);
void gratingMotorZeroPoint(const int sensMotPin, const int buzzerPin);
void gratingMotorChecking(const int sensMotCheckPin, const int buzzerCheckPin);
int SDCardChecking(const int chipSel);
int lampChecking(const int lampSensPin, const int lampStatePin, int lampState, const int buzzerLCPin);
uint16_t simpleRead(int tslReadType);
void backgroundSensor(void);
void displaySensorDetails(void);
void waitingButtonRelease(int pinButton, int* buttonVal);


// =========================================================================================
// ======================================= FUNZIONI ========================================
// =========================================================================================

void waitingButtonRelease(int pinButton, int* buttonVal){
	while(digitalRead(pinButton)){
		;
	}
	(*buttonVal)=LOW;
}

// Funzione per decidere quale sensibilità ed il tempo di integrazione che il mio sensore deve avere.
// Ora il valore è scolpito in fase di compilazione.
// In un futuro bisogna informarsi per capire se è possibile modificare questi dati
// mentre lo spettrofotometro è acceso
void tslConfigureSensor(void){
	//tsl.setGain(TSL2591_GAIN_LOW);					// 1x gain (bright light)
	//tsl.setGain(TSL2591_GAIN_MED);						// 25x gain
	//tsl.setGain(TSL2591_GAIN_HIGH);		 				// 428x gain
	tsl.setGain(TSL2591_GAIN_MAX);							// 9876x (extremely low light)
 
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);			// shortest integration time (bright light)
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
	tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
	//tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);			// longest integration time (dim light)
 
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

// Displays some basic information on this sensor from the unified
// sensor API sensor_t type (see Adafruit_Sensor for more information)
void displaySensorDetails(void){
	sensor_t sensor;
	tsl.getSensor(&sensor);
	Serial.print (">> == Sensor: \t\t\t\t"); 	Serial.println(sensor.name);
	Serial.print (">> == Driver Ver: \t\t\t"); 	Serial.println(sensor.version);
	Serial.print (">> == Unique ID: \t\t\t"); 	Serial.println(sensor.sensor_id);
	Serial.print (">> == Max Value: \t\t\t"); 	Serial.print(sensor.max_value); 	Serial.println(" lux");
	Serial.print (">> == Min Value: \t\t\t"); 	Serial.print(sensor.min_value); 	Serial.println(" lux");
	Serial.print (">> == Resolution: \t\t\t"); 	Serial.print(sensor.resolution); 	Serial.println(" lux");
}


// In caso di errori durante il check questa è la funzione per avvisare l'utente che qualcosa non va.
void printWiringError (const int alarmPin){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("OH-oh..");
	lcd.setCursor(0, 1); lcd.print("check wiring?");
	tone(alarmPin, 400, 1000); delay(500);
	tone(alarmPin, 400, 1000);
}

// Verifico che la comunicazione con il sensore tsl sia funzionante
int tslSensorInitializationConnection(void){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Checking TSL...");
	if (tsl.begin()){
		lcd.setCursor(0, 1); lcd.print("	OK!");
		return TSL_CONNECTION_DONE;
	} 
	else{
		return TSL_CONNECTION_FAILED;
		// printWiringError(PIN_BUZZER);
	}
}

// Azzero la posizione della griglia di rifrazione
void gratingMotorZeroPoint (const int sensMotCheckPin, const int buzzerPin){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Grating Motor");
	lcd.setCursor(0, 1); lcd.print("Homing...");
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
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Motor Setted!");
	lcd.setCursor(0, 1); lcd.print("Dgr.= 0'");
}

// funzione inutile se non per scrivere sullo schermo LCD. DA INCORPORARE CON QUELLA SOPRA
// void gratingMotorChecking (const int sensMotCheckPin, const int buzzerCheckPin){
	// gratingMotorZeroPoint(sensMotCheckPin, buzzerCheckPin);
	// lcd.clear();
	// lcd.setCursor(0, 0); lcd.print("Motor Setted!");
	// lcd.setCursor(0, 1); lcd.print("Dgr.= 0'");
	// delay(500);
// }

// verifico la scheda SD
int SDCardChecking (const int chipSel){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Initializing");
	lcd.setCursor(0, 1); lcd.print("SD card...");
	delay(500);
	if (SD.begin(chipSel)) {
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("Initialization");
		lcd.setCursor(0, 1); lcd.print("DONE!!!");
		delay(500);
		return 0;
	}
	else{
		printWiringError(PIN_BUZZER);
		return 1;
	}
}

// verifico che la lampada sia in funzione e che il relè cambi di stato.
// Bisognerebbe gestire anche il punto in viene generato l'errore visto che utilizzo un array
int lampChecking (const int lampSensPin, const int lampStatePin, int lampState, const int buzzerLCPin){
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
	lcd.print(".");
	if(lampCheckingSensorVal<LAMP_CHECKINGSENSOR_THRESHOLD){
		// lampChecingkArray[0]=1;
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_1;
	}
	lampState=LOW;
	digitalWrite(lampStatePin, lampState);
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	lcd.print(".");
	if(lampCheckingSensorVal>LAMP_CHECKINGSENSOR_THRESHOLD){
		// lampChecingkArray[1]=1;
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_2;
	}
	lampState=HIGH;
	digitalWrite(lampStatePin, lampState);
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	lcd.print(".");
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
// int simpleRead(void){
	// uint16_t x = tsl.getLuminosity(TSL2591_VISIBLE);
	// //uint16_t x = tsl.getLuminosity(TSL2561_FULLSPECTRUM);
	// //uint16_t x = tsl.getLuminosity(TSL2561_INFRARED);

	// Serial.print(">> "); Serial.println(x, DEC);		//rimuovere dal di qui
	// m_readsVal=x;
// }
// int simpleRead(int tslReadType){
uint16_t simpleRead(int tslReadType){
	switch(tslReadType){
		case(TSL_READTYPE_VISIBLE):{
			return tsl.getLuminosity(TSL2591_VISIBLE);
			break;
		}
		case(TSL_READTYPE_FULLSPECTRUM):{
			return tsl.getLuminosity(TSL2591_FULLSPECTRUM);
			break;
		}
		case(TSL_READTYPE_INFRARED):{
			return tsl.getLuminosity(TSL2591_INFRARED);
			break;
		}
	}
}

/*
	Scrivo sulla scheda tutti i valori dello zero rispetto ad ogni lunghezza d'onda
*/
void backgroundSensor(void){
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

// ========================================== OLD ==========================================
// int simpleRead(void);
// int simpleRead(int tslReadType);

#endif
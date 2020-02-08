/* FILE CONTENENTE TUTTE LE FUNZIONI */

#ifndef FUNCTIONS.h
#define FUNCTIONS.h

// =========================================================================================
// ======================================= PROTOTIPI =======================================
// =========================================================================================

void configureSensor(void);
void printWiringError(const int allarmPin);
void gratingMotorZeroPoint(const int sensMotPin, const int buzzerPin);
void gratingMotorChecking(const int sensMotCheckPin, const int buzzerCheckPin);
void SDCardChecking(const int chipSel);
void lampChecking(const int lampSensPin, const int lampStatePin, int* lampState, const int buzzerLCPin);
// int simpleRead(void);
uint16_t simpleRead(int tslReadType);
void backgroundSensor(void);

// =========================================================================================
// ======================================= FUNZIONI ========================================
// =========================================================================================

/*
	Funzione per decidere quale sensibilità ed il tempo di integrazione che il mio sensore deve avere.
	Ora il valore è scolpito in fase di compilazione.
	In un futuro bisogna informarsi per capire se è possibile modificare questi dati
	mentre lo spettrofotometro è acceso
*/
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

/*
	In caso di errori durante il check questa è la funzione per avvisare l'utente che qualcosa non va.
*/
void printWiringError (const int allarmPin){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("OH-oh..");
	tone(allarmPin, 400, 1000); delay(500);
	tone(allarmPin, 400, 1000);
	lcd.setCursor(0, 1); lcd.print("check wiring?");
}

/*
	Verifico che la comunicazione con il sensore tsl sia funzionante
*/
void tslSensorChecking (void){
	lcd.clear();
	// lcd.setCursor(0, 0); lcd.print("TSL2591");
	lcd.setCursor(0, 0); lcd.print("Checking TSL..."); 
	delay(500);
	if (tsl.begin()){
		// lcd.clear();
		// lcd.setCursor(0, 0); lcd.print("TSL2591 Sensor");
		lcd.setCursor(0, 1); lcd.print("	OK!");
	} 
	else{
		printWiringError(PIN_BUZZER);
	}
	delay(500);
	//se il collegamento riesce lo configuro già (da portare fuori successivamente)
	configureSensor();
	delay(500);
}

/*
	Azzero la posizione della griglia di rifrazione
*/
void gratingMotorZeroPoint (const int sensMotPin, const int buzzerPin){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Grating Motor");
	lcd.setCursor(0, 1); lcd.print("Homing...");
	// int positionMotorSensorVal=analogRead(sensMotPin);
	// while(positionMotorSensorVal<=MOTOR_POSITIONSENSOR_THRESHOLD){
		// positionMotorSensorVal=analogRead(sensMotPin);
		// myMotor->step(1, BACKWARD, SINGLE); 
	// }
	//proseguo fintanto che non incontro lo zero macchina (quando la fotocellula vede la fessura nella "ruota fonica")
	while(analogRead(sensMotPin)<=MOTOR_POSITIONSENSOR_THRESHOLD){
		myMotor->step(1, BACKWARD, SINGLE); 
	}
	//a questo punto torno indietro fino a che la mia superfice rifrangente non è perpendicolare al raggio della sorgente luminosa
	myMotor->step(MOTOR_STEPS_MACHINEZERO, FORWARD, SINGLE);
	tone(buzzerPin, 400, 500);
}

/*
	funzione inutile se non per scrivere sullo schermo LCD. DA INCORPORARE CON QUELLA SOPRA
*/
void gratingMotorChecking (const int sensMotCheckPin, const int buzzerCheckPin){
	gratingMotorZeroPoint(sensMotCheckPin, buzzerCheckPin);
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Motor Setted!");
	lcd.setCursor(0, 1); lcd.print("Dgr.= 0'");
	delay(500);
}

/*
	verifico la scheda SD
*/
void SDCardChecking (const int chipSel){
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Initializing");
	lcd.setCursor(0, 1); lcd.print("SD card...");
	delay(500);
	if (!SD.begin(chipSel)) {
		printWiringError(PIN_BUZZER);
		delay(2500);
		return;
	}
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Initialization");
	lcd.setCursor(0, 1); lcd.print("DONE!!!");
	delay(500);
}

/*
	verifico che la lampada sia in funzione e che il relè cambi di stato.
	Bisognerebbe gestire anche il punto in viene generato l'errore visto che utilizzo un array
*/
void lampChecking (const int lampSensPin, const int lampStatePin, int* lampState, const int buzzerLCPin){
	int lampCheckingSensorVal=0;
	int lampChecingkArray[3]={0,0,0};
	lcd.clear();
	lcd.setCursor(0, 0); lcd.print("Lamp");
	lcd.setCursor(0, 1); lcd.print("Checking");
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
	if(lampChecingkArray[0]&&(!lampChecingkArray[1])&&lampChecingkArray[2]){
		lcd.clear();
		lcd.setCursor(0, 0); lcd.print("Lamp ");
		lcd.setCursor(0, 1); lcd.print("is OK!!");
	}
	else{
		printWiringError(buzzerLCPin);
	}
	delay(2500);
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
uint16_t simpleRead(int tslReadType){
	switch(tslReadType){
		case(TSL_READTYPE_VISIBLE):{
			return tsl.getLuminosity(TSL2591_VISIBLE);
			break;
		}
		// case(TSL_READTYPE_FULLSPECTRUM):{
			// return tsl.getLuminosity(TSL2561_FULLSPECTRUM);
			// break;
		// }
		// case(TSL_READTYPE_INFRARED):{
			// return tsl.getLuminosity(TSL2561_INFRARED);
			// break;
		// }
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

#endif
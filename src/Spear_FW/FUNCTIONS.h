//	FILE CONTENENTE TUTTE LE FUNZIONI 

#ifndef FUNCTIONS_h
#define FUNCTIONS_h

// =========================================================================================
// ======================================= PROTOTIPI =======================================
// =========================================================================================

void serialSendSystemDetalis(void);
void tslConfigureSensor(Adafruit_TSL2591 *myTsl, int gainMultiplier, int integrationTime);
void printWiringError(const int alarmPin, LiquidCrystal myLcd);
void gratingMotorZeroPoint(const int sensMotPin, const int buzzerPin, LiquidCrystal myLcd);
void gratingMotorChecking(const int sensMotCheckPin, const int buzzerCheckPin);
int lampChecking(const int lampSensPin, const int lampStatePin, int lampState, const int buzzerLCPin, LiquidCrystal myLcd);
uint16_t simpleRead(Adafruit_TSL2591 *myTsl, int tslReadType);
void serialDisplaySensorDetails(Adafruit_TSL2591 *myTsl);
void waitingButtonPressed(int pinButton, bool *buttonVal);
void waitingButtonReleased(int pinButton, bool* buttonVal);
void waitingButtonPressedFiltered(int pinButton, bool *buttonVal);
void waitingButtonReleasedFiltered(int pinButton, bool *buttonVal);
int SDinitPlusInfo(const int myChipSel);
String getTimeIntoString(DateTime now);

// ========================================== OLD ==========================================

// int tslSensorInitializationConnection(Adafruit_TSL2591 *myTsl, LiquidCrystal myLcd);
// int rtcChecking(RTC_DS1307 *myRtc);
// int SDCardChecking(const int chipSel);
// void serialTest(String myString);

// =========================================================================================
// ======================================= FUNZIONI ========================================
// =========================================================================================
// da spostare poi su un CPP

// void serialTest(String myString){
	// #ifdef SERIAL_OUTPUT
		// Serial.print(myString);
	// #endif
// }

	// Just a Serial.print of the general instrument info
void serialSendSystemDetalis(void){
	Serial.print("|\t#Product Name:\t\t\t"); 			Serial.println(INSTRUMENT_NAME);
	Serial.print("|\t#Model:\t\t\t\t"); 				Serial.println(MODEL_VERSION);
	Serial.print("|\t#Firmware:\t\t\t"); 				Serial.println(FIRMWARE_VERSION);
	Serial.print("|\t#Build Date:\t\t\t"); 				Serial.println(BUILD_TIMESTAP_DATE);
	Serial.print("|\t#Build Time:\t\t\t");				Serial.println(BUILD_TIMESTAP_TIME);
	Serial.print("|\t#Autor Name:\t\t\t"); 				Serial.println(AUTOR_NAME);
	Serial.print("|\t#Email:\t\t\t\t"); 				Serial.println(AUTOR_EMAIL);
}

// 
void serialSendInstrumentDetalis(void){
	Serial.print("|\t#Spectral lower Limit [nm]:\t"); 	Serial.println(SPECTRALIMIT_LOW);
	Serial.print("|\t#Spectral Higher Limit [nm]:\t"); 	Serial.println(SPECTRALIMIT_HIGH);
	Serial.print("|\t#ID TSL reading mode:\t\t"); 		Serial.println(ANALYSISMODE_SIMPLEREAD);
	Serial.print("|\t#DeltaLambda [nm/step]:\t\t");		Serial.println((SPECTRALIMIT_HIGH-SPECTRALIMIT_LOW)/(MOTOR_STEPS_GRATINGLIMIT_HIGH-MOTOR_STEPS_GRATINGLIMIT_LOW));
}

void waitingButtonPressed(int pinButton, bool *buttonVal){
	while(!digitalRead(pinButton)){
		;
	}
	(*buttonVal)=true;
	delay(BUTTON_ANTIDEBOUNCEFILTER_TIME);
}

void waitingButtonReleased(int pinButton, bool *buttonVal){
	while(digitalRead(pinButton)){
		;
	}
	(*buttonVal)=false;
	delay(BUTTON_ANTIDEBOUNCEFILTER_TIME);
}

void waitingButtonPressedFiltered(int pinButton, bool *buttonVal){
	int filterTime=BUTTON_ANTIDEBOUNCEFILTER_TIME;
	unsigned long lastStartTimeRecording=0;
	unsigned long lastStopTimeRecording=0;
	bool buttonPressValid=false;
	bool pinButtonVal=false;
	while(!buttonPressValid){
		// while(!digitalRead(pinButton)){
		while(!pinButtonVal){
			pinButtonVal=digitalRead(pinButton);
			lastStartTimeRecording=millis();
			// #ifdef DEBUG_SERIAL
				// Serial.print(".");
			// #endif
		}
		pinButtonVal=0;
		// #ifdef DEBUG_SERIAL
			// Serial.print("\n Pressed: "); Serial.print(lastStartTimeRecording);
		// #endif
		while(digitalRead(pinButton)&&!buttonPressValid){
			lastStopTimeRecording=millis();
			if(lastStopTimeRecording-lastStartTimeRecording>=filterTime){
				buttonPressValid=true;
			}
		}
		// #ifdef DEBUG_SERIAL
			// Serial.print(" - Released: "); Serial.print(lastStopTimeRecording); Serial.print(" - Result Press Test: "); Serial.print(buttonPressValid); Serial.print("("); Serial.print(lastStopTimeRecording-lastStartTimeRecording); Serial.print("/"); Serial.print(filterTime); Serial.println(")");
		// #endif
	}
	// buttonPressValid=false;
	// lastStartTimeRecording=0;
	// lastStopTimeRecording=0;
	(*buttonVal)=true;
}
 // >> ------------------------------------ TODO ------------------------------------- <<
 // >> -------------------------------- DA VERIFICARE -------------------------------- <<
 // >> -------------------------------- Serve davvero? ------------------------------- <<
void waitingButtonReleasedFiltered(int pinButton, bool *buttonVal){
	int filterTime=BUTTON_ANTIDEBOUNCEFILTER_TIME;
	unsigned long lastStartTimeRecording=0;
	unsigned long lastStopTimeRecording=0;
	bool buttonReleasedValid=false;
	while(!buttonReleasedValid){
		while(digitalRead(pinButton)){
			lastStartTimeRecording=millis();
		}
		#ifdef DEBUG_SERIAL
			Serial.print("Released: "); Serial.print(lastStartTimeRecording);
		#endif
		while(!digitalRead(pinButton)&&!buttonReleasedValid){
			lastStopTimeRecording=millis();
			if(lastStopTimeRecording-lastStartTimeRecording>=filterTime){
				buttonReleasedValid=true;
			}
		}
		#ifdef DEBUG_SERIAL
			Serial.print(" - Pressed: "); Serial.print(lastStopTimeRecording); Serial.print(" - Result Press Test: "); Serial.print(buttonReleasedValid); Serial.print("("); Serial.print(lastStopTimeRecording-lastStartTimeRecording); Serial.print("/"); Serial.print(filterTime); Serial.println(")");
		#endif
	}
	(*buttonVal)=false;
}

String getTimeIntoString(DateTime now){
	String dateTimeStringForm = String(now.year(), DEC)		+
								"/"							+
								String(now.month(), DEC)	+
								"/"							+
								String(now.day(), DEC)		+
								";"							+
								String(now.hour(), DEC)		+
								":"							+
								String(now.minute(), DEC)	+
								":"							+
								String(now.second(), DEC);
	return dateTimeStringForm;
}

	// Funzione per decidere quale sensibilità e quale tempo di integrazione che il mio sensore deve avere.
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
	#ifdef DEBUG_SERIAL
		Serial.print("\t#Gain:\t\t\t\t");
	#endif
	tsl2591Gain_t gain = (*myTsl).getGain();
	#ifdef DEBUG_SERIAL
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
		Serial.print("\t#Timing:\t\t\t"); Serial.print(((*myTsl).getTiming() + 1) * 100, DEC); Serial.println("ms");
		// delay(1);
	#endif
}

	// Displays some basic information on this sensor from the unified
	// sensor API sensor_t type (see Adafruit_Sensor for more information)

void serialDisplaySensorDetails(Adafruit_TSL2591 *myTsl){
	sensor_t sensor;
	(*myTsl).getSensor(&sensor);
	#ifdef DEBUG_SERIAL
		Serial.print ("\t#Sensor: \t\t\t"); 		Serial.println(sensor.name);
		Serial.print ("\t#Driver Ver: \t\t\t"); 	Serial.println(sensor.version);
		Serial.print ("\t#Unique ID: \t\t\t"); 	Serial.println(sensor.sensor_id);
		Serial.print ("\t#Max Value: \t\t\t"); 	Serial.print(sensor.max_value, DEC); 	Serial.println(" lux");
		Serial.print ("\t#Min Value: \t\t\t"); 	Serial.print(sensor.min_value, DEC); 	Serial.println(" lux");
		Serial.print ("\t#Resolution: \t\t\t"); 	Serial.print(sensor.resolution, DEC); 	Serial.println(" lux");
	#endif
}


	// In caso di errori durante il check questa è la funzione per avvisare l'utente che qualcosa non va.

void printWiringError (const int alarmPin, LiquidCrystal myLcd){
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("OH-oh..");
	myLcd.setCursor(0, 1); myLcd.print("check wiring?");
	tone(alarmPin, 400, 1000); delay(500);
	tone(alarmPin, 400, 1000);
}

	// Azzero la posizione della griglia di rifrazione
	// TODO -> myMotor è una variabile globale...

void gratingMotorZeroPoint (const int sensMotCheckPin, const int buzzerPin, LiquidCrystal myLcd, Adafruit_StepperMotor *myMotor){
	myLcd.clear();
	myLcd.setCursor(0, 0); myLcd.print("Grating Motor");
	myLcd.setCursor(0, 1); myLcd.print("Homing...");
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

	// Funzione per scrivere qualche info della schedina SD.
int SDinitPlusInfo(const int myChipSel){
	Sd2Card card;
	SdVolume volume;
	SdFile root;
	if (!card.init(SPI_HALF_SPEED, myChipSel)) {
		return SD_INITIALIZING_CARD_FAILED;
	}
	else if (!volume.init(card)) {
		return SD_INITIALIZING_VOLUME_FAILED;
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Initialized");
			Serial.print("\t#Card type:\t\t\t");
			switch(card.type()){
				case SD_CARD_TYPE_SD1:
					Serial.println("SD1");
					break;
				case SD_CARD_TYPE_SD2:
					Serial.println("SD2");
					break;
				case SD_CARD_TYPE_SDHC:
					Serial.println("SDHC");
					break;
				default:
					Serial.println("Unknown");
			}
			Serial.print("\t#Clusters:\t\t\t");			Serial.println(volume.clusterCount());
			Serial.print("\t#Blocks x Cluster:\t\t");		Serial.println(volume.blocksPerCluster());
			Serial.print("\t#Total Blocks:\t\t\t");		Serial.println(volume.blocksPerCluster() * volume.clusterCount());
			uint32_t volumesize;								// print the type and size of the first FAT-type volume
			Serial.print("\t#Volume type is:\t\tFAT");	Serial.println(volume.fatType(), DEC);
			volumesize = volume.blocksPerCluster();				// clusters are collections of blocks
			volumesize *= volume.clusterCount();				// we'll have a lot of clusters
			volumesize /= 2;									// SD card blocks are always 512 bytes (2 blocks are 1KB)
			Serial.print("\t#Volume size (Kb):\t\t");		Serial.println(volumesize);
			volumesize /= 1024;
			Serial.print("\t#Volume size (Mb):\t\t");		Serial.println(volumesize);
			Serial.print("\t#Volume size (Gb):\t\t");		Serial.println((float)volumesize / 1024.0);
			Serial.println("\t#Files found on the card (name, date and size in bytes):");
			Serial.println("---------------------------------------------------------------------");
			root.openRoot(volume);
			root.ls( LS_R | LS_DATE | LS_SIZE );				// list all files in the card with date and size
			Serial.println("---------------------------------------------------------------------");
			return SD_INITIALIZING_DONE;
		#endif
	}
}

	// verifico che la lampada sia in funzione e che il relè cambi di stato.
	// Bisognerebbe gestire anche il punto in viene generato l'errore visto che utilizzo un array

int lampChecking (const int lampSensPin, const int lampStatePin, int lampState, const int buzzerLCPin, LiquidCrystal myLcd){
	int lampCheckingErrorSum=0;
	int lampCheckingSensorVal=0;
	 // la logica dietro questo test è la seguente:
	 //	- rilevo se è gia accesa (valutare se accendere solo quando necessario);
	 // - La spegno e rilevo se è spenta ;
	 // - La riaccendo e rilevo se è accesa.
	 // Ad ogni check vado a salvare l'esito su un array così avrò modo, in caso di problemi, di sapere dove è avvenuto.
	 // Tra uno stato e l'altro do il tempo alla resistenza della lampada di spegnersi del tutto per evitare false lettura (verificare se si può abbassare il tempo).
	lampCheckingSensorVal=analogRead(lampSensPin);
	myLcd.print(".");
	if(lampCheckingSensorVal<LAMP_CHECKINGSENSOR_THRESHOLD){
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_1;
	}
	lampState=LOW;
	digitalWrite(lampStatePin, lampState);
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	myLcd.print(".");
	if(lampCheckingSensorVal>LAMP_CHECKINGSENSOR_THRESHOLD){
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_2;
	}
	lampState=HIGH;
	digitalWrite(lampStatePin, lampState);
	delay(LAMP_CHECKING_TIMEBREAK);
	lampCheckingSensorVal=analogRead(lampSensPin);
	myLcd.print(".");
	if(lampCheckingSensorVal<LAMP_CHECKINGSENSOR_THRESHOLD){
		lampCheckingErrorSum+=LAMP_CHECK_ERROR_PHASE_3;
	}
	if(lampCheckingErrorSum==LAMP_CHECK_PASSED){
		return LAMP_CHECK_PASSED;
	}
	else{
		return lampCheckingErrorSum;
	}
	delay(1500);
}

	// Funzione per eseguire una semplicissima lettura.
	// La tengo per il momento ma in realtà è totalmente inuitle in quanto talmente semplice.
	// Al più bisogna vedere come gestire il campo di lettura in base alle impostazioni di default

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





// ========================================== OLD ==========================================

// Verifico che la comunicazione con il sensore tsl sia funzionante
// int tslSensorInitializationConnection(Adafruit_TSL2591 *myTsl, LiquidCrystal myLcd){
	// // myLcd.clear();
	// // myLcd.setCursor(0, 0); myLcd.print("Checking TSL...");
	// if ((*myTsl).begin()){
		// // myLcd.setCursor(0, 1); myLcd.print("	OK!");
		// return TSL_CONNECTION_DONE;
	// } 
	// else{
		// return TSL_CONNECTION_FAILED;
		// // printWiringError(BUZZER_PIN);
	// }
// }

// funzione inutile se non per scrivere sullo schermo LCD. DA INCORPORARE CON QUELLA SOPRA
// void gratingMotorChecking (const int sensMotCheckPin, const int buzzerCheckPin){
	// gratingMotorZeroPoint(sensMotCheckPin, buzzerCheckPin);
	// myLcd.clear();
	// myLcd.setCursor(0, 0); myLcd.print("Motor Setted!");
	// myLcd.setCursor(0, 1); myLcd.print("Dgr.= 0'");
	// delay(500);
// }

// verifico la scheda SD
// int SDCardChecking (const int chipSel){
	// // delay(1);
	// if (SD.begin(chipSel)) {
		// // Serial.print("si");
		// return SD_CONNECTION_DONE;
	// }
	// else{
		// // Serial.print("no");
		// return SD_CONNECTION_FAILED;
	// }
	// // Serial.print("forse");
// }

// int simpleRead(void);
// int simpleRead(int tslReadType);

	// Simple data read example. Just read the infrared, fullspecrtrum diode 
	// or 'visible' (difference between the two) channels.
	// This can take 100-600 milliseconds! Uncomment whichever of the following you want to read
// int simpleRead(Adafruit_TSL2591 *myTsl){
	// uint16_t x = (*myTsl).getLuminosity(TSL2591_VISIBLE);
	// //uint16_t x = (*myTsl).getLuminosity(TSL2561_FULLSPECTRUM);
	// //uint16_t x = (*myTsl).getLuminosity(TSL2561_INFRARED);

	// Serial.print(" "); Serial.println(x, DEC);		//rimuovere dal di qui
	// m_readsVal=x;
// }
#endif
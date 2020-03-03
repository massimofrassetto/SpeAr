/* PROGRAMMA PRINCIPALE */

#define DEBUG_SERIAL
// #define SERIAL_OUTPUT

#include <LiquidCrystal.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

#include <Adafruit_MotorShield.h>

#include <RTClib.h>

#include <SPI.h>
#include <SD.h>

#include <Keypad.h>

#include "CONSTANTS.h"
#include "MSG.h"
#include "FUNCTIONS.h"

Adafruit_TSL2591 m_tsl = Adafruit_TSL2591(ADAFRUIT_SENSOR_IDENTIFIER); 						// pass in a number for the sensor identifier (for your use later)

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_StepperMotor *m_grtMotor = AFMS.getStepper(MOTOR_STEPS_PER_REVOLUTION, MOTOR_PORT);

RTC_DS1307 m_rtc;

LiquidCrystal m_lcd(LCD_PIN_RS, LCD_PIN_ENABLE, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);

File m_spearTraceLogFile;
// File m_allSpectrumFile;
// File m_concAnalysisFile;

const byte ROWS = KEYPAD_ROWS;
const byte COLS = KEYPAD_COLS;
char hexaKeys[ROWS][COLS] = {
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};
byte rowPins[ROWS] = {KEYPAD_PIN_ROW_0, KEYPAD_PIN_ROW_1, KEYPAD_PIN_ROW_2, KEYPAD_PIN_ROW_3};
byte colPins[COLS] = {KEYPAD_PIN_COLS_0, KEYPAD_PIN_COLS_1, KEYPAD_PIN_COLS_2};
Keypad m_customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
char m_customKey;
String m_keyPadString;

String m_analysisModeLcdString[ANALYSISMODE_NUMBER]={ANALYSISMODE_LCDSTRING_MOD_1, ANALYSISMODE_LCDSTRING_MOD_2, ANALYSISMODE_LCDSTRING_MOD_3};
String m_menuVoices[MENU_VOICES_NUMBER]={MENU_LCDSTRING_ANALYSIS, MENU_LCDSTRING_SETTINGS, MENU_LCDSTRING_VOICE_3, MENU_LCDSTRING_VOICE_4};
bool m_refreshScreen=true;

int m_gratingMotorFutureSteps;
int m_gratingMotorCurrentSteps;

char m_daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime m_now;

#ifdef DEBUG_SERIAL
	String serialDebugString="";
#endif

int m_lambdaMin;
int m_lambdaMax;
int m_lambdaRequested;
int m_lambdaCorrected;
float m_lambdaCaptured;
int m_nReadsRequested;
int m_nReadsCorrected;
int m_nReadsCaptured;
int m_readsVal=0;
float m_sumBackgroundReadsVal=0;
float m_sumSampleReadsVal=0;

bool m_backVal=false;
bool m_nextVal=false;
bool m_upVal=false;
bool m_downVal=false;
bool m_okVal=false;

int m_lampSwitchVal;
int m_lampCheckingResult=0;
int m_indexAnalysisMode=0;
int m_indexMenuVoices=0;
bool m_AnalysisModeSelected=false;
int m_backgroundSensorVal;
int m_readSensorVal;
int m_allSpectrumScanID=0;
int m_allSpectrumScanReadIndex=0;
int m_menuArrowCursorPostion=0;

int m_deltaLambdaPerSingleStep=(SPECTRALIMIT_HIGH-SPECTRALIMIT_LOW)/(MOTOR_STEPS_GRATINGLIMIT_HIGH-MOTOR_STEPS_GRATINGLIMIT_LOW);	// [NM/STEP]

// ===========================================================================================================
// ================== Inizio fase di preparazione ed inizializzazione dello strumento  =======================
// ===========================================================================================================

void setup(void){
	tone(BUZZER_PIN, 500, 100); delay(100);
	tone(BUZZER_PIN, 300, 200); delay(100);
	tone(BUZZER_PIN, 500, 400); delay(4300);
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial) {
		;		// wait for serial port to connect. Needed for native USB port only
	}
	#ifdef DEBUG_SERIAL
		Serial.println("[===================================================================]");
		Serial.println("|========================= STARTING SYSTEM =========================|");
		Serial.println("|===================================================================|");
		serialSendSystemDetalis();
		Serial.println("[========================= METROLOGIC INFO =========================]");
		serialSendInstrumentDetalis();
		Serial.println("[=================== Initializing Connected Devices ================]");
	#endif
	m_lcd.begin(LCD_COLS, LCD_ROWS);
	#ifdef DEBUG_SERIAL
		Serial.println(">> LCD Inizializated;");
	#endif
	m_lcd.setCursor(0, 0); m_lcd.print("Spe.Ar. Project");
	m_lcd.setCursor(0, 1); m_lcd.print("V "); m_lcd.print(MODEL_VERSION); m_lcd.print(" SW "); m_lcd.print(FIRMWARE_VERSION);
	pinMode(BUTTON_PIN_OK, 				INPUT);
	pinMode(BUTTON_PIN_BACK, 			INPUT);
	pinMode(BUTTON_PIN_NEXT, 			INPUT);
	pinMode(BUTTON_PIN_UP, 				INPUT);
	pinMode(BUTTON_PIN_DOWN, 			INPUT);
	pinMode(LAMP_PIN_CHECKINGSENSOR, 	INPUT);
	pinMode(LAMP_PIN_SWITCH, 			OUTPUT);
	digitalWrite(LAMP_PIN_SWITCH, 		HIGH);
	#ifdef DEBUG_SERIAL
		Serial.println(">> I/O Inizializated;");
	#endif
	// Mi connetto all’RTC e verifico che sia in funzione. Se ció non fosse probabilmente bisogna cambiare la pila.
	#ifdef DEBUG_SERIAL
		Serial.print(">> RTC Connection...\t\t\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Initializing");
	m_lcd.setCursor(0, 1); m_lcd.print("RTC...");
	if(m_rtc.begin()){
		m_lcd.print("OK!");
		#ifdef DEBUG_SERIAL
			Serial.println("Established");
		#endif
		if (!m_rtc.isrunning()) {
			#ifdef DEBUG_SERIAL
				Serial.println(">>\t#RTC is NOT running!");
				Serial.println(">>\t#Build timestamp will be set!");
			#endif
			m_rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));				// following line sets the RTC to the date & time this sketch was compiled
			// rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));					// This line sets the RTC with an explicit date & time, for example to set January 21, 2014 at 3am you would call:
		}
		else{
			// m_rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));			//Imposta l'orario in modo rapido scommentando questa riga [Stile BRUTAL force] - In futuro l'orario e la data saranno configurabili da pannellino. TODO
			m_now = m_rtc.now();
			#ifdef DEBUG_SERIAL
				Serial.print(">>\t#Current Time:\t\t\t");
				Serial.print(getTimeIntoString(m_now));
				Serial.print("(");
				Serial.print(m_daysOfTheWeek[m_now.dayOfTheWeek()]);
				Serial.println(")");
			#endif
		}
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Failed");
		#endif
		m_lcd.print("ERR!");
		printWiringError(BUZZER_PIN, m_lcd);
		waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
		// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
		// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
		waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	}
	// avvio la comuncazione con la scheda motori (non è gestito il caso di fallita comunicazione TODO
	AFMS.begin();
	#ifdef DEBUG_SERIAL
		Serial.println(">> Motor Inizializated;");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Starting");
	m_lcd.setCursor(0, 1); m_lcd.print("Instrument...");
	delay(500);
	//Inizializzo la comunicazione con la schedina SD, rilevo alcune informazioni e scrivo subito sul primo file 
	#ifdef DEBUG_SERIAL
		Serial.print(">> Creating SD Card connection...\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Initializing");
	m_lcd.setCursor(0, 1); m_lcd.print("SD card...");
	if(SD.begin(SD_PIN_CHIPSELECT)){
		m_lcd.print("OK!");
		#ifdef DEBUG_SERIAL
			Serial.println("Established");
			Serial.print(">> Initializing SD Card...\t\t");
		#endif
		switch(SDinitPlusInfo(SD_PIN_CHIPSELECT)){
			case SD_INITIALIZING_CARD_FAILED:
				#ifdef DEBUG_SERIAL
					Serial.println("Failed");
					Serial.println(">> !!! Initialization Failed !!!");
				#endif
				printWiringError(BUZZER_PIN, m_lcd);
				break;
			case SD_INITIALIZING_VOLUME_FAILED:
				#ifdef DEBUG_SERIAL
					Serial.println("Failed");
					Serial.println(">> !!! Could not find FAT16/FAT32 partition !!!");
					Serial.println(">> !!! Make sure you've formatted the card !!!");
				#endif
				printWiringError(BUZZER_PIN, m_lcd);
				break;
		}
		#ifdef DEBUG_SERIAL
			Serial.print(">> Starting TraceLog record...\t\t");
		#endif
		m_now = m_rtc.now();
		m_spearTraceLogFile = SD.open(SPEARTRACELOG_FILENAME, FILE_WRITE);
		if (m_spearTraceLogFile){
			m_spearTraceLogFile.print(getTimeIntoString(m_now));
			m_spearTraceLogFile.println(";--------- Tracelog Started... ---------");
			m_spearTraceLogFile.close();
			Serial.println("DONE");
		}
		else{
			Serial.println("Error opening TraceLog");
			
		}
		// ---------------------------------- non scrive sugli altri file.... -----------------------------
		// ---------------------------------- indagare perchè... ------------------------------------------
		// delay(1000);
		// m_allSpectrumFile = SD.open(ALLSPECTRUM_FILENAME, FILE_WRITE);
		// if(m_allSpectrumFile){
			// m_allSpectrumFile.println("--------- Passed from here ---------");
			// m_allSpectrumFile.close();
		// }
		// else{
			// Serial.println("Error opening allSpec");
			// // m_allSpectrumFile.close();
		// }
		// delay(1000);
		// m_concAnalysisFile = SD.open(CONCANLYSIS_FILENAME, FILE_WRITE);
		// if(m_concAnalysisFile){
			// m_concAnalysisFile.println("--------- Passed from here ---------");
			// m_concAnalysisFile.close();
		// }
		// else{
			// Serial.println("Error opening conAnalysis");
			// // m_concAnalysisFile.close();
		// }
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Failed");
		#endif
		m_lcd.print("ERR!");
		printWiringError(BUZZER_PIN, m_lcd);
		waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
		// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
		// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
		waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	}
	#ifdef DEBUG_SERIAL
		Serial.print(">> Creating connection to TSL Sensor...\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Init.ing TSL...");
	if(m_tsl.begin()){
		#ifdef DEBUG_SERIAL
			Serial.println("Established");
		#endif
		m_lcd.setCursor(0, 1); m_lcd.print("	OK!");
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.println("Failed");
		#endif
		m_lcd.setCursor(0, 1); m_lcd.print("	FAILED!");
		printWiringError(BUZZER_PIN, m_lcd);
	}
	serialDisplaySensorDetails(&m_tsl);
	#ifdef DEBUG_SERIAL
		Serial.println("=================== Configuring Connected Devices ===================");
	#endif
	#ifdef DEBUG_SERIAL
		Serial.print(">> Speed Motor Configured:\t\t"); Serial.print(MOTOR_SPEED_RPM); Serial.println("rpm");
	#endif
	m_grtMotor->setSpeed(MOTOR_SPEED_RPM);
	#ifdef DEBUG_SERIAL
		Serial.print(">> Checking Lamp...\t\t\t");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Lamp");
	m_lcd.setCursor(0, 1); m_lcd.print("Checking");
	m_lampCheckingResult=lampChecking(LAMP_PIN_CHECKINGSENSOR, LAMP_PIN_SWITCH, m_lampSwitchVal, BUZZER_PIN, m_lcd);
	if(m_lampCheckingResult==LAMP_CHECK_PASSED){
		#ifdef DEBUG_SERIAL
			Serial.println("WORKING");
		#endif
		m_lcd.setCursor(0, 1); m_lcd.print("is OK!!         ");
	}
	else{
		#ifdef DEBUG_SERIAL
			Serial.print("Error!!:\t"); Serial.println(m_lampCheckingResult);
		#endif
		m_lcd.clear();
		m_lcd.setCursor(0, 0); m_lcd.print("!!ERRORE!!");
		m_lcd.setCursor(0, 1); m_lcd.print("Bin code: "); m_lcd.print(String(m_lampCheckingResult));
		tone(BUZZER_PIN, 400, 1000); delay(500);
		tone(BUZZER_PIN, 400, 1000);
	}
	#ifdef DEBUG_SERIAL
		Serial.println(">> Configuring TSL Sensor...");
	#endif
	tslConfigureSensor(&m_tsl, TSL_GAIN_MAX, TSL_INTEGRATIONTIME_300ms);
	#ifdef DEBUG_SERIAL
		Serial.println(">> Configuring Keypad...");
		Serial.print(">>\t#Debounce Filter:\t\t"); 			Serial.println(KEYPAD_ANTIDEBOUNCEFILTER_TIME);
	#endif
	m_customKeypad.setDebounceTime(KEYPAD_ANTIDEBOUNCEFILTER_TIME);
	#ifdef DEBUG_SERIAL
		Serial.println("===================================================================");
		Serial.println("===================== Initialization COMPLETED ====================");
		Serial.print("============================= "); Serial.print(millis()/1000.0, 3); Serial.println("s ==============================");
	#endif
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Instrument");
	m_lcd.setCursor(0, 1); m_lcd.print("is Ready!!!!");			//Migliorare l'effetto felice, ora sembra che semplicemente funzioni male
	tone(BUZZER_PIN, 600, 200); delay(200);
	tone(BUZZER_PIN, 600, 200); delay(100);
	tone(BUZZER_PIN, 900, 400); delay(1000);
	m_lcd.clear();
	m_lcd.setCursor(0, 0); m_lcd.print("Press 'OK'");
	m_lcd.setCursor(0, 1); m_lcd.print("to Start...");
	waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
	// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
	// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
	waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
}

// ===========================================================================================================
// ========================================== Fine Inizializzazione ==========================================
// ===================================== Inizio programma vero e proprio =====================================
// ===========================================================================================================

void loop(void){
	// Entriamo subito nel menu principale.
	// Per navigare tra i vari menu bisogna utilizzare le freccie UP/DOWN.
	// Prenendo OK invece si seleziona la voce affianco alla freccia.
	// Se si raggiunge la fine o l'inizio dell'elenco il sw è fatto in modo da fermarsi.
	// Non è possibile saltare dall'ultima voce alla prima (per il momento).
	m_lcd.clear();
	m_lcd.setCursor(0, m_menuArrowCursorPostion); m_lcd.print(">");
	if(m_indexMenuVoices==MENU_VOICES_NUMBER-1 || m_menuArrowCursorPostion==1){
		m_lcd.setCursor(1, 0); m_lcd.print(m_menuVoices[m_indexMenuVoices-1]);
		m_lcd.setCursor(1, 1); m_lcd.print(m_menuVoices[m_indexMenuVoices]);
	}
	else{
		m_lcd.setCursor(1, 0); m_lcd.print(m_menuVoices[m_indexMenuVoices]);
		m_lcd.setCursor(1, 1); m_lcd.print(m_menuVoices[m_indexMenuVoices+1]);
	}
	while(!digitalRead(BUTTON_PIN_OK)){
		// Serial.println(m_indexMenuVoices);
		m_upVal=	digitalRead(BUTTON_PIN_UP);
		m_downVal=	digitalRead(BUTTON_PIN_DOWN);
		if(!m_upVal && m_downVal){
			if(m_indexMenuVoices<MENU_VOICES_NUMBER-1){
				m_indexMenuVoices++;
			}
			if(m_menuArrowCursorPostion==0){
				m_lcd.setCursor(0, 0); m_lcd.print(" ");
				m_lcd.setCursor(0, 1); m_lcd.print(">");
				m_menuArrowCursorPostion=1;
			}
			else{
				if(m_indexMenuVoices<MENU_VOICES_NUMBER){
					m_lcd.setCursor(1, 0); m_lcd.print(m_menuVoices[m_indexMenuVoices-1]);
					m_lcd.setCursor(1, 1); m_lcd.print(m_menuVoices[m_indexMenuVoices]);
				}
			}
			waitingButtonPressedFiltered(BUTTON_PIN_DOWN, &m_downVal);
			waitingButtonReleased(BUTTON_PIN_DOWN, &m_downVal);
		}
		else if(m_upVal && !m_downVal){
			if(m_indexMenuVoices>0){
				m_indexMenuVoices--;
			}
			if(m_menuArrowCursorPostion==1){
				m_lcd.setCursor(0, 0); m_lcd.print(">");
				m_lcd.setCursor(0, 1); m_lcd.print(" ");
				m_menuArrowCursorPostion=0;
			}
			else{
				m_lcd.setCursor(1, 0); m_lcd.print(m_menuVoices[m_indexMenuVoices]);
				m_lcd.setCursor(1, 1); m_lcd.print(m_menuVoices[m_indexMenuVoices+1]);
			}
			waitingButtonPressedFiltered(BUTTON_PIN_UP, &m_upVal);
			waitingButtonReleased(BUTTON_PIN_UP, &m_upVal);
		}
	}
	// waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
	waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
	// #ifdef DEBUG_SERIAL
		// Serial.print("/"); Serial.print(m_okVal); Serial.print("/"); Serial.print(m_backVal); Serial.print("/"); Serial.print(m_nextVal); Serial.print("/"); Serial.print(m_upVal); Serial.print("/"); Serial.println(m_downVal);
	// #endif
	m_refreshScreen=true;
	// Ora che ho selezione la voce del menu entro nel corrispettivo blocco.
	switch(m_indexMenuVoices){
		// Menu per gestire tutte le varie tipologie di analisi
		case(MENU_LABORATORY):{
			while(!digitalRead(BUTTON_PIN_BACK)){
				#ifdef DEBUG_SERIAL
					Serial.println(">> Entering 'Selection Mode' Menu");
				#endif
				// #ifdef DEBUG_SERIAL
					// Serial.print("/"); Serial.print(m_okVal); Serial.print("/"); Serial.print(m_backVal); Serial.print("/"); Serial.print(m_nextVal); Serial.print("/"); Serial.print(m_upVal); Serial.print("/"); Serial.println(m_downVal);
				// #endif
				m_customKey=0;
				m_AnalysisModeSelected=false;
				// MODES:
				// 1 => SIMPLE READ
				// 2 => ALL SPECTRUM
				// 3 => CONCANALYSIS
				while(!m_okVal && !m_backVal){
					m_okVal=	digitalRead(BUTTON_PIN_OK);
					m_backVal=	digitalRead(BUTTON_PIN_BACK);
					m_upVal=	digitalRead(BUTTON_PIN_UP);
					m_downVal=	digitalRead(BUTTON_PIN_DOWN);
					// Questo if serve a non scrivere la prima stringa (fissa per il momento) tutte le volte che si passa dal di qua ma solo la prima volta.
					if(m_refreshScreen){
						m_lcd.clear();
						m_lcd.setCursor(0, 0); m_lcd.print("Sel. Analysis:");
					}
					if(!m_upVal && m_downVal){
						if(m_indexAnalysisMode>=ANALYSISMODE_NUMBER-1){
							m_indexAnalysisMode=0;
						}
						else{
							m_indexAnalysisMode++;
						}
						// waitingButtonReleasedFiltered(BUTTON_PIN_DOWN, &m_downVal);
						waitingButtonReleased(BUTTON_PIN_DOWN, &m_downVal);
						m_refreshScreen=true;
					}
					else if(m_upVal && !m_downVal){
						if(m_indexAnalysisMode<=0){
							m_indexAnalysisMode=ANALYSISMODE_NUMBER-1;
						}
						else{
							m_indexAnalysisMode--;
						}
						// waitingButtonReleasedFiltered(BUTTON_PIN_UP, &m_upVal);
						waitingButtonReleased(BUTTON_PIN_UP, &m_upVal);
						m_refreshScreen=true;
					}
					if(m_refreshScreen){
						// Pulisco la mia riga in quanto non so se la prossima stringa andrà a coprire tutta la precedente.
						m_lcd.setCursor(0, 1); m_lcd.print("                ");
						m_lcd.setCursor(0, 1); m_lcd.print(String(m_indexAnalysisMode+1) + "-" + m_analysisModeLcdString[m_indexAnalysisMode]);
						m_refreshScreen=false;
					}
					if(m_okVal && !m_backVal){
						m_AnalysisModeSelected=true;
					}
					else if(!m_okVal && m_backVal){
						m_AnalysisModeSelected=false;
					}
				}
				if(m_AnalysisModeSelected){
					// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
					waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
					#ifdef DEBUG_SERIAL
						Serial.print(">> Mode Selected:\t\t\t"); Serial.print(m_indexAnalysisMode+1); Serial.print(" - "); Serial.println(m_analysisModeLcdString[m_indexAnalysisMode]);
						Serial.print(">> Waiting confirm...\t\t\t");
					#endif
					switch(m_indexAnalysisMode){
						// Semplice lettura di assorbanza.
						// Dato un bianco di base posso inserire tutti i campioni che voglio.
						// Premendo "Back" si esce tornando indietro al menu di selezione delle modalità.
						case(ANALYSISMODE_SIMPLEREAD):{
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("Simple Read");
							m_lcd.setCursor(0, 1); m_lcd.print("selected?");
							while(!m_okVal && !m_backVal){
								m_okVal=digitalRead(BUTTON_PIN_OK);
								m_backVal=digitalRead(BUTTON_PIN_BACK);
								if(m_okVal){
									#ifdef DEBUG_SERIAL
										Serial.println("Simple Read Selected!");
									#endif
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Lambda (nm): ");
									m_lcd.setCursor(0, 1); m_lcd.print("Set Val: ");
									#ifdef DEBUG_SERIAL
										Serial.print(">> Lambda requested(nm):\t\t");
									#endif
									m_keyPadString="";
									while(!digitalRead(BUTTON_PIN_OK)){
										m_customKey=0;
										m_customKey=m_customKeypad.getKey();
										if(m_customKey){
											m_keyPadString+=m_customKey;
											m_lcd.setCursor(0, 1); m_lcd.print("Set Val: " + m_keyPadString);
											#ifdef DEBUG_SERIAL
												Serial.print(m_customKey);
											#endif
										}
									}
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									#ifdef DEBUG_SERIAL
										Serial.println();
									#endif
									m_lambdaRequested=m_keyPadString.toInt();
									delay(200);
									m_lambdaCorrected=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Lambda: ");
									m_lcd.setCursor(0, 1); m_lcd.print(String(m_lambdaCorrected) + "nm");
									#ifdef DEBUG_SERIAL
										if(m_lambdaCorrected>m_lambdaRequested){
											Serial.print(">> Lambda can't be lower then "); Serial.print(SPECTRALIMIT_LOW); Serial.print("nm. Corrected to this value. Requested was "); Serial.println(m_lambdaRequested);
										}
										else if (m_lambdaCorrected<m_lambdaRequested){
											Serial.print(">> Lambda can't be higher then "); Serial.print(SPECTRALIMIT_HIGH); Serial.print("nm. Corrected to this value. Requested was "); Serial.println(m_lambdaRequested); 
										}
										else{
											Serial.println(">> - Lambda Acceptded!!");
										}
									#endif
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Set Replicates");
									m_lcd.setCursor(0, 1); m_lcd.print("n: ");
									#ifdef DEBUG_SERIAL
										Serial.print(">> Replicates requested:\t\t");
									#endif
									m_keyPadString="";
									while(!m_okVal){
										m_okVal=digitalRead(BUTTON_PIN_OK);
										m_customKey=0;
										m_customKey = m_customKeypad.getKey();
										if(m_customKey){
											m_keyPadString+=m_customKey;
											m_lcd.setCursor(0, 1); m_lcd.print("n: " + m_keyPadString);
											#ifdef DEBUG_SERIAL
												Serial.print(m_customKey);
											#endif
										}
									}
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									#ifdef DEBUG_SERIAL
										Serial.println();
									#endif
									m_nReadsRequested=m_keyPadString.toInt();
									// Per avere una media decente, o un valore "affidabile" si considera una media di almeno MIN_REPLICATES
									if(m_nReadsRequested<MIN_REPLICATES){
										m_nReadsCorrected=MIN_REPLICATES;
										#ifdef DEBUG_SERIAL
											Serial.print(">> Replicates can't be lower then "); Serial.print(MIN_REPLICATES); Serial.print(". Corrected to that value. Requested was "); Serial.println(m_nReadsRequested);
										#endif
									}
									else{
										m_nReadsCorrected=m_nReadsRequested;
										#ifdef DEBUG_SERIAL
											Serial.println(">> - Replicates Acceptded!!");
										#endif
									}
									#ifdef DEBUG_SERIAL
										Serial.println(">> Positioning motor...");
									#endif
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Replicates: " + String(m_nReadsCorrected));
									m_lcd.setCursor(0, 1); m_lcd.print("Pos.ing motor...");
									m_gratingMotorFutureSteps=map(m_lambdaCorrected, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_LOW);
									gratingMotorZeroPoint(MOTOR_PIN_POSITIONSENSOR, BUZZER_PIN, m_lcd, m_grtMotor);			//serve davvero?
									m_grtMotor->step(m_gratingMotorFutureSteps, FORWARD, MOTOR_STEP_TYPE);
									m_grtMotor->release();
									#ifdef DEBUG_SERIAL
										Serial.println(">> Positioning motor done! Waiting confirm to proceed...");
									#endif
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Press 'OK' to");
									m_lcd.setCursor(0, 1); m_lcd.print("AutoZero");
									//rivedere
									while(!m_okVal){
										m_okVal=digitalRead(BUTTON_PIN_OK);
									}
									m_okVal=LOW;
									#ifdef DEBUG_SERIAL
										Serial.println(">> Reading blank...");
									#endif
									m_nReadsCaptured=0;
									m_sumBackgroundReadsVal=0;
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Reading...");
									m_lcd.setCursor(0, 1); m_lcd.print("Index: ");
									for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
										m_lcd.setCursor(7, 1); m_lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
										m_readsVal=simpleRead(&m_tsl, TSL_READTYPE_VISIBLE);
										m_sumBackgroundReadsVal+=m_readsVal;
										#ifdef DEBUG_SERIAL
											Serial.print(">> "); Serial.print(m_readsVal);
										#endif
									}
									#ifdef DEBUG_SERIAL
										Serial.println();
									#endif
									m_nReadsCaptured=0;
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Load Sample!");
									m_lcd.setCursor(0, 1); m_lcd.print("'OK' to read...");
									#ifdef DEBUG_SERIAL
										Serial.println(">> Blank reads done! Waiting confirm to proceed to Sample...");
									#endif
									delay(200);
									while(!m_okVal && !m_backVal){
										m_okVal=digitalRead(BUTTON_PIN_OK);
										m_backVal=digitalRead(BUTTON_PIN_BACK);
										if(m_okVal){
											m_okVal=LOW;
											#ifdef DEBUG_SERIAL
												Serial.println(">> Reading Sample...");
											#endif
											m_lcd.clear();
											m_lcd.setCursor(0, 0); m_lcd.print("Reading...");
											m_lcd.setCursor(0, 1); m_lcd.print("Index: ");
											m_nReadsCaptured=0;
											m_sumSampleReadsVal=0;
											for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
												m_lcd.setCursor(7, 1); m_lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
												m_readsVal=simpleRead(&m_tsl, TSL_READTYPE_VISIBLE);
												m_sumSampleReadsVal+=m_readsVal;
												#ifdef DEBUG_SERIAL
													Serial.print(">> "); Serial.print(m_readsVal);
												#endif
											}
											#ifdef DEBUG_SERIAL
												Serial.println();
											#endif
											m_lcd.clear();
											m_lcd.setCursor(0, 0); m_lcd.print("--LOADING DATA-");
											m_lcd.setCursor(0, 1); m_lcd.print("...PLEAS WAIT...");
											#ifdef DEBUG_SERIAL
												Serial.println(">> Sample reads done! Calculating results...");
											#endif
											/* ----------------- Per i prossimi due calcoli è importante ricorda che stiamo lavorando cone delle medie ----------------- */
											// Si definisce Trasmittanza (T) il rapporto tra l'intensità della luce I in uscita dalla cuvetta e l'intensità I_0 delle luce incidente.
											// 		T=I/I_0
											// Non ha unità di misura e può assumere qualsiasi valore >=0.
											float trasmittance	=	(m_sumSampleReadsVal/(float)m_nReadsCorrected)/(m_sumBackgroundReadsVal/(float)m_nReadsCorrected);
											// Si definisce Assorbanza A come il prodotto della concentrazione C (in moli/litro) della sostanza che assorbe la radiazione, per il cammino ottico b percorso dalla luce, per il coefficiente di assorbimento ε.
											//		A=ε*b*C
											// Inoltre sappiamo che esiste una correlazione tra assorbanza e trasmittanza di natura logaritmica:
											//		A=log10(1/T)=log10(I_0/I)
											float absorbance	=	log10(1/((((m_sumSampleReadsVal/(float)m_nReadsCorrected)/(m_sumBackgroundReadsVal/(float)m_nReadsCorrected))))); //verificare il 1000
											m_lcd.clear();
											m_lcd.setCursor(0, 0); m_lcd.print("Abs Sample:");
											m_lcd.setCursor(0, 1); m_lcd.print(String(absorbance,DECIMAL_LCD_ABSORBANCE));
											#ifdef DEBUG_SERIAL
												Serial.print(">> [---------------------------------------------------]\n");
												Serial.print(">> [ Number of reads:\t\t");	Serial.println(m_nReadsCorrected);
												Serial.print(">> [ Background Average:\t");	Serial.println((m_sumBackgroundReadsVal)/(m_nReadsCorrected));
												Serial.print(">> [ Sample Average:\t\t");	Serial.println((m_sumSampleReadsVal)/(m_nReadsCorrected));
												Serial.print(">> [ Trasmittance:\t\t");		Serial.print(trasmittance, DECIMAL_SERIAL_TRASMITTANCE);	trasmittance>1 	? Serial.println(" - (>1) ?(0_o)?") : Serial.println();
												Serial.print(">> [ Absorbance:\t\t");		Serial.print(absorbance, DECIMAL_SERIAL_ABSORBANCE);		absorbance<0 	? Serial.println(" - (>1) ?(0_o)?") : Serial.println();
												Serial.print(">> [---------------------------------------------------]\n");
											#endif
										}
									}
									m_backVal=LOW;
								}
								else if(m_backVal){
									#ifdef DEBUG_SERIAL
										Serial.println("Return Pressed.");
									#endif
								}
							}
							// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
							waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
							m_refreshScreen=true;
							break;
						}
						// In questa modalità è possibile eseguire una sorta di "Scansione spettrale" del composto, utile per determinare la lambdaMax da utilizzare in fase di analisi di concentrazione.
						case(ANALYSISMODE_ALLSPECTRUM):{
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("All Spectrum");
							m_lcd.setCursor(0, 1); m_lcd.print("selected?");
							while(!m_okVal && !m_backVal){
								m_okVal=digitalRead(BUTTON_PIN_OK);
								m_backVal=digitalRead(BUTTON_PIN_BACK);
								if(m_okVal){
									#ifdef DEBUG_SERIAL
										Serial.println("All Spectrum Selected!");
									#endif
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Sel. from/to nm");
									m_lcd.setCursor(0, 1); m_lcd.print("[");m_lcd.print(SPECTRALIMIT_LOW, DEC); m_lcd.print("-"); m_lcd.print(SPECTRALIMIT_HIGH, DEC); m_lcd.print("],OK?");
									waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Spectrum range");
									m_lcd.setCursor(0, 1); m_lcd.print("Set MIN:  ");
									#ifdef DEBUG_SERIAL
										Serial.print(">> Lower Lambda limit requested(nm):\t");
									#endif
									m_keyPadString="";
									while(!digitalRead(BUTTON_PIN_OK)){
										m_customKey=0;
										m_customKey = m_customKeypad.getKey();
										if(m_customKey){
											m_keyPadString=m_keyPadString+m_customKey;
											m_lcd.setCursor(0, 1); m_lcd.print("Set MIN:  " + m_keyPadString);
											#ifdef DEBUG_SERIAL
												Serial.print(m_customKey);
											#endif
										}
									}
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									#ifdef DEBUG_SERIAL
										Serial.println();
									#endif
									m_lambdaRequested=m_keyPadString.toInt();
									delay(200);
									m_lambdaMin=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
									// //	>> ======================= TODO ======================= <<
									// if(m_lambdaRequested<SPECTRALIMIT_LOW){
										// m_lambdaMin=SPECTRALIMIT_LOW;
										// #ifdef DEBUG_SERIAL
											// Serial.print("I mean... I ask you ");
										// #endif
										
									// }
									// else if(m_lambdaMin>=SPECTRALIMIT_HIGH){
										// #ifdef DEBUG_SERIAL
											// Serial.print("I mean... I ask you the lower limit... Retry");
										// #endif
									// }
									// else{
										// m_lambdaMin=m_lambdaRequested;
									// }
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Spectrum range");
									m_lcd.setCursor(0, 1); m_lcd.print("Set MAX:  ");
									#ifdef DEBUG_SERIAL
										Serial.print(">> Higher Lambda limit requested(nm):\t");
									#endif
									m_keyPadString="";
									while(!digitalRead(BUTTON_PIN_OK)){
										m_customKey=0;
										m_customKey = m_customKeypad.getKey();
										if(m_customKey){
											m_keyPadString=m_keyPadString+m_customKey;
											m_lcd.setCursor(0, 1); m_lcd.print("Set MAX:  " + m_keyPadString);
											#ifdef DEBUG_SERIAL
												Serial.print(m_customKey);
											#endif
										}
									}
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									#ifdef DEBUG_SERIAL
										Serial.println();
									#endif
									m_lambdaRequested=m_keyPadString.toInt();
									m_lambdaMax=constrain(m_lambdaRequested, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH);
									// TODO -> Gestire caso in cui m_lambdaMax < m_lambdaMin
									// TODO -> Cambiare i nomi di queste due variabili, lambdaMax deve essere un'altra cosa.
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("MIN:  "); m_lcd.print(m_lambdaMin);
									m_lcd.setCursor(0, 1); m_lcd.print("MAX:  "); m_lcd.print(m_lambdaMax);
									delay(1500);
									// waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									#ifdef DEBUG_SERIAL
										Serial.print(">> MIN:\t"); Serial.println(m_lambdaMin);
										Serial.print(">> MAX:\t"); Serial.println(m_lambdaMax);
									#endif
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Set Replicates");
									m_lcd.setCursor(0, 1); m_lcd.print("n:  ");
									m_keyPadString="";
									while(!digitalRead(BUTTON_PIN_OK)){
										m_customKey=0;
										m_customKey = m_customKeypad.getKey();
										if(m_customKey){
											m_keyPadString=m_keyPadString+m_customKey;
											m_lcd.setCursor(0, 1); m_lcd.print("n:  " + m_keyPadString);
										}
									}
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									delay(1000);
									m_nReadsCorrected=m_keyPadString.toInt();
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Replicates:  ");
									m_lcd.setCursor(0, 1); m_lcd.print(m_nReadsCorrected);
									waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Grating Motor");
									m_lcd.setCursor(0, 1); m_lcd.print("Zero setting...");
									// Commentare la riga successiva per prove più rapide.
									// RICORDARSI però di scommentarla per il firmware funzionante.
									gratingMotorZeroPoint(MOTOR_PIN_POSITIONSENSOR, BUZZER_PIN, m_lcd, m_grtMotor);
									m_gratingMotorFutureSteps=map(m_lambdaMin, SPECTRALIMIT_LOW, SPECTRALIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_HIGH, MOTOR_STEPS_GRATINGLIMIT_LOW);
									m_grtMotor->step(m_gratingMotorFutureSteps, FORWARD, MOTOR_STEP_TYPE);
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Press 'OK' to");
									m_lcd.setCursor(0, 1); m_lcd.print("Scan Blank");
									waitingButtonPressedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonReleasedFiltered(BUTTON_PIN_OK, &m_okVal);
									// waitingButtonPressed(BUTTON_PIN_OK, &m_okVal);
									waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
									m_now = m_rtc.now();
									#ifdef DEBUG_SERIAL
										Serial.println(">> Reading Blank...");
										Serial.print(">> Number of steps to do:\t\t"); Serial.println((m_lambdaMax-m_lambdaMin)/m_deltaLambdaPerSingleStep);
									#endif
									for(m_allSpectrumScanReadIndex;m_allSpectrumScanReadIndex<=(m_lambdaMax-m_lambdaMin)/m_deltaLambdaPerSingleStep;m_allSpectrumScanReadIndex++){
										#ifdef DEBUG_SERIAL
											Serial.print(">> ;");
											Serial.print(m_allSpectrumScanReadIndex);
											Serial.print(";");
											serialDebugString=String((float)m_lambdaMin+m_deltaLambdaPerSingleStep*(float)m_allSpectrumScanReadIndex);
											serialDebugString.replace(".",",");
											Serial.print(serialDebugString);
											Serial.print(";");
										#endif
										m_lcd.clear();
										m_lcd.setCursor(0, 0); m_lcd.print("Nm: "); m_lcd.print(String((float)m_lambdaMin+m_deltaLambdaPerSingleStep*(float)m_allSpectrumScanReadIndex) + "/" + String(m_lambdaMax));
										m_lcd.setCursor(0, 1); m_lcd.print("Index: ");
										m_nReadsCaptured=0;
										m_sumBackgroundReadsVal=0;
										for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
											m_lcd.setCursor(7, 1); m_lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
											m_readsVal=simpleRead(&m_tsl, TSL_READTYPE_VISIBLE);
											m_sumBackgroundReadsVal+=m_readsVal;
											#ifdef DEBUG_SERIAL
												Serial.print(m_readsVal); Serial.print(";");
											#endif
										}
										#ifdef DEBUG_SERIAL
											serialDebugString=String(m_sumBackgroundReadsVal/m_nReadsCorrected);
											serialDebugString.replace(".",",");
											Serial.println(serialDebugString);
										#endif
										m_grtMotor->step(1, BACKWARD, MOTOR_STEP_TYPE);
									}
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Blank Scan End");
									m_lcd.setCursor(0, 1); m_lcd.print("Resetting Motor.");
									m_grtMotor->step(m_allSpectrumScanReadIndex, FORWARD, MOTOR_STEP_TYPE);
									m_grtMotor->release();
									m_lcd.clear();
									m_lcd.setCursor(0, 0); m_lcd.print("Load Sample!");
									m_lcd.setCursor(0, 1); m_lcd.print("'OK' to read...");
									// m_allSpectrumScanID=0;
									// delay(1000);
									while(!m_backVal){
										// m_okVal=digitalRead(BUTTON_PIN_OK);
										m_backVal=digitalRead(BUTTON_PIN_BACK);
										if(digitalRead(BUTTON_PIN_OK)){
											waitingButtonReleased(BUTTON_PIN_OK, &m_okVal);
											#ifdef DEBUG_SERIAL
												Serial.print(">> Reading Sample...");
											#endif
											m_allSpectrumScanReadIndex=0;
											for(m_allSpectrumScanReadIndex;m_allSpectrumScanReadIndex<=(m_lambdaMax-m_lambdaMin)/m_deltaLambdaPerSingleStep;m_allSpectrumScanReadIndex++){
												#ifdef DEBUG_SERIAL
													Serial.print(">> ;");
													Serial.print(m_allSpectrumScanReadIndex);
													Serial.print(";");
													serialDebugString=String((float)m_lambdaMin+m_deltaLambdaPerSingleStep*(float)m_allSpectrumScanReadIndex);
													serialDebugString.replace(".",",");
													Serial.print(serialDebugString);
													Serial.print(";");
												#endif
												m_lcd.clear();
												m_lcd.setCursor(0, 0); m_lcd.print("Nm: "); m_lcd.print(String((float)m_lambdaMin+m_deltaLambdaPerSingleStep*(float)m_allSpectrumScanReadIndex) + "/" + String(m_lambdaMax));
												m_lcd.setCursor(0, 1); m_lcd.print("Index: ");
												m_nReadsCaptured=0;
												m_sumBackgroundReadsVal=0;
												for(m_nReadsCaptured; m_nReadsCaptured<m_nReadsCorrected; m_nReadsCaptured++){
													m_lcd.setCursor(7, 1); m_lcd.print(String(m_nReadsCaptured) + "/" + String(m_nReadsCorrected));
													m_readsVal=simpleRead(&m_tsl, TSL_READTYPE_VISIBLE);
													m_sumBackgroundReadsVal+=m_readsVal;
													#ifdef DEBUG_SERIAL
														Serial.print(m_readsVal); Serial.print(";");
													#endif
												}
												#ifdef DEBUG_SERIAL
													serialDebugString=String(m_sumBackgroundReadsVal/m_nReadsCorrected);
													serialDebugString.replace(".",",");
													Serial.println(serialDebugString);
												#endif
												m_grtMotor->step(1, BACKWARD, MOTOR_STEP_TYPE);
											}
											m_grtMotor->release();
											m_lcd.clear();
											m_lcd.setCursor(0, 0); m_lcd.print("Scan Ended!");
											m_lcd.setCursor(0, 1); m_lcd.print("Again or abort?");
											#ifdef DEBUG_SERIAL
												Serial.println(">> Scan Finished. Waiting to repeat or abort...");
											#endif
										}
									}
									// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
									waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
								}
								else if(m_backVal){
									#ifdef DEBUG_SERIAL
										Serial.println("Return Pressed.");
									#endif
								}
							}
							// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
							waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
							m_refreshScreen=true;
							break;				
						}
						// Questa è la modalità più complessa.
						// Qui è possiible istruire lo strumento a calcore una determinata concentrazione dopo essersi costruiti una retta di regressione lineare.
						// Si parte facendo leggere allo strumento campioni a concentrazione nota associandogli una certa assorbanza.
						// Si prosegue calcolando la linearità tramite il metodo dei minimi quadrati.
						// Si termina sottoponendo ad esame i campioni di cui voglia sapere la concentrazione.
						case ANALYSISMODE_CONCANALYSIS:{
							m_lcd.clear();
							m_lcd.setCursor(0, 0); m_lcd.print("Conc. Analysis");
							m_lcd.setCursor(0, 1); m_lcd.print("selected?");
							while((m_okVal==LOW) && (m_backVal==LOW)){
								m_okVal=digitalRead(BUTTON_PIN_OK);
								m_backVal=digitalRead(BUTTON_PIN_BACK);
								if(m_okVal){
									m_okVal=LOW;
									//	>> ======================= TODO ======================= << 
								}
								else if(m_backVal){
									#ifdef DEBUG_SERIAL
										Serial.println("Return Pressed.");
									#endif
								}
							}
							// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
							waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
							m_refreshScreen=true;
							break;
						}
					}
				}
				// waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
				#ifdef DEBUG_SERIAL
					Serial.println(">> Returning to the selection mode menu");
				#endif
			}
			// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
			waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
			break;
		}
		case(MENU_SETTINGS):{
			//	>> ======================= TODO ======================= << 
			m_lcd.clear();
			m_lcd.setCursor(0, 0); m_lcd.print("Settings Menu");
			m_lcd.setCursor(0, 1); m_lcd.print("Work in Progress");
			while(!digitalRead(BUTTON_PIN_BACK)){
				;
			}
			waitingButtonPressedFiltered(BUTTON_PIN_BACK, &m_backVal);
			// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
			// waitingButtonPressed(BUTTON_PIN_BACK, &m_backVal);
			waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
			break;
		}
		case(MENU_VOICE_3):{
			//	>> ======================= TODO ======================= << 
			m_lcd.clear();
			m_lcd.setCursor(0, 0); m_lcd.print("!!BIT AT WORK!!3");
			m_lcd.setCursor(0, 1); m_lcd.print("!DON'T DISTURB!3");
			while(!digitalRead(BUTTON_PIN_BACK)){
				;
			}
			waitingButtonPressedFiltered(BUTTON_PIN_BACK, &m_backVal);
			// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
			// waitingButtonPressed(BUTTON_PIN_BACK, &m_backVal);
			waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
			break;
		}
		case(MENU_VOICE_4):{
			//	>> ======================= TODO ======================= << 
			m_lcd.clear();
			m_lcd.setCursor(0, 0); m_lcd.print("!!BIT AT WORK!!4");
			m_lcd.setCursor(0, 1); m_lcd.print("!DON'T DISTURB!4");
			while(!digitalRead(BUTTON_PIN_BACK)){
				;
			}
			waitingButtonPressedFiltered(BUTTON_PIN_BACK, &m_backVal);
			// waitingButtonReleasedFiltered(BUTTON_PIN_BACK, &m_backVal);
			// waitingButtonPressed(BUTTON_PIN_BACK, &m_backVal);
			waitingButtonReleased(BUTTON_PIN_BACK, &m_backVal);
			break;
		}
	}
}


// =========================================================================================
// ======================================= vecchi appunti ========================================
// =========================================================================================

	//float iSource=m_sumBackgroundReadsVal/(float)m_nReadsCorrected;
	//float iSample=((m_sumSampleReadsVal/(float)m_nReadsCorrected));
	//float trasmittance=iSample/iSource;
	//float Abs=log10(1/(iSample/iSource))*1000;
		//Serial.print(">> [ iSource:\t\t\t");		Serial.println(iSource);
		//Serial.print(">> [ iSample:\t\t\t");		Serial.println(iSample);
		//Serial.print(">> [ Abs:\t\t\t");			Serial.print(Abs); 										Serial.println("*10^-3.");

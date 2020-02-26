// FILE PER GESTIRE TUTTE LE STRINGHE (dove possibiel e se ha senso farlo)

#ifndef MSG_h
#define MSG_h

// Informazioni interne della macchina
#define MODEL_VERSION						"1.0"
#define FIRMWARE_VERSION					"15.0"
#define BUILD_TIMESTAP_DATE					F(__DATE__)
#define BUILD_TIMESTAP_TIME					F(__TIME__)
#define INSTRUMENT_NAME						"Spe.Ar"
#define AUTOR_NAME							"Massimo Frassetto"
#define AUTOR_EMAIL							"frassetto.massimoo@gmail.com"

// Stringhe per l'LCD per le tipologie di analisi nel menu di selzione della modalità
#define ANALYSISMODE_LCDSTRING_MOD_1 		"Simple Read"
#define ANALYSISMODE_LCDSTRING_MOD_2 		"All Spectrum"
#define ANALYSISMODE_LCDSTRING_MOD_3 		"Conc.Analysis"

// Nome dei file in cui vado a scrivere i miei dati a seconda della situazione
// IMPORTANTE -> Il numero dei caratteri (estensione esclusa) deve essere al più 8.
// #define SIMPLE_FILENAME						"simpRead.txt"
#define ALLSPECTRUM_FILENAME				"allSpec.txt"
#define CONCANLYSIS_FILENAME				"concent.txt"
#define SPEARTRACELOG_FILENAME				"traceLog.txt"

// Queste sono le stringhe che verranno visualizzate sull'LCD nel menu principale
// Ricordarsi di inserire il nome del menu corretto quando si inserirà un nuovo menu
#define MENU_LCDSTRING_ANALYSIS				"StartAnalysis"
#define MENU_LCDSTRING_SETTINGS				"Settings     "				
#define MENU_LCDSTRING_VOICE_3				"Voice_3      "				// voce 3 per test
#define MENU_LCDSTRING_VOICE_4				"Voice_4      "				// voce 4 per test


#endif
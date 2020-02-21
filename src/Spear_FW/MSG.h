// FILE PER GESTIRE TUTTE LE STRINGHE (dove possibiel e se ha senso farlo)

#ifndef MSG_h
#define MSG_h

// Informazioni interne della macchina
#define MODEL_VERSION						"1.0"
#define FIRMWARE_VERSION					"14.1"
#define BUILD_TIMESTAP_DATE					F(__DATE__)
#define BUILD_TIMESTAP_TIME					F(__TIME__)
#define INSTRUMENT_NAME						"Spe.Ar"
#define AUTOR_NAME							"Massimo Frassetto"
#define AUTOR_EMAIL							"frassetto.massimoo@gmail.com"

#define ANALYSISMODE_LCDSTRING_MOD_1 		"Simple Read"
#define ANALYSISMODE_LCDSTRING_MOD_2 		"All Spectrum"
#define ANALYSISMODE_LCDSTRING_MOD_3 		"Conc.Analysis"

// #define SIMPLE_FILENAME						"simpRead.txt"
#define ALLSPECTRUM_FILENAME				"allSpect.txt"
#define CONCANLYSIS_FILENAME				"concentr.txt"

#define SPEARTRACELOG_FILENAME				"traceLog.txt"

#endif
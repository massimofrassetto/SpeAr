#ifndef SPEAR_h
#define SPEAR_h

#include "Arduino.h"

class SPEAR{
	public:
		void configureSensor(void);
		void tslSensorChecking (void);
		void gratingMotorZeroPoint (const int sensMotPin, const int piezoZeroPin);
		void gratingMotorChecking (const int sensMotCheckPin, const int piezoCheckPin);
		void SDCardChecking (const int chipSel);
		void printWiringError (const int allarmPin);
		void lampChecking (const int lampSensPin, const int lampStatePin, int* lampState, const int piezoLCPin);
};

#endif
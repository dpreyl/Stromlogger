/*
 * DCF77TimeLib.cpp
 *
 *  Created on: 24.06.2023
 *      Author: D-Laptop
 */
#ifdef DEVICE_TYPE_HANDHELD
#include "DCF77TimeLib.h"

DCF77TimeLib::DCF77TimeLib(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin, uint8_t ssPin)
    : _softSPI(mosiPin, misoPin, sckPin), _ssPin(ssPin),
	  _rtcDCF(_softSPI, ssPin){}

void DCF77TimeLib::begin() {
  _softSPI.begin();
  pinMode(_ssPin, OUTPUT);
  digitalWrite(_ssPin, HIGH);
  _rtcDCF.begin();
  enableReceiver();
}

void DCF77TimeLib::enableReceiver() {
  _rtcDCF.enableDCF77Reception();
  _rtcDCF.enableDCF77Interrupt();

    /* den periodischen Interrupt auf 1 Hz einstellen */
  _rtcDCF.setPeriodicInterruptMode(RTC_PERIODIC_INT_PULSE_1_HZ);

    /* den periodischen Interrupt des RTC-DCF aktivieren */
  _rtcDCF.enablePeriodicInterrupt();

  _rtcDCF.enableDCF77LED();
}

bool DCF77TimeLib::hasDCFTime() {
  _rtcDCF.getDateTime(&_readDateTime);
  if(_readDateTime.getYear() > 20){
	  return true;
  }
  return false;
//  char dateTimeString[19];
//  _readDateTime.getDateTimeString(dateTimeString);
//  Serial.printf("Empfangene Zeit: %s", dateTimeString);
//  _rtcDCF.resetDCF77Interrupt();

}

uint32_t DCF77TimeLib::getUnixTimestamp() {
	_rtcDCF.resetDCF77Interrupt();
	if(hasDCFTime()){
		setTime(_readDateTime.getHour(),_readDateTime.getMinute(),_readDateTime.getSecond(),_readDateTime.getDay(),_readDateTime.getMonth(),_readDateTime.getYear());
	  return now();
  }
  else {
	  return 0;
  }
}

void DCF77TimeLib::printTime(){
	_rtcDCF.getDateTime(&_readDateTime);
	char dateTimeString[19];
	dateTimeString[18] = '\0';
	_readDateTime.getDateTimeString(dateTimeString);
	Serial.printf("Aktuelle Zeit: %s", dateTimeString);
}

#endif

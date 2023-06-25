/*
 * DCF77TimeLib.h
 *
 *  Created on: 24.06.2023
 *      Author: D-Laptop
 */

#ifndef DCF77TimeLib_h
#define DCF77TimeLib_h


#ifdef DEVICE_TYPE_HANDHELD

#include "Arduino.h"
#include <SoftSPI.h>
#include <RealTimeClock_DCF.h>
#include <TimeLib.h>

// Replace these pin numbers with the appropriate pins connected to the DCF77 receiver
const uint8_t SCK_PIN = 26;
const uint8_t MISO_PIN = 12;
const uint8_t MOSI_PIN = 27;
const uint8_t SS_PIN = 25;

class DCF77TimeLib {
public:
  DCF77TimeLib(uint8_t sckPin=SCK_PIN, uint8_t misoPin=MISO_PIN, uint8_t mosiPin=MOSI_PIN, uint8_t ssPin=SS_PIN);
  void begin();
  uint32_t getUnixTimestamp();

  bool hasDCFTime();

  void printTime();
private:
  SoftSPI _softSPI;
  uint8_t _ssPin;
  void enableReceiver();

  RealTimeClock_DCF _rtcDCF;
  DateTime _readDateTime;
};

#endif
#endif

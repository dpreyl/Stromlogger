#ifdef DEVICE_TYPE_HANDHELD
#include "RealTimeClock_DCF.h"

/**********************************************************
Konstruktor
**********************************************************/

RealTimeClock_DCF::RealTimeClock_DCF(SoftSPI softSPI, uint8_t ssPin) : _softSPI(softSPI), _ssPin(ssPin)
{}


/**********************************************************
�ffentliche Methoden
**********************************************************/

/*---------------------------------------------------------
Name:           begin

Beschreibung:   initialisiert den gew�hlten Kommunikations-
                weg
 
Eing�nge:       RTCCommunication_t rtcCommunication
                  Kommunikationsweg zum RTC-DCF

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::begin(void)
{
      _softSPI.begin();
      _softSPI.setBitOrder(MSBFIRST);
      _softSPI.setClockDivider(SPI_CLOCK_DIV32);
      _softSPI.setDataMode(SPI_MODE1);
}

/*---------------------------------------------------------
Name:           setTime

Beschreibung:   setzt die Uhrzeit des RTC-DCF
 
Eing�nge:       DateTime *dateTime
                  Struktur mit Datum und Uhrzeit, die im
                  RTC-DCF gesetzt werden soll. 

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::setDateTime(DateTime *dateTime)
{
	uint8_t dateTimeBCD[7];

	dateTimeBCD[0] = decimalToBCD(dateTime->getSecond());
	dateTimeBCD[1] = decimalToBCD(dateTime->getMinute());
	dateTimeBCD[2] = decimalToBCD(dateTime->getHour());
	dateTimeBCD[3] = decimalToBCD(dateTime->getWeekday() + 1);
	dateTimeBCD[4] = decimalToBCD(dateTime->getDay());
	dateTimeBCD[5] = decimalToBCD(dateTime->getMonth());
	dateTimeBCD[6] = decimalToBCD(dateTime->getYear());
	
	writeRegister(RTC_DCF_REG_SECOND, dateTimeBCD, 7);
}

/*---------------------------------------------------------
Name:           getTime

Beschreibung:   liest die Uhrzeit des RTC-DCF
 
Eing�nge:       DateTime *dateTime
                  Struktur mit Datum und Uhrzeit, in die Daten 
                  aus dem RTC-DCF gespeichert werden

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::getDateTime(DateTime *dateTime)
{
	uint8_t dateTimeBCD[7];

  readRegister(RTC_DCF_REG_SECOND, 7, dateTimeBCD);
	
	dateTime->setSecond(BCDToDecimal(dateTimeBCD[0]));
	dateTime->setMinute(BCDToDecimal(dateTimeBCD[1]));
	dateTime->setHour(BCDToDecimal(dateTimeBCD[2]));
	dateTime->setWeekday(BCDToDecimal(dateTimeBCD[3]) - 1);
	dateTime->setDay(BCDToDecimal(dateTimeBCD[4]));
	dateTime->setMonth(BCDToDecimal(dateTimeBCD[5]));
	dateTime->setYear(BCDToDecimal(dateTimeBCD[6]));
}
/*---------------------------------------------------------
Name:           setAlarmTime

Beschreibung:   setzt einen der beiden Alarmuhrzeiten im RTC-Chip
 
Eing�nge:       RTCAlarmWeekdays_t weekdays
                  Wochentage f�r den der Alarm gelten soll
                  m�gliche Werte: 
                  RTC_ALARM_WEEKDAYS_NONE,
                  RTC_ALARM_WEEKDAYS_MON,
                  RTC_ALARM_WEEKDAYS_TUE,
                  RTC_ALARM_WEEKDAYS_WED,
                  RTC_ALARM_WEEKDAYS_THU,
                  RTC_ALARM_WEEKDAYS_FRI,
                  RTC_ALARM_WEEKDAYS_SAT,
                  RTC_ALARM_WEEKDAYS_SUN,
                  RTC_ALARM_WEEKDAYS_MON_TO_FRI,
                  RTC_ALARM_WEEKDAYS_SAT_TO_SUN,
                  RTC_ALARM_WEEKDAYS_ALL

                  die Wochentage sind �ber das bitweise Oder (|)
                  verkn�pfbar.

                  Beispiel: F�r einen Alarm an den Wochentagen
                  Montag und Donnerstag wird der Funktion folgendes
                  �bergeben:
                  (RTC_ALARM_WEEKDAYS_MON | RTC_ALARM_WEEKDAYS_THU)

                uint8_t hour
                  Stunde zu der der Alarm ausl�sen soll

                uint8_t minute
                  Minute zu der der Alarm ausl�sen soll

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::setAlarmTime(RTCAlarmWeekdays_t weekdays, uint8_t hour, uint8_t minute)
{
	uint8_t alarmTimeBCD[] = {	decimalToBCD(minute),
										          decimalToBCD(hour),
										          (uint8_t)weekdays };

	writeRegister(RTC_DCF_REG_ALARM_MINUTE, alarmTimeBCD, 3);
}

/*---------------------------------------------------------
Name:           getAlarmTime

Beschreibung:   liest einen der beiden Alarmuhrzeiten aus dem RTC-Chip
 
Eing�nge:       RTCAlarmWeekdays_t *weekdays
                  ausgelesener Wochentag an dem Alarm aktiv ist
                  (Bitkodierung siehe setAlarmTime)

                uint8_t *hour
                  ausgelesene Stunde des Alarms

                uint8_t *minute
                  ausgelesene Minute des Alarms

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::getAlarmTime(RTCAlarmWeekdays_t *weekdays, uint8_t *hour, uint8_t *minute)
{
	uint8_t alarmTimeBCD[3];
	
	readRegister(RTC_DCF_REG_ALARM_MINUTE, 7, alarmTimeBCD);
	
  *minute   = BCDToDecimal(alarmTimeBCD[0]);
  *hour     = BCDToDecimal(alarmTimeBCD[1]);
  *weekdays = (RTCAlarmWeekdays_t)alarmTimeBCD[2];
}

/*---------------------------------------------------------
Name:           enableAlarmInterrupt

Beschreibung:   aktiviert den eingestellten Alarm
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enableAlarmInterrupt(void)
{
	uint8_t alarmRegister;

  readRegister(RTC_DCF_REG_ALARM_CONFIG, 1, &alarmRegister);

  alarmRegister |= RTC_ALARM_CONFIG_REGISTER_AIE;

	writeRegister(RTC_DCF_REG_ALARM_CONFIG, &alarmRegister, 1);
}

/*---------------------------------------------------------
Name:           disableAlarmInterrupt

Beschreibung:   deaktiviert den eingestellten Alarm
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disableAlarmInterrupt(void)
{
	uint8_t alarmRegister;

  readRegister(RTC_DCF_REG_ALARM_CONFIG, 1, &alarmRegister);

  alarmRegister &= ~RTC_ALARM_CONFIG_REGISTER_AIE;

	writeRegister(RTC_DCF_REG_ALARM_CONFIG, &alarmRegister, 1);
}

/*---------------------------------------------------------
Name:           resetAlarmInterrupt

Beschreibung:   schaltet den Alarm wieder scharf
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::resetAlarmInterrupt(void)
{
  uint8_t statusRegister = RTC_STATUS_AIF;  
  
  writeRegister(RTC_DCF_REG_STATUS, &statusRegister, 1);
}

/*---------------------------------------------------------
Name:           enableAlarmLED

Beschreibung:   aktiviert, dass die LED bei einem Alarm ein- bzw.
                nach ausschalten des Alarms wieder ausgeschaltet
                wird.
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enableAlarmLED(void)
{
	uint8_t alarmRegister;

  readRegister(RTC_DCF_REG_ALARM_CONFIG, 1, &alarmRegister);

  alarmRegister |= RTC_ALARM_CONFIG_REGISTER_AILED;

	writeRegister(RTC_DCF_REG_ALARM_CONFIG, &alarmRegister, 1);
}

/*---------------------------------------------------------
Name:           disableAlarmLED

Beschreibung:   deaktiviert, dass die LED bei einem Alarm ein- bzw.
                nach ausschalten des Alarms wieder ausgeschaltet
                wird.
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disableAlarmLED(void)
{
	uint8_t alarmRegister;

  readRegister(RTC_DCF_REG_ALARM_CONFIG, 1, &alarmRegister);

  alarmRegister &= ~RTC_ALARM_CONFIG_REGISTER_AILED;

	writeRegister(RTC_DCF_REG_ALARM_CONFIG, &alarmRegister, 1);
}

/*---------------------------------------------------------
Name:           setPeriodicInterruptMode

Beschreibung:   stellt den periodischen Interruptbetrieb ein
                 
Eing�nge:       PeriodicInterruptSelection_t periodicInterruptSelection
                  Art des periodischen Betriebs:

                  RTC_PERIODIC_INT_PULSE_1_HZ
                    Pulsmodus mit 1 Hz

                  RTC_PERIODIC_INT_PULSE_2_HZ
                    Pulsmodus mit 2 Hz

                  RTC_PERIODIC_INT_LEVEL_EVERY_SECOND
                    Interrupt jede Sekunde (synchron mit Sekundenz�hler)
                  
                  RTC_PERIODIC_INT_LEVEL_EVERY_MINUTE
                    Interrupt jede Minute (bei Sekunde = 0)

                  RTC_PERIODIC_INT_LEVEL_EVERY_HOUR
                    Interrupt jede Stunde (bei Minute und Sekunde = 0)

                  RTC_PERIODIC_INT_LEVEL_EVERY_DAY
                    Interrupt jeden Tag (bei Stunde, Minute und Sekunde = 0)

                  RTC_PERIODIC_INT_LEVEL_EVERY_MONTH
                    Interrupt jeden Monat (jeweils am 1. um 0:00:00)

                  Hinweis:
                  Die Interrupts mit der Bezeichnung LEVEL werden
                  nicht automatisch zur�ckgesetzt, sondern m�ssen durch
                  den Aufruf der Methode resetPeriodicInterrupt wieder
                  scharf geschaltet werden.

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::setPeriodicInterruptMode(RTCPeriodicInterruptMode_t periodicInterruptMode)
{
	uint8_t mode = (uint8_t)periodicInterruptMode;
  
  writeRegister(RTC_DCF_REG_PERIODIC_INTERRUPT, &mode, 1);
}

/*---------------------------------------------------------
Name:           enablePeriodicInterrupt

Beschreibung:   aktiviert den eingestellten periodischen
                Interrupt
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enablePeriodicInterrupt(void)
{
	uint8_t perIntRegister;

  readRegister(RTC_DCF_REG_PER_INT_CONFIG, 1, &perIntRegister);

  perIntRegister |= RTC_PER_INT_CONFIG_REGISTER_PIE;

  writeRegister(RTC_DCF_REG_PER_INT_CONFIG, &perIntRegister, 1);
}

/*---------------------------------------------------------
Name:           disablePeriodicInterrupt

Beschreibung:   deaktiviert den eingestellten periodischen
                Interrupt
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disablePeriodicInterrupt(void)
{
	uint8_t perIntRegister;

  readRegister(RTC_DCF_REG_PER_INT_CONFIG, 1, &perIntRegister);

  perIntRegister &= ~RTC_PER_INT_CONFIG_REGISTER_PIE;

	writeRegister(RTC_DCF_REG_PER_INT_CONFIG, &perIntRegister, 1);
}

/*---------------------------------------------------------
Name:           resetPeriodicInterrupt

Beschreibung:   schaltet den periodischen Interrupt bei
                Auswahl eines der Level-Modi wieder scharf
                (siehe setPeriodicInterrupt)
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::resetPeriodicInterrupt(void)
{
  uint8_t statusRegister = RTC_STATUS_PIF;  
  
  writeRegister(RTC_DCF_REG_STATUS, &statusRegister, 1);
}

/*---------------------------------------------------------
Name:           enablePeriodicInterruptLED

Beschreibung:   aktiviert, dass die LED bei einem periodischen Interrupt
                ein- bzw. nach ausschalten des Interrupts wieder ausgeschaltet
                wird.
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enablePeriodicInterruptLED(void)
{
	uint8_t perIntRegister;

  readRegister(RTC_DCF_REG_PER_INT_CONFIG, 1, &perIntRegister);

  perIntRegister |= RTC_PER_INT_CONFIG_REGISTER_PILED;

	writeRegister(RTC_DCF_REG_PER_INT_CONFIG, &perIntRegister, 1);
}

/*---------------------------------------------------------
Name:           disablePeriodicInterruptLED

Beschreibung:   deaktiviert, dass die LED bei einem periodischen Interrupt
                ein- bzw. nach ausschalten des Interrupts wieder ausgeschaltet
                wird.
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disablePeriodicInterruptLED(void)
{
	uint8_t perIntRegister;

  readRegister(RTC_DCF_REG_PER_INT_CONFIG, 1, &perIntRegister);

  perIntRegister &= ~RTC_PER_INT_CONFIG_REGISTER_PILED;

	writeRegister(RTC_DCF_REG_PER_INT_CONFIG, &perIntRegister, 1);
}

/*---------------------------------------------------------
Name:           enableDCF77Reception

Beschreibung:   aktiviert den DCF77-Empfang
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enableDCF77Reception(void)
{
	uint8_t dcf77Register;

  readRegister(RTC_DCF_REG_DCF77_CONFIG, 1, &dcf77Register);

  dcf77Register |= RTC_DCF77_CONFIG_REGISTER_DCFE;

	writeRegister(RTC_DCF_REG_DCF77_CONFIG, &dcf77Register, 1);
}

/*---------------------------------------------------------
Name:           disableDCF77Reception

Beschreibung:   deaktiviert den DCF77-Empfang
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disableDCF77Reception(void)
{
	uint8_t dcf77Register;

  readRegister(RTC_DCF_REG_DCF77_CONFIG, 1, &dcf77Register);

  dcf77Register &= ~RTC_DCF77_CONFIG_REGISTER_DCFE;

	writeRegister(RTC_DCF_REG_DCF77_CONFIG, &dcf77Register, 1);
}

/*---------------------------------------------------------
Name:           enableDCF77Interrupt

Beschreibung:   aktiviert den Interrupt bei DCF77-Empfang
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enableDCF77Interrupt(void)
{
	uint8_t dcf77Register;

  readRegister(RTC_DCF_REG_DCF77_CONFIG, 1, &dcf77Register);

  dcf77Register |= RTC_DCF77_CONFIG_REGISTER_DCFIE;

	writeRegister(RTC_DCF_REG_DCF77_CONFIG, &dcf77Register, 1);
}

/*---------------------------------------------------------
Name:           disableDCF77Interrupt

Beschreibung:   deaktiviert den Interrupt bei DCF77-Empfang
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disableDCF77Interrupt(void)
{
	uint8_t dcf77Register;

  readRegister(RTC_DCF_REG_DCF77_CONFIG, 1, &dcf77Register);

  dcf77Register &= ~RTC_DCF77_CONFIG_REGISTER_DCFIE;

	writeRegister(RTC_DCF_REG_DCF77_CONFIG, &dcf77Register, 1);
}

/*---------------------------------------------------------
Name:           resetAlarmInterrupt

Beschreibung:   schaltet den Interrupt bei DCF77-Empfang
                wieder scharf
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::resetDCF77Interrupt(void)
{
  uint8_t statusRegister = RTC_STATUS_DCFIF;  
  
  writeRegister(RTC_DCF_REG_STATUS, &statusRegister, 1);
}

/*---------------------------------------------------------
Name:           enableDCF77LED

Beschreibung:   aktiviert, dass die LED bei einem DCF77-Empfangs-Interrupt
                ein- bzw. nach ausschalten des Interrupts wieder ausgeschaltet
                wird.
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::enableDCF77LED(void)
{
	uint8_t dcf77Register;

  readRegister(RTC_DCF_REG_DCF77_CONFIG, 1, &dcf77Register);

  dcf77Register |= RTC_DCF77_CONFIG_REGISTER_DCFLED;

	writeRegister(RTC_DCF_REG_DCF77_CONFIG, &dcf77Register, 1);
}

/*---------------------------------------------------------
Name:           disableDCF77LED

Beschreibung:   deaktiviert, dass die LED bei einem DCF77-Empfangs-Interrupt
                ein- bzw. nach ausschalten des Interrupts wieder ausgeschaltet
                wird.
 
Eing�nge:       void

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::disableDCF77LED(void)
{
	uint8_t dcf77Register;

  readRegister(RTC_DCF_REG_DCF77_CONFIG, 1, &dcf77Register);

  dcf77Register &= ~RTC_DCF77_CONFIG_REGISTER_DCFLED;

	writeRegister(RTC_DCF_REG_DCF77_CONFIG, &dcf77Register, 1);
}


/**********************************************************
private Methoden
**********************************************************/

/*---------------------------------------------------------
Name:           writeRegister

Beschreibung:   schreibt eine vorgegebene Anzahl Register des
                des RTC-DCF
 
Eing�nge:       unsigned char regAddress
                  Registeradresse ab der die Daten geschrieben
                  werden sollen

                unsigned char *data
                  zu schreibende Daten

                unsigned char datalength
                  Anzahl der zu schreibenden Daten

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::writeRegister(RTCRegister_t regAddress, uint8_t *data, uint8_t datalength)
{

  digitalWrite(_ssPin, LOW);

  delayMicroseconds(50);
  _softSPI.transfer(regAddress | SPI_BURST_WRITE);

  while(datalength--)
  {
    delayMicroseconds(50);
    _softSPI.transfer(*data);

    data++;
  }
  
  digitalWrite(_ssPin, HIGH);

  delayMicroseconds(300);

}

/*---------------------------------------------------------
Name:           readRegister

Beschreibung:   liest eine vorgegebene Anzahl Register des
                des RTC-DCF
 
Eing�nge:       unsigned char regAddress
                  Registeradresse ab der die Daten gelesen
                  werden sollen

                unsigned char length
                  Anzahl der zu lesenden Daten

                unsigned char *data
                  gelesene Daten

Ausgang:        void
---------------------------------------------------------*/
void RealTimeClock_DCF::readRegister(RTCRegister_t regAddress, uint8_t length, uint8_t *data)
{
    digitalWrite(_ssPin, LOW);

    delayMicroseconds(50);
    _softSPI.transfer(regAddress | SPI_BURST_READ);

    while(length--)
    {
      delayMicroseconds(50);
      *data = _softSPI.transfer(0xff);

      data++;
    }
    
    digitalWrite(_ssPin, HIGH);

    delayMicroseconds(300);
}

/*---------------------------------------------------------
Name:           decimalToBCD

Beschreibung:   rechnet Dezimalwerte in das BCD-System um
                (der RTC-DCF arbeitet intern mit BCD-Daten)
 
Eing�nge:       unsigned char decimal
                  umzurechnender Dezimalwert

Ausgang:        unsigned char
                  BCD-Wert der �bergebenen Dezimalzahl
---------------------------------------------------------*/
uint8_t RealTimeClock_DCF::decimalToBCD(uint8_t decimal)
{
    return (((decimal / 10) << 4) | (decimal % 10));
}

/*---------------------------------------------------------
Name:           BCDToDecimal

Beschreibung:   rechnet BCD-Werte in das Dezimal-System um
                (der RTC-DCF arbeitet intern mit BCD-Daten)
 
Eing�nge:       unsigned char bcd
                  umzurechnender BCD-Wert

Ausgang:        unsigned char
                  Dezimalwert der �bergebenen BCD-Zahl
---------------------------------------------------------*/
uint8_t RealTimeClock_DCF::BCDToDecimal(uint8_t bcd)
{
    return (((((bcd)>>4) & 0x0F) * 10) + ((bcd) & 0x0F));
}

#endif

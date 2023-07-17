/*
 * INA3221.h
 *
 *  Created on: 04.07.2023
 *      Author: D-Laptop
 */

#ifndef INA3221_H
#define INA3221_H

#include <Wire.h>
#include <Beastdevices_INA3221.h>

Beastdevices_INA3221 ina3221(INA3221_ADDR41_VCC);

class INA3221
{
public:
   INA3221(uint8_t address);
   void begin();
   int readCurrent();
   void setup();

private:
   struct SensorData
   {
	   int32_t value; // Value from INA3221 sensor
	   ina3221_ch_t  channel; // Index of the sensor
   };

   uint8_t _address;
   void readTask();
   void calcTask();

   QueueHandle_t _xQueue;
};

   #endif

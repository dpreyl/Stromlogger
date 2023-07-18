/*
 * INA3221.h
 *
 *  Created on: 04.07.2023
 *      Author: D-Laptop
 */

#ifndef INA3221_H
#define INA3221_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <Wire.h>
#include <Beastdevices_INA3221.h>

extern Beastdevices_INA3221 ina3221;

class INA3221
{
public:
   INA3221();
   void begin();
   int32_t readCurrent();
   void setup();

   void starter();
private:
   ina3221_ch_t channel;
   struct SensorData
   {
	   int32_t value; // Value from INA3221 sensor
	   ina3221_ch_t  channel; // Index of the sensor
   };

   void readTask();
   void calcTask();

   //QueueHandle_t _xQueue;

   hw_timer_t * timer = NULL;
   static void IRAM_ATTR onTimer();
   static SemaphoreHandle_t timerSemaphore;

   static void timerTask(void * param);

   static void taskmanager(void * param);
   static uint32_t skipCounter;
};

extern INA3221 sensor;

   #endif

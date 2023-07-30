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
	struct ResultSensorData{
		   uint16_t rmsValue;
		   uint16_t maxValue;
	   };
	   struct ResultData{
		   ResultSensorData sensorData[3];
		   uint32_t timestamp;
	   };

   INA3221();
   void begin();
   int32_t readCurrent(uint8_t channel);
   void setup();

   void starter();

   void printData(uint8_t channel);
   void printData();
   ResultData receiveResultData();
   ResultData getCurrentValues();

   static void printResultData(INA3221::ResultData *resultData);
private:
   struct SensorData
   {
	   int32_t value; // Value from INA3221 sensor
	   uint8_t  channel; // Index of the sensor
   };
   struct SumData{
	   uint64_t sum;
	   uint16_t count;
	   long lastReset;
	   uint16_t maxReading;
   };

   uint16_t maxReading[3];
   uint16_t rmsValue[3];
   SumData sensorSumData[3];

   QueueHandle_t _xQueueSensorData;
   QueueHandle_t _xQueueResultData;

   hw_timer_t * timer = NULL;
   static void IRAM_ATTR onTimer();

   static SemaphoreHandle_t timerSemaphore;
   static SemaphoreHandle_t sensorDataMutex;

   static void timerTask(void * param);
   static void taskmanager(void * param);
   static void calcSensorTask(void * param);
   static void calcTimerTask(void * param);

   static uint32_t skipCounter;
   static uint32_t sensorReadTimer;
};

extern INA3221 sensor;

   #endif

/*
 * INA3221.cpp
 *
 *  Created on: 04.07.2023
 *      Author: D-Laptop
 */
#include "INA3221.h"

Beastdevices_INA3221 ina3221(INA3221_ADDR41_VCC);
INA3221 sensor;
SemaphoreHandle_t INA3221::timerSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t INA3221::sensorDataMutex = xSemaphoreCreateMutex();
uint32_t INA3221::skipCounter=0;
uint32_t INA3221::sensorReadTimer=0;

INA3221::INA3221()
{
}

void INA3221::begin()
{
	Wire.begin(-1, -1, (uint32_t)500000);
	Wire.setTimeOut(10);
   ina3221.begin();
   ina3221.reset();

   // Set shunt resistors to 100 mOhm for all channels
   ina3221.setShuntRes(100, 100, 100);

   ina3221.setAveragingMode(INA3221_REG_CONF_AVG_1);
   ina3221.setBusMeasDisable();
   ina3221.setCritAlertLatchDisable();
   ina3221.setCurrentSumDisable(INA3221_CH1);
   ina3221.setCurrentSumDisable(INA3221_CH2);
   ina3221.setCurrentSumDisable(INA3221_CH3);
   ina3221.setFilterRes(10, 10, 10);
   // Bei 50Hz, 3Phasen, 32Samples/Amplitude => 208,3us pro Sample
   ina3221.setShuntConversionTime(INA3221_REG_CONF_CT_204US);
   ina3221.setModeContinious();

   timerAlarmEnable(this->timer);  // Enable the timer
}

int32_t INA3221::readCurrent(uint8_t channel)
{
   return ina3221.getShuntVoltage(static_cast<ina3221_ch_t>(channel));

}

void INA3221::setup() {
	  this->_xQueueSensorData = xQueueCreate(100, sizeof(SensorData));
	  this->_xQueueResultData = xQueueCreate(30, sizeof(ResultData));
	  this->timer = timerBegin(1, 2, true);  // Use timer 1, with a prescaler of 80
	  timerAttachInterrupt(timer, &INA3221::onTimer, true);  // Attach the interrupt function
	  timerAlarmWrite(timer, 8332, true);  // Set the alarm value

	  xTaskCreate(INA3221::timerTask, "timerTask", 2048, NULL, 40, NULL);
	  xTaskCreate(INA3221::calcSensorTask, "calcSensorTask", 2048, NULL, 35, NULL);
	  xTaskCreate(INA3221::calcTimerTask, "calcTimerTask", 2048, NULL, 32, NULL);
}

void IRAM_ATTR INA3221::onTimer() {
  static BaseType_t xHigherPriorityTaskWoken;
  xSemaphoreGiveFromISR(INA3221::timerSemaphore, &xHigherPriorityTaskWoken);
  if(!xHigherPriorityTaskWoken){
	  ++INA3221::skipCounter;
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void INA3221::starter() {
	xTaskCreatePinnedToCore(INA3221::taskmanager, "Taskmanager", 4096, NULL, 31, NULL, 1);
}

void INA3221::taskmanager(void * param){
	sensor.setup();
	sensor.begin();
	while(true){
		vTaskDelay(1000/ portTICK_RATE_MS);
	}
}

void INA3221::timerTask(void * param) {
	uint32_t readcount = 0;
	long start = millis();
	int32_t sensorValue;
	struct SensorData xSensorData = {0, 0};
  while(true) {
    if(xSemaphoreTake(INA3221::timerSemaphore, 1000/ portTICK_RATE_MS) == pdTRUE) {
    	xSensorData.value = sensor.readCurrent(xSensorData.channel);
    	if(xQueueSend(sensor._xQueueSensorData, &xSensorData, ( TickType_t ) 0 ) != pdTRUE){
    		Serial.println("Fehler, sensorValueQueue ist voll");
    	}
    	xSensorData.channel = ((xSensorData.channel + 1) % 3);
        ++readcount;
    } else {
    	Serial.println("Schwerer Fehler, timeout in INA3221::timerTask");
    }
    if(readcount >= 4800){
    	long end = millis();
    	INA3221::sensorReadTimer = end - start;
    	start = end;
    	readcount = 0;

//    	Serial.print("4800 Samples gelesen in: ");
//    	Serial.print(micros() - start);
//    	Serial.print("us ");
//    	Serial.print("Uebersprungen: ");
//    	Serial.println(INA3221::skipCounter);
//    	start = micros();
//    	INA3221::skipCounter = 0;
    }
  }
}

void INA3221::printData(uint8_t channel) {
	Serial.print("Sensordaten von Sensor ");
	Serial.print(channel);
	Serial.print(": RMS ");
	Serial.print(rmsValue[channel]);
	Serial.print(" peak ");
	Serial.println(maxReading[channel]);

}

void INA3221::printData() {
	Serial.print("Sensordaten:\t");
	for (int channel = 0; channel < 3; ++channel) {
		Serial.print(rmsValue[channel]);
		Serial.print("\t");
		Serial.print(maxReading[channel]);
		Serial.print("\t");
	}
	Serial.println("");

}

INA3221::ResultData INA3221::getCurrentValues() {
	ResultData ret;
	for (int ii = 0; ii < 3; ++ii) {
		ret.sensorData[ii].rmsValue = rmsValue[ii];
		ret.sensorData[ii].maxValue = maxReading[ii];
	}
	ret.timestamp = (uint32_t)millis();
	return ret;
}

void INA3221::calcTimerTask(void * param) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	/* Bei 50Hz, 32 Samples pro Periode können maximal 10 Perioden in einem int summiert werden
	 */
	const TickType_t xFrequency = 200 / portTICK_RATE_MS;
	uint8_t channel;
	SumData sensorSumData;

	SumData averageData[3] = {0,0,0,0,0,0,0,0,0,0,0,0};
	long tsNext = millis() + 10000L;
	long now;

	ResultData resultData;

	while(true){
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		for (channel = 0; channel < 3; ++channel) {
			if(xSemaphoreTake(INA3221::sensorDataMutex, 1000/ portTICK_RATE_MS) == pdTRUE){ // enter critical section
				 sensorSumData = sensor.sensorSumData[channel];
				 sensor.sensorSumData[channel].count=0;
				 sensor.sensorSumData[channel].sum=0;
				 sensor.sensorSumData[channel].maxReading=0;
				 sensor.sensorSumData[channel].lastReset=millis();
				 xSemaphoreGive(INA3221::sensorDataMutex); // exit critical section
			} else {
					Serial.println("Schwerer Fehler, timeout2 in INA3221::calcTimerTask");
			}
			if(sensorSumData.count < 50){
				Serial.println("Fehler, zu wenig Sensordaten fuer RMS");
			} else {
				sensor.rmsValue[channel] = sqrt(sensorSumData.sum / sensorSumData.count);
				sensor.maxReading[channel] = sensorSumData.maxReading;

				averageData[channel].sum += sensor.rmsValue[channel];
				++averageData[channel].count;
				if (averageData[channel].maxReading < sensor.maxReading[channel]){
					averageData[channel].maxReading = sensor.maxReading[channel];
				}
			}
		}
		//sensor.printData();
		now = millis();
		if(now - tsNext > 0){
			for (int ii = 0; ii < 3; ++ii) {
				if(averageData[ii].count < 50){
					Serial.println("Fehler, zu wenig Sensordaten fuer average");
					resultData.sensorData[ii].maxValue = 0;
					resultData.sensorData[ii].rmsValue = 0;
				} else {
					resultData.sensorData[ii].maxValue = averageData[ii].maxReading;
					resultData.sensorData[ii].rmsValue = averageData[ii].sum / averageData[ii].count;
				}
				averageData[ii].maxReading = 0;
				averageData[ii].sum = 0;
				averageData[ii].count = 0;
			}
			resultData.timestamp = (uint32_t)(now);
			if(xQueueSend(sensor._xQueueResultData, &resultData, ( TickType_t ) 0 ) != pdTRUE){
			    Serial.println("Fehler, sensorResultQueue ist voll");
			}
			printResultData(&resultData);
			tsNext += 10000L; // alle 10 Sekunden
		}
	}

}

INA3221::ResultData INA3221::receiveResultData() {
	ResultData res = {0,0,0,0,0,0,0};
	if( xQueueReceive( sensor._xQueueResultData, &( res ), 30000/ portTICK_RATE_MS )  != pdTRUE ){
    	Serial.println("Schwerer Fehler, timeout in INA3221::calcSensorTask");
    }
	return res;
}

void INA3221::printResultData(INA3221::ResultData *resultData){
	Serial.print("Ergebnisdaten:\t");
	for (int channel = 0; channel < 3; ++channel) {
		Serial.print(resultData->sensorData[channel].rmsValue);
		Serial.print("\t");
		Serial.print(resultData->sensorData[channel].maxValue);
		Serial.print("\t");
	}
	Serial.print(" Timestamp: ");
	Serial.println(resultData->timestamp);
}

void INA3221::calcSensorTask(void *param) {
	struct SensorData xSensorData;
	uint32_t sqValue;
	int32_t value;
	uint32_t absValue;
	while(true){
	 if( xQueueReceive( sensor._xQueueSensorData, &( xSensorData ), 1000/ portTICK_RATE_MS ) == pdTRUE ){
		 value = xSensorData.value;
		 sqValue = sq(value);
		 absValue = abs(value);
		 if(xSemaphoreTake(INA3221::sensorDataMutex, 1000/ portTICK_RATE_MS) == pdTRUE){ // enter critical section
			 if(absValue > sensor.sensorSumData[xSensorData.channel].maxReading){
				 sensor.sensorSumData[xSensorData.channel].maxReading = absValue;
			 }
			 sensor.sensorSumData[xSensorData.channel].sum += sqValue;
			 ++sensor.sensorSumData[xSensorData.channel].count;
			 xSemaphoreGive(INA3221::sensorDataMutex); // exit critical section
		 } else {
		    	Serial.println("Schwerer Fehler, timeout2 in INA3221::calcSensorTask");
		    }
	 }
	 else {
	    	Serial.println("Schwerer Fehler, timeout in INA3221::calcSensorTask");
	    }
	}
}

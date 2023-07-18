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
uint32_t INA3221::skipCounter=0;

INA3221::INA3221()
{
   this->channel=INA3221_CH1;
}

void INA3221::begin()
{
	Wire.begin(-1, -1, (uint32_t)500000);
   ina3221.begin();
   ina3221.reset();

   // Set shunt resistors to 100 mOhm for all channels
   ina3221.setShuntRes(100, 100, 100);

   ina3221.setAveragingMode(INA3221_REG_CONF_AVG_1);
   ina3221.setBusMeasDisable();
   //ina3221.setCritAlertLatchDisable();
   //ina3221.setCurrentSumDisable(INA3221_CH1);
   //ina3221.setCurrentSumDisable(INA3221_CH2);
   //ina3221.setCurrentSumDisable(INA3221_CH3);
   ina3221.setFilterRes(10, 10, 10);
   // Bei 50Hz, 3Phasen, 32Samples/Amplitude => 208,3us pro Sample
   ina3221.setShuntConversionTime(INA3221_REG_CONF_CT_204US);
   ina3221.setModeContinious();

   timerAlarmEnable(this->timer);  // Enable the timer
}

int32_t INA3221::readCurrent()
{
   this->channel = static_cast<ina3221_ch_t>((this->channel + 1) % 3);
   return ina3221.getShuntVoltage(this->channel);

}

void INA3221::setup() {
	//_xQueue = xQueueCreate(10, sizeof(SensorData));
	  this->timer = timerBegin(1, 2, true);  // Use timer 1, with a prescaler of 80
	  timerAttachInterrupt(timer, &INA3221::onTimer, true);  // Attach the interrupt function
	  timerAlarmWrite(timer, 8332, true);  // Set the alarm value

	  xTaskCreate(INA3221::timerTask, "timerTask", 2048, NULL, 1, NULL);
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
	xTaskCreatePinnedToCore(INA3221::taskmanager, "Taskmanager", 4096, NULL, 10, NULL, 1);
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
	long start = micros();
  while(true) {
    if(xSemaphoreTake(INA3221::timerSemaphore, portMAX_DELAY) == pdTRUE) {
    	sensor.readCurrent();
      ++readcount;
    }
    if(readcount >= 4800){
    	Serial.print("4800 Samples gelesen in: ");
    	Serial.print(micros() - start);
    	Serial.print("us ");
    	Serial.print("Uebersprungen: ");
    	Serial.println(INA3221::skipCounter);
    	readcount = 0;
    	start = micros();
    	INA3221::skipCounter = 0;
    }
  }
}

void INA3221::readTask() {
}

void INA3221::calcTask() {
}


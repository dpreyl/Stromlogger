/*
 * main.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */
#include <Arduino.h>
#include <INA3221.h>
#include <MyDatalogger.h>
#include <Ui.h>

void mainTask(void *param) {
	myDatalogger.setup();
	TickType_t xLastWakeTime = xTaskGetTickCount();

	Ui().setup();

	vTaskDelayUntil( &xLastWakeTime, 500/ portTICK_RATE_MS);

	xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = 10000 / portTICK_RATE_MS;
	while(true){
		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		Serial.println("alive");
		//myDatalogger.printCollectedData();
	}
}

void setup(void) {
	Serial.begin(115200);
	sensor.starter();
	xTaskCreatePinnedToCore(mainTask, "mainTask", 8192, NULL, 2, NULL, 0);
}


void loop() {
	//Serial.println(sensor.readCurrent());
	//Serial.println(ina3221.getCurrentCompensated(INA3221_CH3), 4);
	vTaskDelay(1000/ portTICK_RATE_MS);
}

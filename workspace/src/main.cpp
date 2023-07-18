/*
 * main.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */
#include <Arduino.h>
#include <INA3221.h>

void setup(void) {
	Serial.begin(115200);
	sensor.starter();
}


void loop() {
	//Serial.println(sensor.readCurrent());
	//Serial.println(ina3221.getCurrentCompensated(INA3221_CH3), 4);
	vTaskDelay(1000/ portTICK_RATE_MS);
	Serial.println("alive");
}

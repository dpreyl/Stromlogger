/*
 * main.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */
#include <Arduino.h>
#include <INA3221.h>
#include <MyDatalogger.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid = "ZS_Stromlogger";
const char* password = "geheim";

AsyncWebServer server(80);

void mainTask(void *param) {
	myDatalogger.setup();

	TickType_t xLastWakeTime = xTaskGetTickCount();

	Serial.begin(115200);
	WiFi.mode(WIFI_AP);
	WiFi.begin(ssid, password);
	Serial.println("");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		vTaskDelay(500/ portTICK_RATE_MS);
	    Serial.print(".");
	  }

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	AsyncElegantOTA.begin(&server);    // Start ElegantOTA
	server.begin();
	Serial.println("HTTP server started");

	vTaskDelayUntil( &xLastWakeTime, 500/ portTICK_RATE_MS);

	const TickType_t xFrequency = 10000 / portTICK_RATE_MS;
	while(true){

		vTaskDelayUntil( &xLastWakeTime, xFrequency );

		Serial.println("alive");
		myDatalogger.printCollectedData();
	}
}
void setup(void) {
	Serial.begin(115200);
	sensor.starter();
	xTaskCreatePinnedToCore(mainTask, "mainTask", 8192, NULL, 1, NULL, 0);
}


void loop() {
	//Serial.println(sensor.readCurrent());
	//Serial.println(ina3221.getCurrentCompensated(INA3221_CH3), 4);
	vTaskDelay(1000/ portTICK_RATE_MS);
}

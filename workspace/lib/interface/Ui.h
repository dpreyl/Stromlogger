/*
 * ui.h
 *
 *  Created on: 28.07.2023
 *      Author: D-Laptop
 */

#ifndef LIB_INTERFACE_UI_H_
#define LIB_INTERFACE_UI_H_

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

#include "AsyncJson.h"
#include "ArduinoJson.h"

#include <LittleFS.h>

#include <INA3221.h>
#include <MyDatalogger.h>


class Ui{
public:
	void setup();
private:
	static AsyncWebServer server;
	const char* ssid = "ZS_Stromlogger";
	const char* password = "geheimnis";
	void setupWebpages();
	static void sendSensorValue(AsyncWebServerRequest *request);
	static void sendSequence(AsyncWebServerRequest *request);
	static void deleteFile(AsyncWebServerRequest *request);
	static void sendCSVData(AsyncWebServerRequest *request);
	static void parseSensorResultData(JsonObject  jsonBuffer, INA3221::ResultData *data);
	static void parseSeqDataCollection(JsonObject  jsonBuffer, MyDatalogger::DataCollection *data);
};

class CSVReader{
public:
	CSVReader(uint32_t stSeqNo, uint32_t endSeqNo);
	size_t read(uint8_t *buffer, size_t maxLen);
private:
	uint32_t stSeqNo;
	uint32_t endSeqNo;
	uint32_t curSeqNo;

	static size_t parseSeqDataCollection(char *buffer, MyDatalogger::DataCollection *data);
	static size_t parseSensorResultData(char *buffer, INA3221::ResultData *data, uint32_t seqNo);
};





#endif /* LIB_INTERFACE_UI_H_ */

/*
 * ui.h
 *
 *  Created on: 28.07.2023
 *      Author: D-Laptop
 */

#ifndef LIB_INTERFACE_UI_H_
#define LIB_INTERFACE_UI_H_

#include <FSBrowser.h>

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

class CSVReader: public Stream{
public:
	CSVReader(uint32_t stSeqNo, uint32_t endSeqNo, bool chunked=false);
	~CSVReader();
	size_t readBytes(uint8_t *buffer, size_t maxLen);
	size_t size(){return CONTENT_LENGTH_UNKNOWN;};
	String name();
	int available();
	int read(){return 0;};
	int peek(){return 0;};
	size_t write(uint8_t){return 0;};
private:
	uint32_t stSeqNo;
	uint32_t endSeqNo;
	uint32_t curSeqNo;
	bool _chunked = false;
	uint8_t * buf;

	static size_t parseSeqDataCollection(char *buffer, MyDatalogger::DataCollection *data);
	static size_t parseSensorResultData(char *buffer, INA3221::ResultData *data, uint32_t seqNo);
};





#endif /* LIB_INTERFACE_UI_H_ */

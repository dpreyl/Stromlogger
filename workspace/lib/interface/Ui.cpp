/*
 * ui.cpp
 *
 *  Created on: 28.07.2023
 *      Author: D-Laptop
 */
#include "Ui.h"

AsyncWebServer Ui::server(80);

void Ui::setup() {
	// Connect to Wi-Fi network with SSID and password
	Serial.println("Setting AP (Access Point)...");
	// Remove the password parameter, if you want the AP (Access Point) to be open
	WiFi.softAP(ssid, password);

	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	AsyncElegantOTA.begin(&Ui::server);    // Start ElegantOTA
	server.begin();
	Serial.println("HTTP server started");

	if (!MDNS.begin("esp32")) {
		Serial.println("Error setting up MDNS responder!");
	}
	Serial.println("mDNS responder started");

	MDNS.addService("http", "tcp", 80);

	Ui::setupWebpages();
}

void Ui::setupWebpages() {
	server.on("/sensor", HTTP_GET, Ui::sendSensorValue);
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
	  String html = "<!DOCTYPE html>"
	                "<html>"
	                "<body>"
	                "<p id='sensorValue'></p>"
	                "<script>"
	                "setInterval(function(){"
	                "  fetch('/sensor').then(response => response.text()).then(data => {"
	                "    document.getElementById('sensorValue').innerHTML = data;"
	                "  });"
	                "}, 1000);"
	                "</script>"
	                "</body>"
	                "</html>";
	  request->send(200, "text/html", html);
	});
	server.on("/getSequence", HTTP_GET, Ui::sendSequence);
	server.on("/getCurrentSequenceNo", HTTP_GET, [](AsyncWebServerRequest *request){
		  uint32_t seqNo = myDatalogger.getCurrentSequenceNo();
		  request->send(200, "text/html", String(seqNo));
		});
	server.on("/deleteFile", HTTP_GET, Ui::deleteFile);
	server.on("/getCSVData", HTTP_GET, Ui::sendCSVData);
	server.serveStatic("/data", LittleFS, "/data/");
}

void Ui::sendCSVData(AsyncWebServerRequest *request){
	uint32_t stSeqNo = 1;
	uint32_t endSeqNo = stSeqNo;
	uint32_t curSeqNo = myDatalogger.getCurrentSequenceNo();
	if(request->hasParam("stSeqNo")){
		stSeqNo = (uint32_t) request->getParam("stSeqNo")->value().toInt();
	}
	if(request->hasParam("endSeqNo")){
		endSeqNo = (uint32_t) request->getParam("endSeqNo")->value().toInt();
		if(endSeqNo > curSeqNo){
			endSeqNo = curSeqNo;
		}
	} else {
		endSeqNo = curSeqNo;
	}
	if(stSeqNo > endSeqNo){
		request->send(400, "text/html", "stSeqNo("+String(stSeqNo)+") > endSeqNo("+String(endSeqNo)+")");
		return;
	}

	CSVReader cSVReader(stSeqNo, endSeqNo);

	AsyncWebServerResponse *response = request->beginChunkedResponse("text/csv", [cSVReader](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
	  //Write up to "maxLen" bytes into "buffer" and return the amount written.
	  //index equals the amount of bytes that have been already sent
	  //You will be asked for more data until 0 is returned
	  //Keep in mind that you can not delay or yield waiting for more data!
	  return cSVReader.read(buffer, maxLen);
	});
	response->addHeader("Server","ESP Async Web Server");
	String filename = String((uint32_t)ESP.getEfuseMac(), HEX) + "_" + String(stSeqNo) + "-" + String(endSeqNo) + ".csv";
	response->addHeader("Content-Disposition", "attachment;filename="+filename);
	request->send(response);
}

void Ui::deleteFile(AsyncWebServerRequest *request){
	if(!request->hasParam("fileNo")){
		request->send(400, "text/html", "Bitte fileNo angeben");
		return;
	}
	uint32_t fileNo = (uint32_t) request->getParam("fileNo")->value().toInt();
	bool res = myDatalogger.deleteFile(fileNo);
	if(res){
		request->send(200, "text/html", "Erfolg");
	} else {
		request->send(400, "text/html", "Fehler");
	}
}

void Ui::sendSensorValue(AsyncWebServerRequest *request) {
	INA3221::ResultData data = sensor.getCurrentValues();

	StaticJsonDocument<192> jsonBuffer;

	Ui::parseSensorResultData(jsonBuffer.to<JsonObject>(), &data);

	AsyncResponseStream *response = request->beginResponseStream("application/json");
	serializeJson(jsonBuffer, *response);

	request->send(response);
}

void Ui::sendSequence(AsyncWebServerRequest *request) {
	if(!request->hasParam("seqNo")){
		request->send(400, "text/html", "Bitte seqNo angeben");
		return;
	}
	uint32_t seqNo = (uint32_t) request->getParam("seqNo")->value().toInt();
	if(seqNo > myDatalogger.getCurrentSequenceNo()){
		request->send(400, "text/html", "seqNo zu gross");
		return;
	}
	MyDatalogger::DataCollection seq = MyDatalogger::getSequence(seqNo);
	StaticJsonDocument<1536> jsonBuffer;

	Ui::parseSeqDataCollection(jsonBuffer.to<JsonObject>(), &seq);

	AsyncResponseStream *response = request->beginResponseStream("application/json");
	serializeJson(jsonBuffer, *response);

	request->send(response);
}

void Ui::parseSensorResultData(JsonObject jsonBuffer, INA3221::ResultData *data){
	for (int ii = 0; ii < 3; ++ii) {
		jsonBuffer["sensorData"][ii]["rms"]=data->sensorData[ii].rmsValue;
		jsonBuffer["sensorData"][ii]["max"]=data->sensorData[ii].maxValue;
	}
	jsonBuffer["timestamp"]=data->timestamp;
}

void Ui::parseSeqDataCollection(JsonObject jsonBuffer, MyDatalogger::DataCollection *data){
	for (int ii = 0; ii < 6; ++ii) {
		JsonObject seqData = jsonBuffer["seqData"].createNestedObject();
		Ui::parseSensorResultData(seqData, &data->sensorData[ii]);
	}
	jsonBuffer["seqNo"] = data->seqNo;
}

CSVReader::CSVReader(uint32_t stSeqNo, uint32_t endSeqNo) : stSeqNo(stSeqNo), endSeqNo(endSeqNo), curSeqNo(stSeqNo) {
}

size_t CSVReader::read(uint8_t *buffer, size_t maxLen) {
	size_t len = 0;
	while(true){
		if(this->curSeqNo > this->endSeqNo){
			return 0;
		}
		Serial.print("Read seqNo: ");
		Serial.println(this->curSeqNo);
		MyDatalogger::DataCollection seq = MyDatalogger::getSequence(this->curSeqNo);
		Serial.print("Seq 2");
		if(seq.seqNo == this->curSeqNo){
			len += CSVReader::parseSeqDataCollection((char*) buffer, &seq);
		}
		Serial.print("Seq 3");
		vTaskDelay(10 / portTICK_RATE_MS);
		esp_task_wdt_reset();
		Serial.print("Seq 4");
		++this->curSeqNo;
		if(len > 0){
			Serial.print("Seq 41");
			return len;
		}
		Serial.print("Seq 42");
	}
}
size_t CSVReader::parseSeqDataCollection(char *buffer, MyDatalogger::DataCollection *data){
	size_t len = 0;
	for (int ii = 0; ii < 6; ++ii) {
		len += CSVReader::parseSensorResultData(buffer+len, &data->sensorData[ii], data->seqNo);
	}
	return len;
}
size_t CSVReader::parseSensorResultData(char *buffer, INA3221::ResultData *data, uint32_t seqNo){
	size_t len = 0;
	len += sprintf(buffer+len, "%d;%d", seqNo, data->timestamp);
	for (int ii = 0; ii < 3; ++ii) {
		len += sprintf(buffer+len, ";%d;%d", data->sensorData[ii].rmsValue, data->sensorData[ii].maxValue);
	}
	len += sprintf(buffer+len, "\r\n");
	return len;
}

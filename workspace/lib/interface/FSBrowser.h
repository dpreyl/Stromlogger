/*
 * FSBrowser.h
 *
 *  Created on: 30.07.2023
 *      Author: D-Laptop
 */

#ifndef LIB_INTERFACE_FSBROWSER_H_
#define LIB_INTERFACE_FSBROWSER_H_


#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <Ui.h>


class FSBrowser{
public:
	static void setup(int port = 80);
	FSBrowser(int port = 80);
private:
	WebServer server;
	//holds the current upload
	File fsUploadFile;

	static void task(void* parameter);
	void _setup();
	static void initFs();
	void handleFileList();
	void handleFileCreate();
	void handleFileDelete();
	void handleFileUpload();
	bool handleFileRead(String path);
	bool handleStaticFileRead(String path);
	static bool exists(String path);
	String getContentType(String filename);
	String formatBytes(size_t bytes);

	void handleCSVData();
};


#endif /* LIB_INTERFACE_FSBROWSER_H_ */

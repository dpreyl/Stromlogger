/*
  FSWebServer - Example WebServer with FS backend for esp8266/esp32
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP32 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `ls -A1`; do curl -F "file=@$PWD/$file" esp32fs.local/edit; done

  access the sample web page at http://esp32fs.local
  edit the page by going to http://esp32fs.local/edit
*/

#include "FSBrowser.h"

#define FILESYSTEM LittleFS
// You only need to format the filesystem once
#define FORMAT_FILESYSTEM false
#define DBG_OUTPUT_PORT Serial

FSBrowser::FSBrowser(int port) : server(port) {
}

//format bytes
String FSBrowser::formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String FSBrowser::getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool FSBrowser::exists(String path){
  bool yes = false;
  return FILESYSTEM.exists(path);
//  File file = FILESYSTEM.open(path, "r");
//  if(!file.isDirectory()){
//    yes = true;
//  }
//  file.close();
//  return yes;
}

bool FSBrowser::handleStaticFileRead(String path) {
	const int num = 9;
	const char *fNames[] = {
			"/edit.htm",
			"/favicon.ico",
//			"/graphs.js",
//			"/ace.js",
//			"/index.htm",
//			"/ext-searchbox.js",
//			"/worker-html.js",
//			"/mode-javascript.js",
//			"/mode-html.js",
//			"/mode-css.js",
			0};
	extern const uint8_t _binary_data_edit_htm_gz_start[] asm("_binary_data_edit_htm_gz_start");
	extern const uint8_t _binary_data_edit_htm_gz_end[] asm("_binary_data_edit_htm_gz_end");
	extern const uint8_t _binary_data_favicon_ico_gz_start[] asm("_binary_data_favicon_ico_gz_start");
	extern const uint8_t _binary_data_favicon_ico_gz_end[] asm("_binary_data_favicon_ico_gz_end");

	const uint8_t *ptr_start[] = {
			_binary_data_edit_htm_gz_start,
			_binary_data_favicon_ico_gz_start
	};
	const uint8_t *ptr_end[] = {
			_binary_data_edit_htm_gz_end,
			_binary_data_favicon_ico_gz_end
	};
	int i = 0;
	while(fNames[i]) {
		if(strcmp(fNames[i], path.c_str()) == 0) {
			break;
		}
		++i;
	}
	if(!fNames[i]){
		return false;
	}
	Serial.println("Handle static file: "+path);
	String contentType = getContentType(path);
	server.send_P(200, contentType.c_str(), (const char*)ptr_start[i], ptr_end[i]-ptr_start[i]);
	return true;
}

bool FSBrowser::handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
//  if(handleStaticFileRead(path)){
//	  return true;
//  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (exists(pathWithGz) || exists(path)) {
    if (exists(pathWithGz)) {
      path = pathWithGz;
      DBG_OUTPUT_PORT.println(".gz found: " + path);
    }
    File file = FILESYSTEM.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void FSBrowser::handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = FILESYSTEM.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void FSBrowser::handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  FILESYSTEM.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void FSBrowser::handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = FILESYSTEM.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void FSBrowser::handleCSVData(){
	uint32_t stSeqNo = 1;
	uint32_t endSeqNo = 0;
	if(server.hasArg("stSeqNo")){
		stSeqNo = (uint32_t) server.arg("stSeqNo").toInt();
	}
	if(server.hasArg("endSeqNo")){
		endSeqNo = (uint32_t) server.arg("endSeqNo").toInt();
	}

	if(stSeqNo > endSeqNo && endSeqNo>0){
		server.send(500, "text/plain", "stSeqNo("+String(stSeqNo)+") > endSeqNo("+String(endSeqNo)+")");
		return;
	}

	CSVReader cSVReader(stSeqNo, endSeqNo, true);

	server.sendHeader(F("Content-Disposition"), "attachment;filename="+cSVReader.name());

	server.streamFile(cSVReader, "text/csv");
}

void FSBrowser::handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);


  File root = FILESYSTEM.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.path()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.send(200, "text/json", output);
}

void FSBrowser::_setup() {

  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);

  {
      File root = FILESYSTEM.open("/");
      File file = root.openNextFile();
      while(file){
          String fileName = file.name();
          size_t fileSize = file.size();
          DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
          file.close();
          file = root.openNextFile();
      }
      file.close();
      root.close();
      DBG_OUTPUT_PORT.printf("\n");
  }

  FSBrowser::initFs();


  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, [this]() { handleFileList();});
  //load editor
  server.on("/edit", HTTP_GET, [this]() {
    if (!this->handleFileRead("/edit.htm")) {
    	this->server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
  server.on("/edit", HTTP_PUT, [this]() { handleFileCreate();});
  //delete file
  server.on("/edit", HTTP_DELETE, [this]() { handleFileDelete();});
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, [this]() {
	  this->server.send(200, "text/plain", "");
  }, [this]() { handleFileUpload();});

  server.on("/getCSVData", HTTP_GET, [this]() { handleCSVData();});

  //called when the url is not defined here
  //use it to load content from FILESYSTEM
  server.onNotFound([this]() {
    if (!this->handleFileRead(this->server.uri())) {
    	this->server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, [this]() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(0));
    json += ", \"freeSpace\":" + String(myFS.freeSpace());
    json += ", \"totalSpace\":" + String(myFS.totalBytes());
    json += ", \"usedSpace\":" + String(myFS.usedBytes());
    json += "}";
    this->server.send(200, "text/json", json);
    json = String();
  });

  server.on("/deleteOldestLog", HTTP_GET, [this](){
		  myDatalogger.removeOldestLog();
		  this->server.send(200, "text/plain", "OK");
		});
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
}

void FSBrowser::initFs(){
	String path;
	File fsFile;
	path = "/edit.htm.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_edit_htm_gz_start[] asm("_binary_data_edit_htm_gz_start");
		extern const uint8_t _binary_data_edit_htm_gz_end[] asm("_binary_data_edit_htm_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_edit_htm_gz_start, _binary_data_edit_htm_gz_end - _binary_data_edit_htm_gz_start);
		fsFile.close();
	}
	path = "/favicon.ico.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_favicon_ico_gz_start[] asm("_binary_data_favicon_ico_gz_start");
		extern const uint8_t _binary_data_favicon_ico_gz_end[] asm("_binary_data_favicon_ico_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_favicon_ico_gz_start, _binary_data_favicon_ico_gz_end - _binary_data_favicon_ico_gz_start);
		fsFile.close();
	}
	path = "/graphs.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_graphs_js_gz_start[] asm("_binary_data_graphs_js_gz_start");
		extern const uint8_t _binary_data_graphs_js_gz_end[] asm("_binary_data_graphs_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_graphs_js_gz_start, _binary_data_graphs_js_gz_end - _binary_data_graphs_js_gz_start);
		fsFile.close();
	}
	path = "/ace.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_ace_js_gz_start[] asm("_binary_data_ace_js_gz_start");
		extern const uint8_t _binary_data_ace_js_gz_end[] asm("_binary_data_ace_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_ace_js_gz_start, _binary_data_ace_js_gz_end - _binary_data_ace_js_gz_start);
		fsFile.close();
	}
	path = "/index.htm.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_index_htm_gz_start[] asm("_binary_data_index_htm_gz_start");
		extern const uint8_t _binary_data_index_htm_gz_end[] asm("_binary_data_index_htm_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_index_htm_gz_start, _binary_data_index_htm_gz_end - _binary_data_index_htm_gz_start);
		fsFile.close();
	}
	path = "/ext-searchbox.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_ext_searchbox_js_gz_start[] asm("_binary_data_ext_searchbox_js_gz_start");
		extern const uint8_t _binary_data_ext_searchbox_js_gz_end[] asm("_binary_data_ext_searchbox_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_ext_searchbox_js_gz_start, _binary_data_ext_searchbox_js_gz_end - _binary_data_ext_searchbox_js_gz_start);
		fsFile.close();
	}
	path = "/worker-html.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_worker_html_js_gz_start[] asm("_binary_data_worker_html_js_gz_start");
		extern const uint8_t _binary_data_worker_html_js_gz_end[] asm("_binary_data_worker_html_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_worker_html_js_gz_start, _binary_data_worker_html_js_gz_end - _binary_data_worker_html_js_gz_start);
		fsFile.close();
	}
	path = "/mode-javascript.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_mode_javascript_js_gz_start[] asm("_binary_data_mode_javascript_js_gz_start");
		extern const uint8_t _binary_data_mode_javascript_js_gz_end[] asm("_binary_data_mode_javascript_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_mode_javascript_js_gz_start, _binary_data_mode_javascript_js_gz_end - _binary_data_mode_javascript_js_gz_start);
		fsFile.close();
	}
	path = "/mode-html.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_mode_html_js_gz_start[] asm("_binary_data_mode_html_js_gz_start");
		extern const uint8_t _binary_data_mode_html_js_gz_end[] asm("_binary_data_mode_html_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_mode_html_js_gz_start, _binary_data_mode_html_js_gz_end - _binary_data_mode_html_js_gz_start);
		fsFile.close();
	}
	path = "/mode-css.js.gz";
	if(!exists(path)){
		DBG_OUTPUT_PORT.println("Erstelle: "+path);
		extern const uint8_t _binary_data_mode_css_js_gz_start[] asm("_binary_data_mode_css_js_gz_start");
		extern const uint8_t _binary_data_mode_css_js_gz_end[] asm("_binary_data_mode_css_js_gz_end");
		fsFile = FILESYSTEM.open(path, "w");
		fsFile.write(_binary_data_mode_css_js_gz_start, _binary_data_mode_css_js_gz_end - _binary_data_mode_css_js_gz_start);
		fsFile.close();
	}
	DBG_OUTPUT_PORT.println("FS checked");
}

void FSBrowser::setup(int port) {
  xTaskCreate(FSBrowser::task, "FSBrowserLoopTask", 8192, (void*) port, 2, NULL);
}

void FSBrowser::task(void* parameter) {
	FSBrowser myFSBrowser((int) parameter);
	myFSBrowser._setup();
	while(true){
		myFSBrowser.server.handleClient();
		vTaskDelay(2 / portTICK_RATE_MS);
	}
}

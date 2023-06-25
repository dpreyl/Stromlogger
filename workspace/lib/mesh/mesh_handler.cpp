/*
 * mesh_handler.cpp
 *
 *  Created on: 20.06.2023
 *      Author: D-Laptop
 */

#include "mesh_handler.h"

// Define the static handheldId outside the class
uint32_t MeshHandler::handheldId = 0;

// Implementation of the MeshHandler constructor
MeshHandler::MeshHandler()
  : taskSendRSSIandPeers(STATION_DATA_INTERVAL, TASK_FOREVER, [this]() { sendRSSIandPeersCallback(); }),
    taskSendHandheldId(30000, TASK_FOREVER, [this]() { sendHandheldIdCallback(); }),
    otaHandler(mesh, myFS, "/"),
    taskOTACheck(60000, TASK_FOREVER, [this]() { otaHandler.loop(); })
{}



// Implementation of the MeshHandler static methods
void MeshHandler::sendHandheldIdCallback() {
	StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
	jsonDoc["handheldId"] = mesh.getNodeId();
  meshHandler.sendMessage(jsonDoc, HANDHELDID);

  jsonDoc.clear();
  jsonDoc["currentTime"] = now();
  jsonDoc["meshTime"] = mesh.getNodeTime();
  meshHandler.sendMessage(jsonDoc, TIMEBROADCAST);
}


// Add this method to send the RSSI and connected peers using the task scheduler
void MeshHandler::sendRSSIandPeersCallback() {
#ifdef DEVICE_TYPE_HANDHELD
	Serial.println("Zeige lokale Daten an.");
  meshHandler.handleStationData(meshHandler.collectRSSIandPeers());
#endif
#ifdef DEVICE_TYPE_MEAS
  Serial.println("Sende Daten zum Master");
  meshHandler.sendRSSIandPeers(meshHandler.collectRSSIandPeers());
#endif
}

void MeshHandler::setup() {

  mesh.setDebugMsgTypes(ERROR | STARTUP);

//#ifdef DEVICE_TYPE_HANDHELD
//  WiFi.mode(WIFI_AP);
//  WiFi.softAP(MESH_PREFIX, MESH_PASSWORD);
//  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N) );
//
//  WiFi.mode(WIFI_AP_STA);
//  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_STA);
//  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR) );
//  WiFi.mode(WIFI_AP_STA);
//#endif
//#ifdef DEVICE_TYPE_MEAS
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA);
  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR) );
  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR) );
//#endif

  // Set the bandwidth to 20 MHz for the station interface
  // ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20));

  // Set the bandwidth to 20 MHz for the access point interface
  // ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  otaHandler.begin();

  userScheduler.addTask(taskSendRSSIandPeers);
  taskSendRSSIandPeers.enable();

#ifdef DEVICE_TYPE_HANDHELD
  userScheduler.addTask(taskSendHandheldId);
  taskSendHandheldId.enable();

  userScheduler.addTask(taskOTACheck);
  taskOTACheck.enable();
#endif
}

void MeshHandler::loop() {
  mesh.update();
}

void MeshHandler::sendMessage(StaticJsonDocument<JSON_DOC_SIZE> jsonDoc, MessageTypes type, uint32_t reciverId) {
	jsonDoc["messageType"] = type;
	// Convert the JSON object to a string
	String jsonData;
	serializeJson(jsonDoc, jsonData);
	if(reciverId != 0){
		mesh.sendSingle(reciverId, jsonData);
	} else if (handheldId != 0) {
    mesh.sendSingle(handheldId, jsonData);
  } else {
	  mesh.sendBroadcast(jsonData);
  }
}

void MeshHandler::newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\r\n", nodeId);
}

void MeshHandler::changedConnectionCallback() {
  Serial.printf("Changed connections\r\n");
}

void MeshHandler::nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\r\n", mesh.getNodeTime(), offset);
}

StaticJsonDocument<JSON_DOC_SIZE> MeshHandler::collectRSSIandPeers(){
    // Create a JSON object to store the data
    StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
    jsonDoc["nodeId"] = mesh.getNodeId();
    jsonDoc["STArssi"] = WiFi.RSSI();
    jsonDoc["peers"] = mesh.subConnectionJson();
    jsonDoc["time"] = now();

//    // Convert the JSON object to a string
//    String jsonData;
//    serializeJson(jsonDoc, jsonData);
    return jsonDoc;
}

void MeshHandler::sendRSSIandPeers(StaticJsonDocument<JSON_DOC_SIZE> jsonDoc) {
	// Send the data to the handheld device
	//Serial.println(jsonData);
    sendMessage(jsonDoc, MEASUREMENT);
}

// Add this new method to handle the station data received from other nodes
void MeshHandler::handleStationData(StaticJsonDocument<JSON_DOC_SIZE> jsonDoc) {
  // Parse the JSON data
//  StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
//  DeserializationError error = deserializeJson(jsonDoc, data);

//  if (error) {
//    Serial.println("Failed to parse station data");
//    return;
//  }

  // Extract the data from the JSON object
  uint32_t nodeId = jsonDoc["nodeId"];
  int rssi = jsonDoc["STArssi"];
  String peers = jsonDoc["peers"];

  Serial.printf("%u\t%i\t%s\r\n", nodeId, rssi, peers.c_str());

  // Process the data (e.g., store it in a database or display it on a web page)
  // ...
}

void MeshHandler::receivedCallback(uint32_t from, String &msg) {
	// Parse the JSON data
	Serial.println("Message recieved:");
			Serial.println(msg);
	StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
	  DeserializationError error = deserializeJson(jsonDoc, msg);

	  if (error) {
	    Serial.println("Failed to parse station data");
	    return;
	  }

	  if(compareMessageType(jsonDoc["messageType"], HANDHELDID)){
		  uint32_t handheldId = (uint32_t)strtol(jsonDoc["handheldId"], nullptr, 10);
		  setHandheldId(handheldId);
	  } else if (compareMessageType(jsonDoc["messageType"], TIMEBROADCAST)){
		  uint32_t recievedMeshTime = (uint32_t)strtol(jsonDoc["meshTime"], nullptr, 10);
		  uint32_t currentTime = (uint32_t)strtol(jsonDoc["currentTime"], nullptr, 10);
		  if(labs(static_cast<long>(recievedMeshTime) - static_cast<long>(mesh.getNodeTime())) < 100){
			  	Serial.printf("Setze aktuelle Zeit auf %u\r\n", currentTime);
				setTime(currentTime);
		  	  }
		  else {
			  Serial.printf("Zeitdiff zu groß: %i", recievedMeshTime - mesh.getNodeTime());
		  }
	  } else if (compareMessageType(jsonDoc["messageType"], MEASUREMENT)){
#ifdef DEVICE_TYPE_HANDHELD
		  Serial.println("Daten vn Sation empfangen:");
		  meshHandler.handleStationData(jsonDoc);
#endif
	  } else {
		    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
		  }
}

bool MeshHandler::compareMessageType(ArduinoJson::JsonVariantConst jsonVariant, MeshHandler::MessageTypes messageType) {
  if (jsonVariant.is<int>()) {
    int intValue = jsonVariant.as<int>();
    return intValue == static_cast<int>(messageType);
  }
  return false;
}

void MeshHandler::setHandheldId(uint32_t handheldId) {
	MeshHandler::handheldId = handheldId;
}


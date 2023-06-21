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
    taskSendHandheldId(30000, TASK_FOREVER, [this]() { sendHandheldIdCallback(); })
{}



// Implementation of the MeshHandler static methods
void MeshHandler::sendHandheldIdCallback() {
  String msg = "HANDHELD_ID:" + String(mesh.getNodeId());
  meshHandler.sendMessage(msg);
}


// Add this method to send the RSSI and connected peers using the task scheduler
void MeshHandler::sendRSSIandPeersCallback() {
#ifdef DEVICE_TYPE_HANDHELD
	Serial.println("Zeige lokale Daten an.");
  meshHandler.handleStationData(meshHandler.collectRSSIandPeers());
#endif
#ifdef DEVICE_TYPE_MEAS
  meshHandler.sendRSSIandPeers(meshHandler.collectRSSIandPeers());
#endif
}

void MeshHandler::setup() {

  mesh.setDebugMsgTypes(ERROR | STARTUP);

#ifdef DEVICE_TYPE_HANDHELD
  WiFi.mode(WIFI_AP);
  WiFi.softAP(MESH_PREFIX, MESH_PASSWORD);
  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N) );

  WiFi.mode(WIFI_AP_STA);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_STA);
  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR) );
  WiFi.mode(WIFI_AP_STA);
#endif
#ifdef DEVICE_TYPE_MEAS
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA);
  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_LR) );
  ESP_ERROR_CHECK( esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_LR) );
#endif

  // Set the bandwidth to 20 MHz for the station interface
  ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20));

  // Set the bandwidth to 20 MHz for the access point interface
  ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20));

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendRSSIandPeers);
  taskSendRSSIandPeers.enable();

#ifdef DEVICE_TYPE_HANDHELD
  userScheduler.addTask(taskSendHandheldId);
  taskSendHandheldId.enable();

  mesh.initOTAReceive("otaHandheld");
#endif
#ifdef DEVICE_TYPE_MEAS
  mesh.initOTAReceive("otaStation");
#endif
}

void MeshHandler::loop() {
  mesh.update();
}

void MeshHandler::sendMessage(const String &msg) {
  if (handheldId != 0) {
    mesh.sendSingle(handheldId, msg);
  } else {
	  mesh.sendBroadcast(msg);
  }
}

void MeshHandler::newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void MeshHandler::changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void MeshHandler::nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

String MeshHandler::collectRSSIandPeers(){
    // Create a JSON object to store the data
    StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
    jsonDoc["nodeId"] = mesh.getNodeId();
    jsonDoc["STArssi"] = WiFi.RSSI();
    jsonDoc["peers"] = mesh.subConnectionJson();

    // Convert the JSON object to a string
    String jsonData;
    serializeJson(jsonDoc, jsonData);
    return jsonData;
}

void MeshHandler::sendRSSIandPeers(String jsonData) {
	// Send the data to the handheld device
    sendMessage(jsonData);
}

// Add this new method to handle the station data received from other nodes
void MeshHandler::handleStationData(const String &data) {
  // Parse the JSON data
  StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, data);

  if (error) {
    Serial.println("Failed to parse station data");
    return;
  }

  // Extract the data from the JSON object
  uint32_t nodeId = jsonDoc["nodeId"];
  int rssi = jsonDoc["STArssi"];
  String peers = jsonDoc["peers"];

  Serial.printf("%u\t%i\t%s\r\n", nodeId, rssi, peers.c_str());

  // Process the data (e.g., store it in a database or display it on a web page)
  // ...
}

void MeshHandler::receivedCallback(uint32_t from, String &msg) {
  // Check if the message is from the handheld device
  if (msg.startsWith("HANDHELD_ID:")) {
    uint32_t handheldId = (uint32_t)strtol(msg.substring(12).c_str(), nullptr, 10);
    setHandheldId(handheldId);
  } else if (msg.startsWith("{") && msg.endsWith("}")) {
	  Serial.println("Daten von Messstation erhalten");
	meshHandler.handleStationData(msg);
  } else {
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  }
}


void MeshHandler::setHandheldId(uint32_t handheldId) {
	MeshHandler::handheldId = handheldId;
}


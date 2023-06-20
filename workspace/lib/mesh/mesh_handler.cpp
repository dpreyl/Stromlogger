/*
 * mesh_handler.cpp
 *
 *  Created on: 20.06.2023
 *      Author: D-Laptop
 */

#include <TaskSchedulerDeclarations.h> // Include TaskScheduler declarations
#include "mesh_handler.h"

#define MESH_PREFIX     "yourPrefix"
#define MESH_PASSWORD   "yourPassword"
#define MESH_PORT       5555

#define STATION_DATA_INTERVAL 10000 // 10 seconds
#define JSON_DOC_SIZE 512

MeshHandler::MeshHandler() {}


void MeshHandler::setup() {
  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  callbackWrapper = CallbackWrapper(this);

  taskSendHandheldId.set(30000, TASK_FOREVER, [](Task* task) { callbackWrapper.sendHandheldIdCallback(task); }); // Send handheld ID every 30 seconds
    mesh.mScheduler.addTask(taskSendHandheldId);
    taskSendHandheldId.enable();

    taskSendRSSIandPeers.set(STATION_DATA_INTERVAL, TASK_FOREVER, [](Task* task) { callbackWrapper.sendRSSIandPeersCallback(task); });
    mesh.mScheduler.addTask(taskSendRSSIandPeers);
    taskSendRSSIandPeers.enable();
}

void MeshHandler::loop() {
  mesh.update();
}

void MeshHandler::sendMessage(const String &msg) {
  if (handheldId != 0) {
    mesh.sendSingle(handheldId, msg);
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

void MeshHandler::sendRSSIandPeers() {
    // Create a JSON object to store the data
    StaticJsonDocument<JSON_DOC_SIZE> jsonDoc;
    jsonDoc["nodeId"] = mesh.getNodeId();
    jsonDoc["rssi"] = WiFi.RSSI();
    jsonDoc["apPeers"] = mesh.getAPConnections();
    jsonDoc["staPeers"] = mesh.getStationConnections();

    // Convert the JSON object to a string
    String jsonData;
    serializeJson(jsonDoc, jsonData);

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
  int rssi = jsonDoc["rssi"];
  int apPeers = jsonDoc["apPeers"];
  int staPeers = jsonDoc["staPeers"];

  // Process the data (e.g., store it in a database or display it on a web page)
  // ...
}

void MeshHandler::receivedCallback(uint32_t from, String &msg) {
  // Check if the message is from the handheld device
  if (msg.startsWith("HANDHELD_ID:")) {
    uint32_t handheldId = (uint32_t)strtol(msg.substring(12).c_str(), nullptr, 10);
    setHandheldId(handheldId);
  } else if (msg.startsWith("{") && msg.endsWith("}")) {
    handleStationData(msg);
  } else {
    Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  }
}

void MeshHandler::setHandheldId(uint32_t handheldId) {
  this->handheldId = handheldId;
}

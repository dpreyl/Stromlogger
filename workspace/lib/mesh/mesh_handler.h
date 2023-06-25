/*
 * mesh_handler.h
 *
 *  Created on: 20.06.2023
 *      Author: D-Laptop
 */
#pragma once

#include <Arduino.h>
#include "painlessMesh.h"
#include <TaskSchedulerDeclarations.h>
#include "OTAHandler.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <TimeLib.h>

#define MESH_PREFIX     "yourPrefix"
#define MESH_PASSWORD   "yourPassword"
#define MESH_PORT       5555

#define JSON_DOC_SIZE 512
#define STATION_DATA_INTERVAL 10000 // 10 seconds

extern Scheduler userScheduler; // to control your personal task
extern painlessMesh mesh;
extern MyFS myFS;

class MeshHandler {
public:
  MeshHandler();
  void setup();
  void loop();

  enum MessageTypes {
      HANDHELDID,
      TIMEBROADCAST,
      MEASUREMENT
    };

  void sendMessage(StaticJsonDocument<JSON_DOC_SIZE> jsonDoc, MessageTypes type, uint32_t reciverId=0);

  StaticJsonDocument<JSON_DOC_SIZE> collectRSSIandPeers();
  void sendRSSIandPeers(StaticJsonDocument<JSON_DOC_SIZE> jsonDoc);

  void handleStationData(StaticJsonDocument<JSON_DOC_SIZE> jsonDoc);

  OTAHandler otaHandler;

private:
  static void receivedCallback(uint32_t from, String &msg);
  static void newConnectionCallback(uint32_t nodeId);
  static void changedConnectionCallback();
  static void nodeTimeAdjustedCallback(int32_t offset);

  static void setHandheldId(uint32_t handheldId);
  static uint32_t handheldId;

  void sendRSSIandPeersCallback();
  void sendHandheldIdCallback();

  static bool compareMessageType(ArduinoJson::JsonVariantConst jsonVariant, MeshHandler::MessageTypes messageType);

  Task taskSendRSSIandPeers;
  Task taskSendHandheldId;
  Task taskOTACheck;
};

extern MeshHandler meshHandler;

/*
 * mesh_handler.h
 *
 *  Created on: 20.06.2023
 *      Author: D-Laptop
 */
#pragma once

#include <TaskSchedulerDeclarations.h> // Include TaskScheduler declarations
#include <Arduino.h>
#include "painlessMesh.h"
#include "callback_wrapper.h"
#include <functional> // Include the functional library

class MeshHandler {
public:
  MeshHandler();
  void setup();
  void loop();
  void sendMessage(const String &msg);

  painlessMesh mesh;

  void sendRSSIandPeers();
private:
  static void receivedCallback(uint32_t from, String &msg);
  static void newConnectionCallback(uint32_t nodeId);
  static void changedConnectionCallback();
  static void nodeTimeAdjustedCallback(int32_t offset);

  void handleStationData(const String &data);

  void setHandheldId(uint32_t handheldId);
  uint32_t handheldId = 0;

  CallbackWrapper callbackWrapper;

  Task taskSendRSSIandPeers;
  Task taskSendHandheldId;
};

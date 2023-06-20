/*
 * callback_wrapper.cpp
 *
 *  Created on: 20.06.2023
 *      Author: D-Laptop
 */
#include "callback_wrapper.h"
#include "mesh_handler.h" // Include MeshHandler header

CallbackWrapper::CallbackWrapper(MeshHandler* meshHandler)
  : meshHandler(meshHandler) {}

void CallbackWrapper::sendHandheldIdCallback(Task* task) {
  String msg = "HANDHELD_ID:" + String(meshHandler->mesh.getNodeId());
  meshHandler->sendMessage(msg);
}

void CallbackWrapper::sendRSSIandPeersCallback(Task* task) {
  meshHandler->sendRSSIandPeers();
}




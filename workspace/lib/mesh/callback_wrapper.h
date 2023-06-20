/*
 * callback_wrapper.h
 *
 *  Created on: 20.06.2023
 *      Author: D-Laptop
 */

#ifndef CALLBACK_WRAPPER_H
#define CALLBACK_WRAPPER_H

#include <TaskSchedulerDeclarations.h> // Include TaskScheduler declarations

class MeshHandler; // Forward declaration of MeshHandler class

class CallbackWrapper {
public:
  CallbackWrapper(MeshHandler* meshHandler);
  void sendHandheldIdCallback(Task* task);
  void sendRSSIandPeersCallback(Task* task);

private:
  MeshHandler* meshHandler;
};

#endif // CALLBACK_WRAPPER_H

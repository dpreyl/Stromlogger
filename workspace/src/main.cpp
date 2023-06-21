/*
 * main.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */
#include <Arduino.h>
#include "mesh_handler.h"

Scheduler userScheduler;
painlessMesh mesh;
MeshHandler meshHandler;

void setup() {
  Serial.begin(115200);
  meshHandler.setup();
}

void loop() {
  meshHandler.loop();
}

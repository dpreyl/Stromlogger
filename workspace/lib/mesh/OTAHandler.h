/*
 * OTAHandler.h
 *
 *  Created on: 24.06.2023
 *      Author: D-Laptop
 */

#ifndef MESH_OTA_HANDLER_H
#define MESH_OTA_HANDLER_H

#include <Arduino.h>
#include <painlessMesh.h>
#include "MyFS.h"

#define OTA_PART_SIZE 1024 //How many bytes to send per OTA data packet

class OTAHandler {
public:
    OTAHandler(painlessMesh& mesh, MyFS& fs, const char* firmwareFolder);
    void begin();
    void loop();

private:
    painlessMesh& _mesh;
    MyFS& _fs;
    const char* _firmwareFolder;

    char role[64];
    char hardware[64];
    String md5Value;
    size_t noPart=0;

    void checkForNewFirmware();
};

#endif

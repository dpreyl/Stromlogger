/*
 * MyLittleFS.cpp
 *
 *  Created on: 22.06.2023
 *      Author: D-Laptop
 */
#include <MyFS.h>

MyFS::MyFS(uint8_t mosi, uint8_t miso, uint8_t sck, uint8_t cs) : _spi(VSPI), _cs(cs), _spiInitialized(false) {
#ifdef DEVICE_TYPE_HANDHELD
        _spi.begin(sck, miso, mosi, cs);
        _spiInitialized = true;
#else
        //Serial.println("SD card not allowed when not handheld");
#endif
}

bool MyFS::begin(bool formatOnFailure) {
#ifdef DEVICE_TYPE_HANDHELD
        if (!_spiInitialized) {
            Serial.println("SPI not initialized for SD card");
            return false;
        }
        if (!SD.begin(_cs, _spi)) {
            Serial.println("SD card initialization failed");
            return false;
        }
        Serial.println("SD card initialized");
#endif
#ifdef DEVICE_TYPE_MEAS
    if (!LittleFS.begin(formatOnFailure)) {
        Serial.println("LittleFS initialization failed");
        return false;
    }
    Serial.println("LittleFS initialized");
#endif
return true;
}

void MyFS::listFiles(const char* dir) {
	File root = open(dir);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    File file = root.openNextFile();
    while (file) {
        Serial.print("FILE: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
}

File MyFS::open(const char* path, const char* mode, const bool create){
#ifdef DEVICE_TYPE_HANDHELD
	File root = SD.open(path, mode, create);
#endif
#ifdef DEVICE_TYPE_MEAS
	File root = LittleFS.open(path, mode, create);
#endif
	return root;
}

// Implement more functions here as needed




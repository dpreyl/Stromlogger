#ifndef MY_LITTLE_FS_H
#define MY_LITTLE_FS_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>

#define SD_MOSI_PIN 15
#define SD_MISO_PIN 2
#define SD_SCK_PIN  14
#define SD_CS_PIN   13

class MyFS {
public:
    MyFS(uint8_t mosi = SD_MOSI_PIN, uint8_t miso = SD_MISO_PIN, uint8_t sck = SD_SCK_PIN, uint8_t cs = SD_CS_PIN);
    bool begin(bool formatOnFailure=false);
    void listFiles(const char* dir);
    // Add more function declarations here as needed
    File open(const char* path, const char* mode = FILE_READ, const bool create = false);
private:
    SPIClass _spi;
    uint8_t _cs;
    bool _spiInitialized;
};

#endif

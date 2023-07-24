/*
 * MyLittleFS.cpp
 *
 *  Created on: 22.06.2023
 *      Author: D-Laptop
 */
#include <MyFS.h>

MyFS myFS;
SemaphoreHandle_t MyFS::fsMutex = xSemaphoreCreateMutex();

MyFS::MyFS() {
}

bool MyFS::begin(bool formatOnFailure) {
    if (!LittleFS.begin(formatOnFailure)) {
        Serial.println("LittleFS initialization failed");
        return false;
    }
    Serial.println("LittleFS initialized");
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
	File root = LittleFS.open(path, mode, create);
	return root;
}

bool MyFS::remove(const char* path){
	return LittleFS.remove(path);
}

void MyFS::mkdir(const char* path){
	LittleFS.mkdir(path);
}

BaseType_t MyFS::takeSemaphore(unsigned int timeout) {
	return xSemaphoreTake(MyFS::fsMutex, timeout / portTICK_RATE_MS);
}

void MyFS::giveSemaphore() {
	xSemaphoreGive(MyFS::fsMutex);
}
// Implement more functions here as needed

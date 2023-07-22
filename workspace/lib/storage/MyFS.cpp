/*
 * MyLittleFS.cpp
 *
 *  Created on: 22.06.2023
 *      Author: D-Laptop
 */
#include <MyFS.h>

MyFS myFS;

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

// Implement more functions here as needed

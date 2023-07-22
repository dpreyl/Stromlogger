#ifndef MY_LITTLE_FS_H
#define MY_LITTLE_FS_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>


class MyFS {
public:
    MyFS();
    bool begin(bool formatOnFailure=false);
    void listFiles(const char* dir);
    // Add more function declarations here as needed
    File open(const char* path, const char* mode = FILE_READ, const bool create = false);
private:
};

extern MyFS myFS;

#endif

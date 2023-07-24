/*
 * MyDatalogger.cpp
 *
 *  Created on: 22.07.2023
 *      Author: D-Laptop
 */
#include "MyDatalogger.h"

MyDatalogger myDatalogger;
SemaphoreHandle_t MyDatalogger::fsMutex = xSemaphoreCreateMutex();

void MyDatalogger::setup() {
	if(myFS.takeSemaphore(fsMutexTimeout) != pdTRUE){
		Serial.println("Timeout beim warten aufs FS setup");
	} else {
		myFS.begin(false);
		File root = myFS.open("/data");
		if (!root || !root.isDirectory()) {
			myFS.mkdir("/data");
		} else {
			root.close();
		}

		this->seqNo = findSequenceNo();
		myFS.giveSemaphore(); // exit critical section
	}
	Serial.print("Sequenz Nummer: ");
	Serial.println(this->seqNo);
	xTaskCreate(MyDatalogger::dataLoggerTask, "dataLoggerTask", 8192, NULL, 10, NULL);
}

uint32_t MyDatalogger::findSequenceNo() {
	uint32_t seqNo=0;
	File root = myFS.open("/data");
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory");
        return 0;
    }
    File file = root.openNextFile();
    while (file) {
    	Serial.println(file.name());

    	while(file.read((byte *)&tmpDataCollection, sizeof(DataCollection)) > 0){
    		if(tmpDataCollection.seqNo > seqNo){
    			seqNo = tmpDataCollection.seqNo;
    		}
    	}
    	file.close();
    	file = root.openNextFile();
    }
    file.close();
    root.close();
    return seqNo;
}

void MyDatalogger::dataLoggerTask(void *param) {
	while(true){
		myDatalogger.tmpDataCollection.seqNo = ++myDatalogger.seqNo;
		for (int ii = 0; ii < 6; ++ii) {
			myDatalogger.tmpDataCollection.sensorData[ii] = sensor.receiveResultData();
		}
		MyDatalogger::writeDataCollection(&myDatalogger.tmpDataCollection);
	}
}

void MyDatalogger::printCollectedData() {
	DataCollection readDataCollection;
	if(myFS.takeSemaphore(fsMutexTimeout) != pdTRUE){
		Serial.println("Timeout beim warten aufs FS print");
	} else {
		File root = myFS.open("/data");
		if (!root || !root.isDirectory()) {
			Serial.println("Failed to open directory");
			return;
		}
		File file = root.openNextFile();
		while (file) {
			Serial.print("Datei: ");
			Serial.print(file.name());
			Serial.print(" Groese: ");
			Serial.println(file.size());

			ssize_t res = file.read((byte *)&readDataCollection, sizeof(DataCollection));
			while(res > 0){
				printDataCollection(&readDataCollection);
				res = file.read((byte *)&readDataCollection, sizeof(DataCollection));
				}
			file.close();
			file = root.openNextFile();
		}
		file.close();
		root.close();
		myFS.giveSemaphore(); // exit critical section
	}
}

void MyDatalogger::printDataCollection(DataCollection *dataCollection) {
	Serial.print("Datacollection, Sequenz Nummer: ");
	Serial.println(dataCollection->seqNo);
	for (int ii = 0; ii < 6; ++ii) {
		INA3221::printResultData(&dataCollection->sensorData[ii]);
	}
}

void MyDatalogger::writeDataCollection(DataCollection *dataCollection) {
	uint32_t fileindex = (dataCollection->seqNo)/60;
	String path = "/data/"+String(fileindex)+".zsl";
	Serial.print("Schreibe dataCollection, Sequenz Nummer: ");
	Serial.print(dataCollection->seqNo);
	Serial.print(" Datei: ");
	Serial.println(path);
	if(myFS.takeSemaphore(fsMutexTimeout) != pdTRUE){
		Serial.println("Timeout beim warten aufs FS write");
	} else {
		File file = myFS.open(path.c_str(), FILE_APPEND, true);
		size_t res = file.write((byte *)dataCollection, sizeof(DataCollection));
		if(res <= 0){
			Serial.println("Fehler beim schreiben");
		} else {
			Serial.print("Erfolgreich ");
			Serial.print(res);
			Serial.print(" Bytes von ");
			Serial.print(sizeof(DataCollection));
			Serial.println(" geschrieben");
		}
		file.flush();
		file.close();
		myFS.giveSemaphore(); // exit critical section
	}
}

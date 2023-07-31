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

		findSequenceNo();
		myFS.giveSemaphore(); // exit critical section
	}
	Serial.print("Sequenz Nummer: ");
	Serial.println(this->seqNo);
	xTaskCreate(MyDatalogger::dataLoggerTask, "dataLoggerTask", 8192, NULL, 18, NULL);
	xTaskCreate(MyDatalogger::houskeepingTask, "houskeepingTask", 8192, NULL, 1, NULL);
}

void MyDatalogger::findSequenceNo() {
	DataCollection myDataCollection;
	File root = myFS.open("/data");
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory");
        return;
    }
    File file = root.openNextFile();
    while (file) {
    	Serial.println(file.name());

    	while(file.read((byte *)&myDataCollection, sizeof(DataCollection)) > 0){
    		if(myDataCollection.seqNo > seqNo){
    			seqNo = myDataCollection.seqNo;
    		}
    		if(myDataCollection.seqNo < minSeqNo){
    			minSeqNo = myDataCollection.seqNo;
    		}
    	}
    	file.close();
    	file = root.openNextFile();
    }
    file.close();
    root.close();
    Serial.println("Min seq no: "+String(minSeqNo)+" cur seq no: "+String(seqNo));
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
void MyDatalogger::houskeepingTask(void *param) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = 60000 / portTICK_RATE_MS;
	while(true){
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
		Serial.println("Housekeeping");
		if(myFS.freeSpace() < 300000){
			Serial.println("Loesche um Platz zu gewinnen, aktuell frei: "+String(myFS.freeSpace()));
			myDatalogger.removeOldestLog();
		}
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

MyDatalogger::DataCollection MyDatalogger::getSequence(uint32_t seqNo) {
	DataCollection readDataCollection = {};
	//Serial.println("getSequence 1");
	if(myFS.takeSemaphore(fsMutexTimeout) != pdTRUE){
		Serial.println("Timeout beim warten aufs FS print");
		return {};
	} else {
		//Serial.println("getSequence 2");
		uint32_t fileindex = (seqNo)/60;
		String path = "/data/"+String(fileindex)+".zsl";
		if(!myFS.exists(path.c_str())){
			Serial.print("File not found: ");
			Serial.println(path);
			myFS.giveSemaphore(); // exit critical section
			return {};
		}
		Serial.print("File found: ");
		Serial.println(path);
		File file = myFS.open(path.c_str());
		//Serial.println("getSequence 3");
		if(!file){
			myFS.giveSemaphore(); // exit critical section
			return {};
		}
		ssize_t res = file.read((byte *)&readDataCollection, sizeof(DataCollection));
		//Serial.println("getSequence 4");
		while(res > 0){
			if(readDataCollection.seqNo == seqNo){
				break;
			}
			res = file.read((byte *)&readDataCollection, sizeof(DataCollection));
			}
		//Serial.println("getSequence 5");
		file.close();
		//Serial.println("getSequence 6");
		myFS.giveSemaphore(); // exit critical section
		//Serial.println("getSequence 7");
	}
	//Serial.println("getSequence 8");
	return readDataCollection;
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

uint32_t MyDatalogger::getCurrentSequenceNo() {
	return seqNo;
}
uint32_t MyDatalogger::getMinimumSequenceNo() {
	return minSeqNo;
}

bool MyDatalogger::deleteFile(uint32_t fileNo){
	uint32_t actFileindex = myDatalogger.getCurrentSequenceNo()/60;
	if(fileNo >= actFileindex){
		return false;
	}
	bool ret=false;
	String path = "/data/"+String(fileNo)+".zsl";
	Serial.print("Loesche Datei: ");
	Serial.println(path);
	if(myFS.takeSemaphore(fsMutexTimeout) != pdTRUE){
			Serial.println("Timeout beim warten aufs FS delete");
	} else {
		ret = myFS.remove(path.c_str());
		if(ret){
			myDatalogger.minSeqNo = UINT32_MAX;
			myDatalogger.findSequenceNo();
			}
		myFS.giveSemaphore(); // exit critical section
	}
	return ret;
}

void MyDatalogger::removeOldestLog() {
	int foundIndex = INT_MAX;
	int curIndex;
	if(myFS.takeSemaphore(fsMutexTimeout) != pdTRUE){
		Serial.println("Timeout beim warten aufs FS removeOldestLog 1");
	} else {
		File root = myFS.open("/data");
		if (!root || !root.isDirectory()) {
			Serial.println("Failed to open directory");
			return;
		}
		File file = root.openNextFile();
		while (file) {
			curIndex = atoi(file.name());
			if(curIndex < foundIndex){
				foundIndex = curIndex;
			}
			file.close();
			file = root.openNextFile();
		}
		file.close();
		root.close();
		myFS.giveSemaphore(); // exit critical section
	}

	if(foundIndex > 0){
		Serial.println("Found oldest index: "+String(foundIndex));
		deleteFile(foundIndex);
	}
}


/*
 * MyDatalogger.cpp
 *
 *  Created on: 22.07.2023
 *      Author: D-Laptop
 */
#include "MyDatalogger.h"

void MyDatalogger::setup() {
	this->seqNo = findSequenceNo();
	Serial.print("Sequenz Nummer: ");
	Serial.println(this->seqNo);
	xTaskCreate(MyDatalogger::dataLoggerTask, "dataLoggerTask", 2048, NULL, 1, NULL);
}

uint32_t MyDatalogger::findSequenceNo() {
	DataCollection tmpDataCollection;
	unit32_t seqNo=0;
	File root = myFS.open("/data");
    if (!root || !root.isDirectory()) {
        Serial.println("Failed to open directory");
        return;
    }
    File file = root.openNextFile();
    while (file) {
    	Serial.println(file.name());

    	while(file.read((byte *)&tmpDataCollection, sizeof(tmpDataCollection)) > -1){
    		if(tmpDataCollection.seqNo > seqNo){
    			seqNo = tmpDataCollection.seqNo;
    		}
    	}
    	file = root.openNextFile();
    }
    file.close();
    root.close();
    return seqNo;
}

void MyDatalogger::dataLoggerTask(void *param) {
	DataCollection dataCollection;
	while(true){
		dataCollection.seqNo = ++this->seqNo;
		for (int ii = 0; ii < 6; ++ii) {
			dataCollection.sensorData[ii] = sensor.receiveResultData();
		}
		writeDataCollection(&dataCollection);
	}
}

void MyDatalogger::writeDataCollection(DataCollection *dataCollection) {
	String path = "/data/"+String(dataCollection->seqNo%60);
	File file = myFS.open(path.c_str(), FILE_APPEND, true);
	file.write((byte *)dataCollection, sizeof(dataCollection));
	file.close();
}

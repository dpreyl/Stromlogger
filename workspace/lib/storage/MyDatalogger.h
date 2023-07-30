/*
 * MyDatalogger.h
 *
 *  Created on: 22.07.2023
 *      Author: D-Laptop
 */

#ifndef LIB_STORAGE_MYDATALOGGER_H_
#define LIB_STORAGE_MYDATALOGGER_H_

#include "INA3221.h"
#include "MyFS.h"

class MyDatalogger{
public:
	void setup();
	static void printCollectedData();
	struct DataCollection{
			INA3221::ResultData sensorData[6];
			uint32_t seqNo;
			};
	static DataCollection getSequence(uint32_t seqNo);
	uint32_t getCurrentSequenceNo();
	static bool deleteFile(uint32_t fileNo);
private:
	uint32_t findSequenceNo();
	static void dataLoggerTask(void * param);
	uint32_t seqNo=0;
	static void printDataCollection(DataCollection *dataCollection);
	static void writeDataCollection(DataCollection *dataCollection);
	DataCollection tmpDataCollection;
	static SemaphoreHandle_t fsMutex;
	static const unsigned int fsMutexTimeout = 10000;
};

extern MyDatalogger myDatalogger;

#endif /* LIB_STORAGE_MYDATALOGGER_H_ */

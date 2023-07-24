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
private:
	uint32_t findSequenceNo();
	static void dataLoggerTask(void * param);
	uint32_t seqNo=0;
	struct DataCollection{
		INA3221::ResultData sensorData[6];
		uint32_t seqNo;
		};
	static void printDataCollection(DataCollection *dataCollection);
	static void writeDataCollection(DataCollection *dataCollection);
	DataCollection tmpDataCollection;
	static SemaphoreHandle_t fsMutex;
	static const unsigned int fsMutexTimeout = 10000;
};

extern MyDatalogger myDatalogger;

#endif /* LIB_STORAGE_MYDATALOGGER_H_ */

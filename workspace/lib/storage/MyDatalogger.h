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
private:
	uint32_t findSequenceNo();
	static void dataLoggerTask(void * param);
	uint32_t seqNo=0;
	struct DataCollection{
		INA3221::ResultData sensorData[6];
		uint32_t seqNo;
		};
	void writeDataCollection(DataCollection *dataCollection);
};



#endif /* LIB_STORAGE_MYDATALOGGER_H_ */

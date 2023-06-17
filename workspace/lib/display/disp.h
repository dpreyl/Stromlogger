/*
 * disp.h
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */

#ifndef LIB_DISPLAY_DISP_H_
#define LIB_DISPLAY_DISP_H_

#define EPD_MOSI                23
#define EPD_MISO                -1
#define EPD_SCLK                18
#define EPD_CS                  5

#define EPD_BUSY                4
#define EPD_RSET                16
#define EPD_DC                  17

#include <GxEPD.h>
#include <GxDEPG0213BN/GxDEPG0213BN.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

class DispManagement{
public:
	void init();
	void dispHelloWorld();
	void printLine(char*);
	void update();
private:
	GxIO_Class io = GxIO_Class(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
	GxEPD_Class display = GxEPD_Class(io, EPD_RSET, EPD_BUSY);
};



#endif /* LIB_DISPLAY_DISP_H_ */

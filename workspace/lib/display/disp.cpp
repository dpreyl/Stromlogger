/*
 * disp.cpp
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */

#include <disp.h>

void DispManagement::init(){
	SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
	display.init(); // enable diagnostic output on Serial
}

void DispManagement::dispHelloWorld(){
	display.setRotation(1);
	display.fillScreen(GxEPD_WHITE);
	display.setTextColor(GxEPD_BLACK);
	display.setFont(&FreeMonoBold9pt7b);
	display.setCursor(0, 45);
	display.println("Hello World");
	display.update();
}

void DispManagement::printLine(char* line){
	display.println(line);
}

void DispManagement::update(){
	display.update();
}

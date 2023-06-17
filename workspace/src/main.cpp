/*
 * main.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */
#include <Arduino.h>
#include <mesh.h>

#ifdef DEVICE_TYPE_HANDHELD

#define SDCARD_CS               (13)
#define SDCARD_MOSI             (15)
#define SDCARD_MISO             (2)
#define SDCARD_SCLK             (14)

#define BUTTON_1                (39)
#define BUTTONS                 {39}

#define BUTTON_COUNT            (1)

#define LED_PIN                 (19)
#define LED_ON                  (LOW)

#define ADC_PIN                 (35)

#define _HAS_ADC_DETECTED_
#define _HAS_LED_
#define _HAS_SDCARD_


#include <disp.h>

DispManagement dispMgmt;
#endif

MeshManagement meshMgmt;

void setup() {
	Serial.begin(115200);
	meshMgmt.init();

#ifdef DEVICE_TYPE_HANDHELD
	dispMgmt.init();
	dispMgmt.dispHelloWorld();
#endif

}


void loop() {
	meshMgmt.loop();
#ifdef DEVICE_TYPE_HANDHELD
	delay(10000);
	meshMgmt.dispFoundStations(dispMgmt);
	dispMgmt.update();
#endif
}

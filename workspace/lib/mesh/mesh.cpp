/*
 * mesh.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */

#include <mesh.h>


void MeshManagement::init(){
	  // myMesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC |
	  // COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
	  myMesh.setDebugMsgTypes(ERROR | STARTUP | DEBUG);  // set before init() so that you can see startup messages

	  myMesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

	  //myMesh.onReceive(&MeshManagement::receivedCallback);

#ifdef DEVICE_TYPE_STATION
	  myMesh.initOTAReceive("messstation");
	  Serial.printf("OTA gestartet mit Name: %s\n", "messstation");
#elif DEVICE_TYPE_HANDHELD
	  myMesh.initOTAReceive("handheld");
	  Serial.printf("OTA gestartet mit Name: %s\n", "handheld");
#endif

}

void MeshManagement::dispFoundStations(DispManagement disp){
	char buffer[40];
	  auto num = WiFi.scanComplete();
	  if (num == WIFI_SCAN_FAILED) {
	    Serial.printf("wifi scan failed. Retrying....\n");
	    return;
	  } else if (num == WIFI_SCAN_RUNNING) {
		  Serial.printf("scanComplete should never be called when scan is still running.\n");
	    return;
	  }


	  for (auto i = 0; i < num; ++i) {
	    WiFi_AP_Record_t record;
	    record.ssid = WiFi.SSID(i);

	    record.rssi = WiFi.RSSI(i);
	    if (record.rssi == 0) continue;

	    sprintf(buffer, "\tfound : %s, %ddBm\n", record.ssid.c_str(),
		        (int16_t)record.rssi);

	    disp.printLine(buffer);

	    Log(CONNECTION, "\tfound : %s, %ddBm\n", record.ssid.c_str(),
	        (int16_t)record.rssi);
	  }
}

void MeshManagement::loop(){
	myMesh.update();
}


//// Needed for painless library
//void ICACHE_FLASH_ATTR MeshManagement::receivedCallback(uint32_t from, String &msg) {
//  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
//}

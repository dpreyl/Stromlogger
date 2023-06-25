/*
 * OTAHandler.cpp
 *
 *  Created on: 24.06.2023
 *      Author: D-Laptop
 */
#include "OTAHandler.h"

OTAHandler::OTAHandler(painlessMesh& mesh, MyFS& fs, const char* firmwareFolder)
    : _mesh(mesh), _fs(fs), _firmwareFolder(firmwareFolder) {
}

void OTAHandler::begin() {
#ifdef DEVICE_TYPE_HANDHELD
  _mesh.initOTAReceive("otaHandheld");
  checkForNewFirmware();
#endif
#ifdef DEVICE_TYPE_MEAS
  _mesh.initOTAReceive("otaStation");
#endif
}

void OTAHandler::loop() {
	if(noPart > 0){
	Serial.println("Offer firmware");
		//Make it known to the network that there is OTA firmware available.
		          //This will send a message every minute for an hour letting nodes know
		          //that firmware is available.
		          //This returns a task that allows you to do things on disable or more,
		          //like closing your files or whatever.
		          _mesh.offerOTA(role, hardware, md5Value, noPart, false);
	}
}

void OTAHandler::checkForNewFirmware() {
	File dir = _fs.open(_firmwareFolder);
	  while (true) {
	    File entry = dir.openNextFile();
	    if (!entry) { //End of files
	      //Serial.println("Could not find valid firmware, please validate");
	      break;
	    }

	    //This block of code parses the file name to make sure it is valid.
	    //It will also get the role and hardware the firmware is targeted at.
	    if (!entry.isDirectory()) {
	    	char name[256];
	    	  strcpy(name, entry.name());
	    	  name[sizeof(name) - 1] = '\0';

	    	  Serial.printf("Found File: %s, Var: %s\r\n", entry.name(), name);

	    	  const char* first_underscore = strchr(name, '_');
	    	  const char* last_underscore = strrchr(name, '_');
	    	  const char* dot = strchr(name, '.');

	    	  if (first_underscore && last_underscore && dot && first_underscore != last_underscore) {
	    		Serial.printf("Found underlines and dot: %i\t%i\t%i\r\n", first_underscore-name, last_underscore-name, dot-name);
	    	    char firmware[64];
	    	    strncpy(firmware, name, first_underscore - name);
	    	    firmware[first_underscore - name] = '\0';

	    	    strncpy(hardware, first_underscore + 1, last_underscore - first_underscore - 1);
	    	    hardware[last_underscore - first_underscore - 1] = '\0';

	    	    strncpy(role, last_underscore + 1, dot - last_underscore - 1);
	    	    role[dot - last_underscore - 1] = '\0';

	    	    char extension[64];
	    	    strncpy(extension, dot + 1, name + strlen(name) - dot - 1);
	    	    extension[name + strlen(name) - dot - 1] = '\0';

	    	    Serial.printf("Found Data: %s\t%s\t%s\t%s\r\n", firmware, hardware, role, extension);
	    	    if (strcmp(firmware, "firmware") == 0 &&
	    	        (strcmp(hardware, "ESP8266") == 0 || strcmp(hardware, "ESP32") == 0) &&
	    	        strcmp(extension, "bin") == 0) {


	          Serial.println("OTA FIRMWARE FOUND, NOW BROADCASTING");

	          //This is the important bit for OTA, up to now was just getting the file.
	          //If you are using some other way to upload firmware, possibly from
	          //mqtt or something, this is what needs to be changed.
	          //This function could also be changed to support OTA of multiple files
	          //at the same time, potentially through using the pkg.md5 as a key in
	          //a map to determine which to send

	          // Create a new File object for the firmware file.
	          char path[256];
	          strcpy(path, entry.path());
	          path[sizeof(path) - 1] = '\0';

	          _mesh.initOTASend(
	              [path, this](painlessmesh::plugin::ota::DataRequest pkg,
	                       char* buffer) {

	        	  Serial.printf("Sending OTA-Package No. %i\r\n", pkg.partNo);
	        	  //Serial.printf("Got Path: %s\r\n", path);
	        	  File firmwareFile = _fs.open(path, "r");
	        	  //Serial.printf("Opened: %s %s\r\n", firmwareFile.path(), firmwareFile.name());

	                //fill the buffer with the requested data packet from the node.
	        	  firmwareFile.seek(OTA_PART_SIZE * pkg.partNo);
	        	  firmwareFile.readBytes(buffer, OTA_PART_SIZE);
	        	  size_t size = firmwareFile.size();
	        	  firmwareFile.close();


	                //The buffer can hold OTA_PART_SIZE bytes, but the last packet may
	                //not be that long. Return the actual size of the packet.
	                return min((unsigned)OTA_PART_SIZE,
	                		size - (OTA_PART_SIZE * pkg.partNo));
	              },
	              OTA_PART_SIZE);

	          //Calculate the MD5 hash of the firmware we are trying to send. This will be used
	          //to validate the firmware as well as tell if a node needs this firmware.
	          MD5Builder md5;
	          md5.begin();
	          md5.addStream(entry, entry.size());
	          md5.calculate();

	          md5Value = md5.toString();
	          noPart = ceil(((float)entry.size()) / OTA_PART_SIZE);
	        }
	      }
	    }
	  }

}




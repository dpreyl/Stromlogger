/*
 * main.c
 *
 *  Created on: 17.06.2023
 *      Author: D-Laptop
 */
#include <Arduino.h>
#include <MyFS.h>
#include "mesh_handler.h"
#include "Database.h"

Scheduler userScheduler;
painlessMesh mesh;
MeshHandler meshHandler;

MyFS myFS;

#ifdef DEVICE_TYPE_HANDHELD
Database db("/sd/test2.db");

#include <DCF77TimeLib.h>
DCF77TimeLib dcf77Time;
#endif
#ifdef DEVICE_TYPE_MEAS
Database db("/littlefs/test2.db");
#endif



static int callback(void* data, int argc, char** argv, char** azColName) {
	if(argc == 2){
		Serial.printf("%s: %s\t%s: %s\r\n", azColName[0], argv[0], azColName[1], argv[1]);
	}
//    for (int i = 0; i < argc; ++i) {
//        Serial.print(azColName[i]);
//        Serial.print(": ");
//        Serial.println(argv[i] ? argv[i] : "NULL");
//    }
//    Serial.println();
    return 0;
}

static void taskCallback(){
	myFS.listFiles("/");
//	if (db.open() == SQLITE_OK) {
//		const char* select_sql = "SELECT * FROM my_table;";
//		db.execute(select_sql, callback, nullptr);
//		db.close();
//	}
#ifdef DEVICE_TYPE_HANDHELD
  Serial.printf("Unix-Timestamp: %u\r\n", dcf77Time.getUnixTimestamp());
  dcf77Time.printTime();
#endif
}

Task taskShowFilesOnSystem(10000, TASK_FOREVER, []() { taskCallback(); });

void setup() {
  Serial.begin(115200);
  myFS.begin(true);
  meshHandler.setup();

  if (db.open() == SQLITE_OK) {
          const char* create_table_sql = "CREATE TABLE IF NOT EXISTS my_table (id INTEGER PRIMARY KEY, name TEXT);";
          db.execute(create_table_sql, nullptr, nullptr);

          const char* insert_sql = "INSERT INTO my_table (name) VALUES ('John Doe');";
          db.execute(insert_sql, nullptr, nullptr);

          insert_sql = "INSERT INTO my_table (name) VALUES ('John Doe2');";
          db.execute(insert_sql, nullptr, nullptr);

          const char* select_sql = "SELECT * FROM my_table;";
          db.execute(select_sql, callback, nullptr);

          db.close();
  } else {
	  Serial.printf("SQLite Open fehlgeschlagen, code: %i\n", db.open());
  }

  userScheduler.addTask(taskShowFilesOnSystem);
  taskShowFilesOnSystem.enable();
#ifdef DEVICE_TYPE_HANDHELD
  /* der RTC-DCF benötigt ca. 1,5 Sekunden bis er Daten empfangen kann */
   delay(1500);
  dcf77Time.begin();
#endif
}

void loop() {
  meshHandler.loop();
}

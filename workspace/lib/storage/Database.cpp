/*
 * Database.cpp
 *
 *  Created on: 22.06.2023
 *      Author: D-Laptop
 */
#include "Database.h"

Database::Database(const char* filename) : filename(filename), db(nullptr) {}

Database::~Database() {
    close();
}

int Database::open() {
#ifdef DEVICE_TYPE_MEAS
	// Create the db file before trying to open it.
	if (!LittleFS.exists(filename+9)){
	  File file = LittleFS.open(filename+9, FILE_WRITE);   //  /littlefs is automatically added to the front
	  file.close();
	}
#endif

    return sqlite3_open(filename, &db);
}

int Database::close() {
    return sqlite3_close(db);
}

int Database::execute(const char* sql, int (*callback)(void*, int, char**, char**), void* data) {
    char* error_message = nullptr;
    int result = sqlite3_exec(db, sql, callback, data, &error_message);

    if (result != SQLITE_OK) {
        sqlite3_free(error_message);
    }

    return result;
}




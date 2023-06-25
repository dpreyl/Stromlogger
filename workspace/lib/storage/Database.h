/*
 * Database.h
 *
 *  Created on: 22.06.2023
 *      Author: D-Laptop
 */

#ifndef LIB_STORAGE_DATABASE_H_
#define LIB_STORAGE_DATABASE_H_

#include <sqlite3.h>
#include <LittleFS.h>

class Database {
public:
    Database(const char* filename);
    ~Database();

    int open();
    int close();
    int execute(const char* sql, int (*callback)(void*, int, char**, char**), void* data);

private:
    const char* filename;
    sqlite3* db;
};


#endif /* LIB_STORAGE_DATABASE_H_ */

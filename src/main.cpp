#include "sqlite3.h"
#include <iostream>

#define BUFFER_SIZE 32767

int main(int argc, char* argv[]) {
	if(argc < 2) {
		std::cerr << "Usage: sdedup <file> [pattern]" << std::endl;
		return 1;
	}
	FILE* file;
	sqlite3 *database;
	sqlite3_stmt* query;
	char* sqliteError = 0;
	const char* sqliteTail = 0;
	char* databaseName = "sdedup.db";
	char buffer[BUFFER_SIZE];
	file = fopen(argv[1], "r");
	if(!file) {
		std::cerr << "Unable to open file: " << argv[1] << std::endl;
		return 1;
	}
	if(sqlite3_open(databaseName, &database) != SQLITE_OK) {
		sqlite3_close(database);
		std::cerr << "Can't open sorting file: " << sqlite3_errmsg(database) << std::endl;
		return 1;
	}
	sqlite3_exec(database, "PRAGMA synchronous = OFF", NULL, NULL, &sqliteError);
	sqlite3_exec(database, "PRAGMA journal_mode = OFF", NULL, NULL, &sqliteError);
	sqlite3_exec(database, "CREATE TABLE IF NOT EXISTS file (word TEXT);", NULL, NULL, &sqliteError);
	sqlite3_prepare_v2(database, "INSERT INTO file (word) VALUES (@line);", BUFFER_SIZE, &query, &sqliteTail);
	sqlite3_exec(database, "BEGIN TRANSACTION", NULL, NULL, &sqliteError);
	while(!feof(file)) {
		fgets(buffer, BUFFER_SIZE, file);
		sqlite3_bind_text(query, 1, buffer, -1, SQLITE_TRANSIENT);
		sqlite3_step(query);
		sqlite3_reset(query);
	}
	sqlite3_finalize(query);
	sqlite3_exec(database, "END TRANSACTION", NULL, NULL, &sqliteError);
	fclose(file);
	if(sqlite3_close(database) != SQLITE_OK) {
		sqlite3_close(database);
		std::cerr << "Can't close sorting file: " << sqlite3_errmsg(database) << std::endl;
		return 1;
	}
	if(std::remove(databaseName) != 0) {
		std::cerr << "Failed to cleanup sorting file." << std::endl;
	}
	return 0;
}

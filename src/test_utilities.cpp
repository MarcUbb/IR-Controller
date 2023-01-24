#include "tests.h"

// deletes all files and directories in LittleFS
void clean_LittleFS() {
	LittleFS.begin();
	Dir dir = LittleFS.openDir("/");
	while (dir.next()) {
		String filename = dir.fileName();
		Serial.println("Deleting: " + filename);
		LittleFS.remove(filename);
	}
	LittleFS.end();
}
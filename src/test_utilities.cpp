#include "tests.h"
#include <LittleFS.h>

// recursively goes into a directory and deletes all files
void clean_LittleFS() {
	LittleFS.begin();
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    LittleFS.remove(dir.fileName());
  }
  dir = LittleFS.openDir("/programs");
  while (dir.next()) {
    LittleFS.remove("/programs/" + dir.fileName());
  }

  dir = LittleFS.openDir("/signals");
  while (dir.next()) {
    LittleFS.remove("/signals/" + dir.fileName());
  }
  LittleFS.end();
}
/**
 * @file test_utilities.cpp
 * @author Marc Ubbelohde
 * @brief This file contains functions that are used in multiple tests.
 * 
 */

#include "tests.h"

/**
 * @brief Deletes all files in "/", "/signals" and "/programs" in the LittleFS
 * 
 * @callgraph This function does not call any other function.
 * 
 * @callergraph
 */
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
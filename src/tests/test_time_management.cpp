/**
 * @file test_time_management.cpp
 * @author Marc Ubbelohde
 * @brief This file contains unit tests for all functions from the time_management.cpp.
 * 
 */


#include "tests.h"

/**
 * @brief Unit test for the function "weekday_to_num"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: -
 * -# checks if the correct number for each weekday is returned
 * -# checks if "error" is returned if the weekday is not valid
 * 
 * @see weekday_to_num
 */
boolean test_weekday_to_num() {

	// sample Strings
	String weekdays[15] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday", "bruh"};

	// expected results
	String weekdays_num[15] = {"1", "2", "3", "4", "5", "6", "0", "1", "2", "3", "4", "5", "6", "0", "error"};

	String output;

	// check if the correct number is returned
	for (int i = 0; i < 15; i++) {
		output = weekday_to_num(weekdays[i]);
		if (output != weekdays_num[i]) {
			Serial.println("\e[0;31mtest_weekday_to_num: FAILED");
			Serial.println("incorrect output for weekday: " + weekdays[i]);
			Serial.println("expected: " + String(weekdays_num[i]));
			Serial.println("actual: " + String(output) + "\e[0;37m");
			return(false);
		}
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_weekday_to_num: PASSED\e[0;37m");
	return(true);
}

/**
 * @brief Unit test for the function "compare_time"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS and initialize time.json with test data
 * -# checks with loop if true is returned when times match
 * -# checks if false is returned when times do not match
 * 
 * @see compare_time
 */
boolean test_compare_time() {

	// clean LittleFS
	clean_LittleFS();

	// test data
	String time1 = "00:00:02 0";

	DynamicJsonDocument doc(512);
	doc["hours"] = 0;
	doc["minutes"] = 0;
	doc["seconds"] = 0;
	doc["weekday"] = 0;
	doc["timezone"] = 0;
	doc["init_offset"] = millis();
	doc["last_offset"] = millis();
	doc.shrinkToFit();

	// serialize json to time.json
	LittleFS.begin();
	File file1 = LittleFS.open("/time.json", "w");
	serializeJson(doc, file1);
	file1.close();
	LittleFS.end();

	// test in loop that waits 3 seconds if value of function changes to true
	unsigned long start_time = millis();
	boolean checker = false;
	while (millis() - start_time < 3000) {
		if (compare_time(time1, true) == true) {
			checker = true;
			break;
		}
	}

	if (checker == false) {
		Serial.println("\e[0;31mtest_compare_time: FAILED");
		Serial.println("function did not return true when times matched");
		Serial.println("expected: true");
		Serial.println("actual: false\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	delay(1000);

	// test if times are compared correctly
	if (compare_time(time1, true) == true) {
		Serial.println("\e[0;31mtest_compare_time: FAILED");
		Serial.println("times were not compared correctly");
		Serial.println("expected: false");
		Serial.println("actual: true\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_compare_time: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

/**
 * @brief Unit test for the function "update_time"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS and initialize time.json with test data
 * -# checks if the time is updated correctly in Station mode
 * -# checks if the time is updated correctly in AP mode
 * 
 * @see update_time
 */
boolean test_update_time() {

	// clean LittleFS
	clean_LittleFS();

	// test data
	DynamicJsonDocument write_doc(512);
	write_doc["hours"] = 0;
	write_doc["minutes"] = 0;
	write_doc["seconds"] = 0;
	write_doc["weekday"] = 0;
	write_doc["timezone"] = 0;
	write_doc["init_offset"] = 0;
	write_doc["last_offset"] = 0;
	write_doc.shrinkToFit();

	String time = "3 19:13:30 -60";

	// serialize json to time.json
	LittleFS.begin();
	File file1 = LittleFS.open("/time.json", "w");
	serializeJson(write_doc, file1);
	file1.close();
	LittleFS.end();

	// test if the time is updated correctly in Station mode
	update_time(time, false);
	
	// read time.json
	LittleFS.begin();
	File file2 = LittleFS.open("/time.json", "r");
	DynamicJsonDocument read_doc1(512);
	deserializeJson(read_doc1, file2);
	file2.close();
	LittleFS.end();

	int hours1 = read_doc1["hours"];
	int minutes1 = read_doc1["minutes"];
	int seconds1 = read_doc1["seconds"];
	int weekday1 = read_doc1["weekday"];
	int timezone1 = read_doc1["timezone"];
	unsigned long init_offset1 = read_doc1["init_offset"];
	unsigned long last_offset1 = read_doc1["last_offset"];

	// check if only timezone was updated
	if (hours1 != 0 || minutes1 != 0 || seconds1 != 0 || weekday1 != 0 || init_offset1 != 0 || last_offset1 != 0 || timezone1 != 3600) {
		Serial.println("\e[0;31mtest_update_time: FAILED");
		Serial.println("time was updated incorrectly");
		Serial.println("expected: 00:00:00 0 3600 init_offset: 0 (about) last_offset: 0");
		Serial.println("actual: " + read_doc1.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if the time is updated correctly in AP mode
	unsigned long expected_offset = millis();
	update_time(time, true);
	
	
	// read time.json
	LittleFS.begin();
	File file3 = LittleFS.open("/time.json", "r");
	DynamicJsonDocument read_doc2(512);
	deserializeJson(read_doc2, file3);
	file3.close();
	LittleFS.end();
	

	// check if all values were updated
	int hours2 = read_doc2["hours"];
	int minutes2 = read_doc2["minutes"];
	int seconds2 = read_doc2["seconds"];
	int weekday2 = read_doc2["weekday"];
	int timezone2 = read_doc2["timezone"];
	unsigned long init_offset2 = read_doc2["init_offset"];
	unsigned long last_offset2 = read_doc2["last_offset"];
	

	if (hours2 != 19 || minutes2 != 13 || seconds2 != 30 || weekday2 != 3 || init_offset2 != last_offset2 || (last_offset2 - expected_offset) > 100 || timezone2 != 3600) {
		Serial.println("\e[0;31mtest_update_time: FAILED");
		Serial.println("time was updated incorrectly");
		Serial.println("expected: 3 19 30 0 0 0 3600");
		Serial.println("actual: " + read_doc2.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_update_time: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

/**
 * @brief Unit test for the function "get_current_time"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS and initialize time.json with test data
 * -# checks twice if time is returned correctly
 * 
 * @see get_current_time
 */
boolean test_get_current_time() {

	// clean LittleFS
	clean_LittleFS();

	// test data
	DynamicJsonDocument doc(512);
	doc["hours"] = 0;
	doc["minutes"] = 0;
	doc["seconds"] = 0;
	doc["weekday"] = 0;
	doc["timezone"] = 0;
	doc["init_offset"] = millis();
	doc["last_offset"] = millis();
	doc.shrinkToFit();

	// serialize json to time.json
	LittleFS.begin();
	File file1 = LittleFS.open("/time.json", "w");
	serializeJson(doc, file1);
	file1.close();
	LittleFS.end();

	delay(1000);

	// test if the time is returned correctly
	String output1 = get_current_time();

	if (output1 != "00:00:01 0") {
		Serial.println("\e[0;31mtest_get_current_time: FAILED");
		Serial.println("time was returned incorrectly");
		Serial.println("expected: 00:00:01 0");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	delay(2000);

	// test if the time is returned correctly
	String output2 = get_current_time();

	if (output2 != "00:00:03 0") {
		Serial.println("\e[0;31mtest_get_current_time: FAILED");
		Serial.println("time was returned incorrectly");
		Serial.println("expected: 00:00:03 0");
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_get_current_time: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

/**
 * @brief Unit test for the function "turn_seconds_in_time"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: -
 * -# checks samples of conversions
 * 
 * @see turn_seconds_in_time
 */
boolean test_turn_seconds_in_time() {

	// test data
	unsigned long seconds[15] = {0, 1, 59, 60, 61, 3599, 3600, 3601, 86399, 86400, 86401, 604799, 604800, 4294967294, 4294967295};

	// 4294967294 % 60 = 14


	// expected output
	String expected_output[15] = {"00:00:00", "00:00:01", "00:00:59", "00:01:00", "00:01:01", "00:59:59", "01:00:00", "01:00:01", "23:59:59", "24:00:00", "24:00:01", "167:59:59", "168:00:00", "1193046:28:14", "1193046:28:15"};

	String output;

	// test if the conversion is correct
	for (int i = 0; i < 15; i++) {
		output = turn_seconds_in_time(seconds[i]);
		if (output != expected_output[i]) {
			Serial.println("\e[0;31mtest_turn_seconds_in_time: FAILED");
			Serial.println("seconds: " + String(seconds[i]));
			Serial.println("expected: " + expected_output[i]);
			Serial.println("actual: " + output + "\e[0;37m");
			return(false);
		}
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_turn_seconds_in_time: PASSED\e[0;37m");
	return(true);
}

/**
 * @brief Unit test for the function "add_time"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: -
 * -# checks samples of timestamps
 * 
 * @see add_time
 */
boolean test_add_time() {

	// test data
	String time[5] = {"00:00:00 0", "00:00:01 3", "00:00:59 5", "00:01:00 4", "01:01:59 6"};
	String offset_time[15] = {"00:00:00", "00:00:01", "00:00:59", "00:01:00", "00:01:01", "00:59:59", "01:00:00", "01:00:01", "23:59:59", "24:00:00", "24:00:01", "167:59:59", "168:00:00", "1193046:59:54", "1193046:59:55"};

	// expected outputs (adds offset_time to time and adjusts the weekday if overflow occures from 0 to 6)
	String expected_output[75] = {"00:00:00 0", "00:00:01 0", "00:00:59 0", "00:01:00 0", "00:01:01 0", "00:59:59 0", "01:00:00 0", "01:00:01 0", "23:59:59 0", "00:00:00 1", "00:00:01 1", "23:59:59 6", "00:00:00 0", "06:59:54 3", "06:59:55 3", "00:00:01 3", "00:00:02 3", "00:01:00 3", "00:01:01 3", "00:01:02 3", "01:00:00 3", "01:00:01 3", "01:00:02 3", "00:00:00 4", "00:00:01 4", "00:00:02 4", "00:00:00 3", "00:00:01 3", "06:59:55 6", "06:59:56 6", "00:00:59 5", "00:01:00 5", "00:01:58 5", "00:01:59 5", "00:02:00 5", "01:00:58 5", "01:00:59 5", "01:01:00 5", "00:00:58 6", "00:00:59 6", "00:01:00 6", "00:00:58 5", "00:00:59 5", "07:00:53 1", "07:00:54 1", "00:01:00 4", "00:01:01 4", "00:01:59 4", "00:02:00 4", "00:02:01 4", "01:00:59 4", "01:01:00 4", "01:01:01 4", "00:00:59 5", "00:01:00 5", "00:01:01 5", "00:00:59 4", "00:01:00 4", "07:00:54 0", "07:00:55 0", "01:01:59 6", "01:02:00 6", "01:02:58 6", "01:02:59 6", "01:03:00 6", "02:01:58 6", "02:01:59 6", "02:02:00 6", "01:01:58 0", "01:01:59 0", "01:02:00 0", "01:01:58 6", "01:01:59 6", "08:01:53 2", "08:01:54 2"};

	String output;

	// test if the addition is correct
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 15; j++) {
			output = add_time(time[i], offset_time[j]);
			if (output != expected_output[i * 15 + j]) {
				Serial.println("\e[0;31mtest_add_time: FAILED");
				Serial.println("time: " + time[i]);
				Serial.println("offset_time: " + offset_time[j]);
				Serial.println("expected: " + expected_output[i * 15 + j]);
				Serial.println("actual: " + output + "\e[0;37m");
				return(false);
			}
		}
	}


	// print success message and return true
	Serial.println("\e[0;32mtest_add_time: PASSED\e[0;37m");
	return(true);
}

/**
 * @brief Unit test for the function "get_NTP_time"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: -
 * -# checks if the document is empty (since Wifi is not available)
 * -# functionality is tested empirically
 * 
 * @see get_NTP_time
 */
// TODO: überarbeiten
boolean test_get_NTP_time() {

	init_time();

	DynamicJsonDocument doc = load_json("/time.json");

	int hours = doc["hours"];
	int minutes = doc["minutes"];
	int seconds = doc["seconds"];
	int weekday = doc["weekday"];
	unsigned long init_offset = doc["init_offset"];
	int timezone = doc["timezone"];
	unsigned long last_offset = doc["last_offset"];

	unsigned long expected_offset = millis();

	// check if the document is empty
	if (hours != 0 || minutes != 0 || seconds != 20 || weekday != 4 || init_offset != last_offset || (expected_offset - last_offset) > 100 || timezone != 0) {
		Serial.println("\e[0;31mtest_get_NTP_time: FAILED");
		Serial.println("expected: 00:00:20 4 ");
		Serial.print("actual: ");
		serializeJson(doc, Serial);
		Serial.println();
		Serial.print("\e[0;37m\n");
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_get_NTP_time: PASSED\e[0;37m");
	return(true);
}

/**
 * @brief Unit test for the function "check_and_update_offset"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: clean LittleFS and initialize time.json with test data
 * -# check if the offset is updated correctly
 * 
 * @see check_and_update_offset
 */
boolean test_check_and_update_offset() {

	// clean LittleFS
	clean_LittleFS();

	// initialize time.json
	// test data
	DynamicJsonDocument write_doc(512);
	write_doc["hours"] = 0;
	write_doc["minutes"] = 0;
	write_doc["seconds"] = 0;
	write_doc["weekday"] = 0;
	write_doc["timezone"] = 0;
	write_doc["init_offset"] = 0;
	write_doc["last_offset"] = 0;
	write_doc.shrinkToFit();

	// serialize json to time.json
	LittleFS.begin();
	File file1 = LittleFS.open("/time.json", "w");
	serializeJson(write_doc, file1);
	file1.close();
	LittleFS.end();

	// execute function
	check_and_update_offset();

	// read time.json
	LittleFS.begin();
	File file2 = LittleFS.open("/time.json", "r");
	DynamicJsonDocument read_doc(512);
	deserializeJson(read_doc, file2);
	file2.close();
	LittleFS.end();

	// check if the offset is updated correctly (by comparing the last_offset with the current time)
	unsigned long last_offset = read_doc["last_offset"];
	if (millis() - last_offset > 1000) {
		Serial.println("\e[0;31mtest_check_and_update_offset: FAILED");
		Serial.println("expected: " + millis());
		Serial.println("actual: " + String(last_offset) + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_check_and_update_offset: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

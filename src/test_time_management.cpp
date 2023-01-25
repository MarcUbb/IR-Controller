#include "tests.h"

// tests for time_management.cpp

boolean test_weekday_to_num() {
	/*
	- checks if the correct number for each weekday is returned
	- checks if "error" is returned if the weekday is not valid
	*/

	// sample Strings
	String weekdays[15] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday", "bruh"};

	// expected results
	String weekdays_num[15] = {"1", "2", "3", "4", "5", "6", "0", "1", "2", "3", "4", "5", "6", "0", "error"};

	// check if the correct number is returned
	for (int i = 0; i < 15; i++) {
		if (weekday_to_num(weekdays[i]) != weekdays_num[i]) {
			Serial.println("\e[0;31mtest_weekday_to_num: FAILED");
			Serial.println("incorrect output for weekday: " + weekdays[i]);
			Serial.println("expected: " + String(weekdays_num[i]));
			Serial.println("actual: " + String(weekday_to_num(weekdays[i])) + "\e[0;37m");
			return(false);
		}
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_weekday_to_num: PASSED\e[0;37m");
	return(true);
}

boolean test_compare_time() {
	/*
	- checks if sample times are compared correctly
	*/

	// TODO: initialize time.json with actual millis() value
	// test with while loop if compare_time returns true after the correct amount of time
	// for case with weekday and without weekday
	return(true);
}

boolean test_update_time() {
	/*
	- checks if the time is updated correctly (format is always assumed to be correct)
	*/

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
	File file = LittleFS.open("/time.json", "w");
	serializeJson(write_doc, file);
	file.close();

	// test if the time is updated correctly in Station mode
	update_time(time, false);

	// read time.json
	DynamicJsonDocument read_doc(512);
	boolean error = deserializeJson(read_doc, LittleFS.open("/time.json", "r"));
	
	if (error) {
		Serial.println("\e[0;31mtest_update_time: FAILED");
		Serial.println("error while reading time.json");
		clean_LittleFS();
		return(false);
	}

	// check if only timezone was updated
	if (read_doc["hours"] != 0 || read_doc["minutes"] != 0 || read_doc["seconds"] != 0 || read_doc["weekday"] != 0 || read_doc["init_offset"] != 0 || read_doc["last_offset"] != 0 || read_doc["timezone"] != 3600) {
		Serial.println("\e[0;31mtest_update_time: FAILED");
		Serial.println("time was updated incorrectly");
		Serial.println("expected: 0 0 0 0 0 0 3600");
		Serial.println("actual: " + read_doc.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if the time is updated correctly in AP mode
	unsigned long expected_offset = millis();
	update_time(time, true);
	
	// read time.json
	error = deserializeJson(read_doc, LittleFS.open("/time.json", "r"));
	
	if (error) {
		Serial.println("\e[0;31mtest_update_time: FAILED");
		Serial.println("error while reading time.json");
		clean_LittleFS();
		return(false);
	}

	// check if all values were updated
	if (read_doc["hours"] != 3 || read_doc["minutes"] != 19 || read_doc["seconds"] != 30 || read_doc["weekday"] != 0 || (read_doc["init_offset"] - expected_offset) > 1000 || (read_doc["last_offset"] - expected_offset) > 1000 || read_doc["timezone"] != 3600) {
		Serial.println("\e[0;31mtest_update_time: FAILED");
		Serial.println("time was updated incorrectly");
		Serial.println("expected: 3 19 30 0 0 0 3600");
		Serial.println("actual: " + read_doc.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_update_time: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

boolean test_get_current_time() {
	/*
	- checks if time is returned correctly
	*/
	// TODO: initialize time.json with actual millis() value
	// test if correct time is returned

	return(true);
}

boolean test_turn_seconds_in_time() {
	/*
	- checks sample of conversions
	*/

	// test data
	unsigned long seconds[15] = {0, 1, 59, 60, 61, 3599, 3600, 3601, 86399, 86400, 86401, 604799, 604800, 4294967294, 4294967295};

	// expected output
	String expected_output[15] = {"00:00:00", "00:00:01", "00:00:59", "00:01:00", "00:01:01", "00:59:59", "01:00:00", "01:00:01", "23:59:59", "24:00:00", "24:00:01", "167:59:59", "168:00:00", "1193046:59:54", "1193046:59:55"};

	// test if the conversion is correct
	for (int i = 0; i < 15; i++) {
		if (turn_seconds_in_time(seconds[i]) != expected_output[i]) {
			Serial.println("\e[0;31mtest_turn_seconds_in_time: FAILED");
			Serial.println("seconds: " + String(seconds[i]));
			Serial.println("expected: " + expected_output[i]);
			Serial.println("actual: " + turn_seconds_in_time(seconds[i]) + "\e[0;37m");
			return(false);
		}
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_turn_seconds_in_time: PASSED\e[0;37m");
	return(true);
}

boolean test_add_time() {
	/*
	- checks sample of additions
	*/
	return(true);
}

boolean test_get_NTP_time() {
	/*
	- tested manually
	*/
	return(true);
}

boolean test_init_time() {
	/*
	- tested manually
	*/
	return(true);
}

boolean test_check_and_update_offset() {
	/*
	- checks if the offset is updated correctly
	*/
	return(true);
}

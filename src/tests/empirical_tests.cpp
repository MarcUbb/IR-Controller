/**
 * @file empirical_tests.cpp
 * @author Marc Ubbelohde
 * @brief collection of empirical tests
 * 
 * @details In this file you can find all emprical tests. This tests
 * are used to varify the functionality of parts of the code that are not easily
 * testable with common unit tests because they rely on internet connection or
 * other external factors.
 * 
 */

#include "tests.h"

/**
 * @brief Tests empirically the function get_NTP_time()
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: This test connects to the mobile hotspot of a phone. (didn't want to write my wifi credentials here)\n 
 * -# request NTP time and check if the init_offset is correct (error should be less than 1000ms)
 * -# request NTP time 100 times after random time intervals and check if the time is equal to the expected time 
 * (error should be less than 1000ms due to rounding errors)
 * 
 */

// TODO: überarbeiten
boolean empirical_test_get_NTP_time() {
	/*
	- tests empirically if the NTP time is correct
	*/

	// connect to mobile Hotspot
	WiFi.mode(WIFI_STA);
	WiFi.begin("Honor 6X", "12345678");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	
	unsigned long start_time = millis();

	// get reference time (checked manually):
	init_time();

	DynamicJsonDocument current_doc = load_json("/time.json");

	// print current_doc
	serializeJson(current_doc, Serial);
	Serial.println();
	
	// check init_offset
	unsigned long init_offset = current_doc["init_offset"];

	if (init_offset - start_time > 1000) {
		Serial.println("\e[0;31memprical_test_get_NTP_time: FAILED");
		Serial.println("expected: init_offset - start_time < 1000");
		Serial.print("actual: ");
		Serial.print(init_offset - start_time);
		Serial.print("\e[0;37m\n");
		return(false);
	}

	DynamicJsonDocument previous_doc(1024);

	// check 100 times if the time is correct
	for (int i = 0; i < 100; i++) {
		// wait random time between checks from 5s to 10s (problems occured when frequency was below or at 3 request per second)
		unsigned long random_time = random(5000, 10000);
		delay(random_time);
		unsigned long start_time = millis();

		previous_doc = current_doc;
		// get NTP time
		init_time();
		unsigned long execution_time = millis() - start_time;

		current_doc = load_json("/time.json");

		// retrieve data from json's
		int current_hours = current_doc["hours"];
		int current_minutes = current_doc["minutes"];
		int current_seconds = current_doc["seconds"];
		int current_weekday = current_doc["weekday"];
		int current_timezone = current_doc["timezone"];
		unsigned long current_last_offset = current_doc["last_offset"];
		unsigned long current_init_offset = current_doc["init_offset"];

		int previous_hours = previous_doc["hours"];
		int previous_minutes = previous_doc["minutes"];
		int previous_seconds = previous_doc["seconds"];
		int previous_weekday = previous_doc["weekday"];
		int previous_timezone = previous_doc["timezone"];
		unsigned long previous_last_offset = previous_doc["last_offset"];
		unsigned long previous_init_offset = previous_doc["init_offset"];

		// check if data is correct (change of day not respected because its in my control)
		if (current_timezone != previous_timezone || current_weekday != previous_weekday || (current_init_offset - previous_init_offset) > (500 + random_time + execution_time) || (current_last_offset - previous_last_offset) > (500 + random_time + execution_time)) {
			Serial.println("\e[0;31memprical_test_get_NTP_time: FAILED");
			Serial.println("expected: timezone and weekday are the same and init_offset and last_offset are increasing by max 500ms + random_time");
			Serial.print("actual: timezones:" + String(current_timezone) + " != " + String(previous_timezone));
			Serial.println("weekdays: " + String(current_weekday) + " != " + String(previous_weekday));
			Serial.println("random_time: " + String(random_time));
			Serial.println("current_init_offset: " + String(current_init_offset));
			Serial.println("previous_init_offset: " + String(previous_init_offset));
			Serial.println("current_last_offset: " + String(current_last_offset));
			Serial.println("previous_last_offset: " + String(previous_last_offset));
			Serial.println();
			Serial.print("\e[0;37m\n");
			return(false);
		}

		// calculate expected values for hours, minutes and seconds
		int expected_hours = previous_hours;
		int expected_minutes = previous_minutes;
		int expected_seconds = previous_seconds + ((millis() - previous_init_offset))/1000;

		if (expected_seconds >= 60) {
			expected_minutes++;
			expected_seconds -= 60;
		}
		if (expected_minutes >= 60) {
			expected_hours++;
			expected_minutes -= 60;
		}

		// check if data is correct (1 second error is accepted because of rounding errors)
		if (current_hours != expected_hours || current_minutes != expected_minutes || (current_seconds != expected_seconds && current_seconds != expected_seconds + 1 && current_seconds != expected_seconds - 1 && current_seconds != expected_seconds + 59 && current_seconds != expected_seconds - 59)) {
			Serial.println("\e[0;31memprical_test_get_NTP_time: FAILED");
			Serial.println("expected: hours, minutes and seconds are increasing by random_time");
			Serial.print("actual: hours: " + String(current_hours) + " != " + String(expected_hours));
			Serial.print(" minutes: " + String(current_minutes) + " != " + String(expected_minutes));
			Serial.print(" seconds: " + String(current_seconds) + " != " + String(expected_seconds));
			Serial.print("\e[0;37m\n");
			return(false);
		}
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_get_NTP_time: PASSED\e[0;37m");
	return(true);
}
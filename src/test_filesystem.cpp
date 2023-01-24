#include "tests.h"

// tests for filesystem.cpp


boolean test_capture_signal() {
  /*
  - checks if execution time is normal
  - checks if return value is correct (no random noise signal is received)
  */
  unsigned long start_time = millis();
  String return_val = capture_signal();
  unsigned long end_time = millis();

  unsigned long elapsed_time = end_time - start_time;

	// checks if return value is correct and if execution time is normal
  if (return_val == "no_signal" && elapsed_time > 10000 && elapsed_time < 11000) {
    Serial.println("\e[0;32mtest_capture_signal: PASSED\e[0;37m");
    return true; 
  }
  // prints error message if test failed
  else {
    Serial.println("\e[0;31mtest_capture_signal: FAILED");
    Serial.println("return value: " + return_val + " , elapsed time: " + elapsed_time + "\e[0;37m");
    return false;
  }
}

boolean test_save_signal() {
  /*
  - checks if formatting of result_string is considered
  - checks if formatting of name is considered
  */
  
  String name1 = "___test123"; // should work
  String name2 = "___üäöß"; // should not work
  String name3 = "___/"; // should not work
  String name4 = "___ "; // should not work
  String name5 = "________10________20________30________40"; // should not work
  String name6 = ""; // should not work

  String result_string1 = "uint16_t rawData[3] = {1234, 5678, 412};"; // should work
  String result_string2 = "uint16_t rawData[4] = {1234, 5678, 412};"; // should work (error check in sending)
  String result_string3 = "abc [] {}"; // should not work
  String result_string4 = ""; // should not work
  String result_string5 = "uint16_t rawData[3 = {1234, 5678, 412};"; // should not work
  String result_string6 = "uint16_t rawData 3] = {1234, 5678, 412}"; // should not work
  String result_string7 = "uint16_t rawData][3] = {1234, 5678, 412}"; // should not work
  String result_string8 = "uint16_t rawData[3] = 1234, 5678, 412}"; // should not work
  String result_string9 = "uint16_t rawData[3] = {1234, 5678,"; // should not works
  String result_string10 = "uint16_t rawData[3] = }{1234, 5678, 412};"; // should not work

	// create array of names
	String names[6] = {name1, name2, name3, name4, name5, name6};

	// create array of result_strings
	String result_strings[10] = {result_string1, result_string2, result_string3, result_string4, result_string5, result_string6, result_string7, result_string8, result_string9, result_string10};

	// create array of expected return values for names
	String expected_return_values_names[6] = {"success", "Error: name is not alphanumeric", "Error: name is not alphanumeric", "Error: name is not alphanumeric", "Error: name exceeds 32 characters", "Error: name is empthy"};
	
	// create array of expected return values for result_strings (where success = "success", error with [length] = "Error: could not extract length from String" and error with {sequence} = "Error: could not extract sequence from String")
	String expected_return_values_result_strings[10] = {"success", "success", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract sequence from String", "Error: could not extract sequence from String", "Error: could not extract sequence from String"};

	// loops through all names with result_string1 and checks if return value is correct
	for (int i = 0; i < 6; i++) {
		String return_value = save_signal(names[i], result_string1);
		if (return_value != expected_return_values_names[i]) {
			Serial.println("\e[0;31mtest_save_signal: FAILED");
			Serial.println("name: " + names[i] + " , result_string: " + result_string1 + " , expected return value: " + expected_return_values_names[i] + " , actual return value: " + return_value + "\e[0;37m");
			return false;
		}
	}

	// loops through all result_strings with name1 and checks if return value is correct
	for (int i = 0; i < 10; i++) {
		String return_value = save_signal(name1, result_strings[i]);
		if (return_value != expected_return_values_result_strings[i]) {
			Serial.println("\e[0;31mtest_save_signal: FAILED");
			Serial.println("name: " + name1 + " , result_string: " + result_strings[i] + " , expected return value: " + expected_return_values_result_strings[i] + " , actual return value: " + return_value + "\e[0;37m");
			return false;
		}
	}

	//prints success message and return true
	Serial.println("\e[0;32mtest_save_signal: PASSED\e[0;37m");
	return true;
}

boolean test_save_json() {
	/*
	- checks if file is created
	- checks if JSON-Document is correctly written to file
	*/

}


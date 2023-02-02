/**
 * @file test_filesystem.cpp
 * @author Marc Ubbelohde
 * @brief This file contains unit tests for all functions from the filesystem.cpp.
 * 
 */


#include "tests.h"

/**
 * @brief Unit test for the function "capture_signal"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: -
 * -# check if return value is correct (no random noise signal is received)
 * -# check if execution time is normal (1s more is acceptable)
 * 
 * @see capture_signal
 */
boolean test_capture_signal() {

  unsigned long start_time = millis();
  String return_val = capture_signal();
  unsigned long end_time = millis();

  unsigned long elapsed_time = end_time - start_time;

	// checks if return value is correct and if execution time is normal
  if (return_val != "no_signal" || elapsed_time < 10000 || elapsed_time > 11000) {
    Serial.println("\e[0;31mtest_capture_signal: FAILED");
    Serial.println("return value: " + return_val + " , elapsed time: " + elapsed_time + "\e[0;37m");
    return(false);	
  }

  // prints success message and returns true
  Serial.println("\e[0;32mtest_capture_signal: PASSED\e[0;37m");
  return(true); 
}

/**
 * @brief Unit test for the function "save_signal"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS
 * -# checks if formatting of name is considered
 * -# checks if formatting of result_string is considered
 * 
 * @see save_signal
 */
boolean test_save_signal() {

	// Clean LittleFS
	clean_LittleFS();
  
  String name1 = "___test123"; // should work
  String name2 = "___üäöß"; // should not work
  String name3 = "___/"; // should not work
  String name4 = "___ "; // should work
  String name5 = "________10________20__"; // should not work
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
	String expected_return_values_names[6] = {"success", "Error: name is not alphanumeric", "Error: name is not alphanumeric", "success", "Error: name exceeds 32 characters", "Error: name is empthy"};
	
	// create array of expected return values for result_strings (where success = "success", error with [length] = "Error: could not extract length from String" and error with {sequence} = "Error: could not extract sequence from String")
	String expected_return_values_result_strings[10] = {"success", "success", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract length from String", "Error: could not extract sequence from String", "Error: could not extract sequence from String", "Error: could not extract sequence from String"};

	// loops through all names with result_string1 and checks if return value is correct
	for (int i = 0; i < 6; i++) {
		String return_value = save_signal(result_string1, names[i]);
		if (return_value != expected_return_values_names[i]) {
			Serial.println("\e[0;31mtest_save_signal: FAILED");
			Serial.println("name: " + names[i] + " , result_string: " + result_string1 + " , expected return value: " + expected_return_values_names[i] + " , actual return value: " + return_value + "\e[0;37m");
      clean_LittleFS();
			return(false);
		}
	}

	// loops through all result_strings with name1 and checks if return value is correct
	for (int i = 0; i < 10; i++) {
		String return_value = save_signal(result_strings[i], name1);
		if (return_value != expected_return_values_result_strings[i]) {
			Serial.println("\e[0;31mtest_save_signal: FAILED");
			Serial.println("name: " + name1 + " , result_string: " + result_strings[i] + " , expected return value: " + expected_return_values_result_strings[i] + " , actual return value: " + return_value + "\e[0;37m");
      clean_LittleFS();
			return(false);
		}
	}

	// prints success message and return true
	Serial.println("\e[0;32mtest_save_signal: PASSED\e[0;37m");
  clean_LittleFS();
	return(true);
}

/**
 * @brief Unit test for the function "save_json"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS
 * -# checks if file is created
 * -# checks if JSON-Document is correctly written to file
 * -# checks if file is overwritten if it already exists
 * - (no check if filename or data is correct (this is checked by higher level))
 * 
 * @see save_json
 */
boolean test_save_json() {

	// Clean LittleFS
	clean_LittleFS();

  // test names
  String name1 = "/test_file.json"; 

  // test JSON-Documents
  DynamicJsonDocument doc1(512);
  doc1["length"] = 3;
  doc1["sequence"] = "1234, 5678, 412";
  doc1.shrinkToFit();

	DynamicJsonDocument doc2(512);
  doc2["length"] = 4;
  doc2["sequence"] = "1234, 5678, 412, 123";
  doc2.shrinkToFit();

  // tests if file is created
  save_json(name1, doc1);
	LittleFS.begin();

  if (!LittleFS.exists(name1)) {
    Serial.println("\e[0;31mtest_save_json: FAILED");
    Serial.println("file " + name1 + " was not created\e[0;37m");
		LittleFS.end();
    clean_LittleFS();
    return(false);
  }

  // test if file is correctly written
  File file = LittleFS.open(name1, "r");
  String file_content = file.readString();
  file.close();

  if (file_content != "{\"length\":3,\"sequence\":\"1234, 5678, 412\"}") {
    Serial.println("\e[0;31mtest_save_json: FAILED");
    Serial.println("file " + name1 + " was not written correctly");
    Serial.println("expected: {\"length\":3,\"sequence\":\"1234, 5678, 412\"}");
    Serial.println("actual: " + file_content + "\e[0;37m");
		LittleFS.end();
    clean_LittleFS();
    return(false);
  }

	LittleFS.end();

  // test if file is overwritten
  save_json(name1, doc2);

	LittleFS.begin();
  file = LittleFS.open(name1, "r");
  String file_content2 = file.readString();
  file.close();

  if (file_content2 != "{\"length\":4,\"sequence\":\"1234, 5678, 412, 123\"}") {
    Serial.println("\e[0;31mtest_save_json: FAILED");
    Serial.println("file " + name1 + " was not overwritten correctly");
    Serial.println("expected: {\"length\":4,\"sequence\":\"1234, 5678, 412, 123\"}");
    Serial.println("actual: " + file_content2 + "\e[0;37m");
		LittleFS.end();
    clean_LittleFS();
    return(false);
  }

  // prints success message and returns true
  Serial.println("\e[0;32mtest_save_json: PASSED\e[0;37m");
	LittleFS.end();
  clean_LittleFS();
  return(true);
}

/**
 * @brief Unit test for the function "load_json"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS
 * -# checks if data is correctly loaded from file
 * -# checks if empthy JSON Doc is returned if file does not exist
 * -# checks if empthy JSON Doc is returned if file is empthy
 * -# checks if empthy JSON Doc is returned if file is not in JSON format
 * 
 * @see load_json
 */
boolean test_load_json() {

	// Clean LittleFS
	clean_LittleFS();

 	// start LittleFS
	LittleFS.begin();

  // test names
	String name1 = "/test_file";
	String name2 = "/empthy_file";
	String name3 = "/non_json_file";

	// test JSON-Document
	DynamicJsonDocument doc1(512);
	doc1["length"] = 3;
	doc1["sequence"] = "1234, 5678, 412";
	doc1.shrinkToFit();

	// non json test String
	String test_string = "abc";

	// write test data to file
	File file = LittleFS.open(name1, "w");
	serializeJson(doc1, file);
	file.close();

	file = LittleFS.open(name3, "w");
	file.print(test_string);
	file.close();
	LittleFS.end();

	// test if data is correctly loaded from file
	DynamicJsonDocument doc2 = load_json(name1);
	if (doc2["length"] != 3 || doc2["sequence"] != "1234, 5678, 412") {
		Serial.println("\e[0;31mtest_load_json: FAILED");
		Serial.println("file " + name2 + " was not loaded correctly");
		Serial.println("expected: {\"length\":3,\"sequence\":\"1234, 5678, 412\"}");
		Serial.println("actual: " + doc2.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if empthy JSON Doc is returned if file does not exist
	DynamicJsonDocument doc3 = load_json("abc");
	if (doc3.size() != 0) {
		Serial.println("\e[0;31mtest_load_json: FAILED");
		Serial.println("empthy JSON Doc was not returned if file does not exist");
		Serial.println("expected: {}");
		Serial.println("actual: " + doc3.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if empthy JSON Doc is returned if file is empthy
	DynamicJsonDocument doc4 = load_json(name2);
	if (doc4.size() != 0) {
		Serial.println("\e[0;31mtest_load_json: FAILED");
		Serial.println("empthy JSON Doc was not returned if file is empthy");
		Serial.println("expected: {}");
		Serial.println("actual: " + doc4.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if empthy JSON Doc is returned if file is not in JSON format
	DynamicJsonDocument doc5 = load_json(name3);
	if (doc5.size() != 0) {
		Serial.println("\e[0;31mtest_load_json: FAILED");
		Serial.println("empthy JSON Doc was not returned if file is not in JSON format");
		Serial.println("expected: {}");
		Serial.println("actual: " + doc5.as<String>() + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// prints success message and returns true
	Serial.println("\e[0;32mtest_load_json: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

/**
 * @brief Unit test for the function "send_signal"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: -
 * -# checks if signal is sent correctly
 * -# checks if JSON Doc with invalid length is not accepted
 * -# checks if JSON Doc with without sequence is not accepted
 * -# checks if JSON Doc with without length is not accepted
 * 
 * @see send_signal
 */
boolean test_send_signal() {

  // test Json-Documents
	DynamicJsonDocument doc1(512);
	doc1["length"] = 3;
	doc1["sequence"] = "1234, 5678, 412";
	doc1.shrinkToFit();

	DynamicJsonDocument doc2(512);
	doc2["length"] = 3;
	doc2["sequence"] = "1234, 5678, 412, 123";
	doc2.shrinkToFit();

	DynamicJsonDocument doc3(512);
	doc3["length"] = 3;
	doc3.shrinkToFit();

	DynamicJsonDocument doc4(512);
	doc4["sequence"] = "1234, 5678, 412";
	doc4.shrinkToFit();

	// test if JSON Doc can be send
	String output1 = send_signal(doc1);
	if (output1 != "success") {
		Serial.println("\e[0;31mtest_send_signal: FAILED");
		Serial.println("JSON Doc was not send correctly");
		Serial.println("expected: success");
		Serial.println("actual: " + output1 + "\e[0;37m");
		return(false);
	}

	// test if JSON Doc with invalid length is not accepted
	String output2 = send_signal(doc2);
	if (output2 != "Error: length of sequence does not match length in JSON document! Please save signal again.") {
		Serial.println("\e[0;31mtest_send_signal: FAILED");
		Serial.println("JSON Doc with invalid length was accepted");
		Serial.println("expected: Error: length of sequence does not match length in JSON document! Please save signal again.");
		Serial.println("actual: " + output2 + "\e[0;37m");
		return(false);
	}

	// test if JSON Doc without sequence is not accepted
	String output3 = send_signal(doc3);
	if (output3 != "Error: invalid signal") {
		Serial.println("\e[0;31mtest_send_signal: FAILED");
		Serial.println("JSON Doc without sequence was handled incorrectly");
		Serial.println("expected: Error: invalid signal");
		Serial.println("actual: " + output3 + "\e[0;37m");
		return(false);
	}

	// test if JSON Doc without length is not accepted
	String output4 = send_signal(doc4);
	if (output4 != "Error: invalid signal") {
		Serial.println("\e[0;31mtest_send_signal: FAILED");
		Serial.println("JSON Doc without length was handled incorrectly");
		Serial.println("expected: Error: invalid signal");
		Serial.println("actual: " + output4 + "\e[0;37m");
		return(false);
	}

	// prints success message and returns true
	Serial.println("\e[0;32mtest_send_signal: PASSED\e[0;37m");
	return(true);
}

/**
 * @brief Unit test for the function "get_files"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS and create test files in LittleFS
 * -# checks if files are returned correctly
 * -# checks if no files are returned if no files exist
 * 
 * @see get_files
 */
boolean test_get_files() {

  // clear LittleFS
	clean_LittleFS();

	// create files
	LittleFS.begin();
	File file1 = LittleFS.open("/signals/test1.json", "w");
	file1.close();
	File file2 = LittleFS.open("/signals/test2.json", "w");
	file2.close();
	File file3 = LittleFS.open("/programs/test3.json", "w");
	file3.close();
	LittleFS.end();

	// test if files are returned correctly
	String files1 = get_files("signals", "programs");

	if (files1 != "test1,test2;test3") {
		Serial.println("\e[0;31mtest_get_files: FAILED");
		Serial.println("files were not returned correctly");
		Serial.println("expected: test1,test2;test3");
		Serial.println("actual: " + files1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if no files are returned if no files exist
	String files2 = get_files("abc", "def");

	if (files2 != ";") {
		Serial.println("\e[0;31mtest_get_files: FAILED");
		Serial.println("no files were returned if no files exist");
		Serial.println("expected: ;");
		Serial.println("actual: " + files2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// prints success message and returns true
	Serial.println("\e[0;32mtest_get_files: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

/**
 * @brief Unit test for the function "check_if_file_exists"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS and create test file in LittleFS
 * -# checks existing file is found
 * -# checks non-existing file is not found
 * 
 * @see check_if_file_exists
 */
boolean test_check_if_file_exists() {

 	// clear LittleFS
	clean_LittleFS();

 	// create test file
	LittleFS.begin();
	File file = LittleFS.open("test.json", "w");
	file.close();
	LittleFS.end();

	// test if existing file is found
	boolean output1 = check_if_file_exists("test.json");
	if (output1 != true) {
		Serial.println("\e[0;31mtest_check_if_file_exists: FAILED");
		Serial.println("existing file was not found");
		Serial.println("expected: true");
		Serial.println("actual: " + String(output1) + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if non-existing file is not found
	boolean output2 = check_if_file_exists("test2.json");
	if (output2 != false) {
		Serial.println("\e[0;31mtest_check_if_file_exists: FAILED");
		Serial.println("non-existing file was found");
		Serial.println("expected: false");
		Serial.println("actual: " + String(output2) + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// prints success message and returns true
	Serial.println("\e[0;32mtest_check_if_file_exists: PASSED\e[0;37m");
	clean_LittleFS();
  return(true);
}

/**
 * @brief Unit test for the function "read_program"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details - Setup: Clean LittleFS and create test program in LittleFS
 * -# checks if program is read correctly
 * -# checks if empthy string is returned if program does not exist
 * 
 * @see read_program
 */
boolean test_read_program() {

 	// clean LittleFS
	clean_LittleFS();

	// create test file
	LittleFS.begin();
	File file = LittleFS.open("/programs/test.json", "w");
	file.println(String("play abc\nwait 500\nplay def\nloop 3\nplay ghi\nwait 100\nend").c_str());
	file.close();
	LittleFS.end();

	// test if program is read correctly
	String program1 = read_program("test");

	if (program1 != "play abc\nwait 500\nplay def\nloop 3\nplay ghi\nwait 100\nend") {
		Serial.println("\e[0;31mtest_read_program: FAILED");
		Serial.println("program was not read correctly");
		Serial.println("expected: play abc\nwait 500\nplay def\nloop 3\nplay ghi\nwait 100\nend");
		Serial.println("actual: " + program1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// test if error is returned if program does not exist
	String program2 = read_program("test2");

	if (program2 != "") {
		Serial.println("\e[0;31mtest_read_program: FAILED");
		Serial.println("error was not returned if program does not exist");
		Serial.println("expected: ");
		Serial.println("actual: " + program2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// prints success message and returns true
	Serial.println("\e[0;32mtest_read_program: PASSED\e[0;37m");
	clean_LittleFS();
  return(true);
}

/**
 * @brief Unit test for the function "test_control_led_output"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details The functionality of this function is manually tested.
 * 
 * @see test_control_led_output
 */
boolean test_control_led_output() {
  return(true);
}

/**
 * @brief Unit test for the function "test_check_if_string_is_alphanumeric"
 * 
 * @return boolean - true if the test passed, false if the test failed
 * 
 * @details -Setup: -
 * -# checks if sample Strings are recognized correctly
 * 
 * @see test_check_if_string_is_alphanumeric
 */
boolean test_check_if_string_is_alphanumeric() {

  // sample Strings
	String string1 = "abc"; //should return true
	String string2 = "abc123"; //should return true
	String string3 = "abc123!"; //should return false
	String string4 = "abc123!@#"; //should return false
	String string5 = "_ -"; //should return true

	// collect samples in array
	String samples[5] = {string1, string2, string3, string4, string5};

	// expected return values
	boolean expected[5] = {true, true, false, false, true};

	boolean output;
	// test if sample Strings are recognized correctly
	for (int i = 0; i < 5; i++) {
		output = check_if_string_is_alphanumeric(samples[i]);
		if (output != expected[i]) {
			Serial.println("\e[0;31mtest_check_if_string_is_alphanumeric: FAILED");
			Serial.println("incorrect return value for sample String" + samples[i]);
			Serial.println("expected: " + String(expected[i]));
			Serial.println("actual: " + String(output) + "\e[0;37m");
			return(false);
		}
	}

	// prints success message and returns true
	Serial.println("\e[0;32mtest_check_if_string_is_alphanumeric: PASSED\e[0;37m");
  return(true);
}
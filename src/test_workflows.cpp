#include "tests.h"

// tests for workflow.cpp

boolean test_deleding_workflow() {
	/*
	- checks if file is deleted correctly
	- checks if error message is correct when file does not exist
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test file
	File file = LittleFS.open("/signals/test_signal.json", "w");
	file.close();

	// create test data
	String test_directory = "signals";
	String name_fake = "test_signal2";
	String name = "test_signal";

	String output1;

	// tests if error message is correct when file does not exist
	output1 = deleting_workflow(test_directory, name_fake);
	if (output1 != "could not find " + test_directory + ": " + name_fake) {
		Serial.println("\e[0;31mtest_deleting_workflow: FAILED");
		Serial.println("function did not return correct error message when file does not exist");
		Serial.println("expected: could not find " + test_directory + ": " + name_fake);
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if no other file is deleted
	if (LittleFS.exists("/signals/test_signal.json") == false) {
		Serial.println("\e[0;31mtest_deleting_workflow: FAILED");
		Serial.println("function deleted wrong file");
		Serial.println("expected: could not find " + test_directory + ": " + name_fake);
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if file is deleted correctly
	String output2 = deleting_workflow(test_directory, name);
	if (output2 != "deleted " + test_directory + ": " + name) {
		Serial.println("\e[0;31mtest_deleting_workflow: FAILED");
		Serial.println("function did not return correct message when file was deleted");
		Serial.println("expected: deleted " + test_directory + ": " + name);
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_deleting_workflow: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

boolean test_recording_workflow() {
	/*
	- checks if error message is correct when nothing was recorded
	*/
	return(true);
}

boolean test_sending_workflow() {
	/*
	- check if error message is correct when file does not exist
	- check if sequence can be correctly sent
	- check if error messages of send_signal are shown correctly (1 example)
	*/
	return(true);
}

boolean test_adding_workflow() {
	/*
	- checks if program code is correctly written to file
	*/
	return(true);
}

boolean test_playing_workflow() {
	/*
	- check if error message is correct when file does not exist
	- check if program can be correctly executed
	- check if error messages of program_parser are shown correctly (1 example)
	*/
	return(true);
}

boolean test_program_parser() {
	/*
	- check every error message of program_parser and called functions
	- check if program can be executed correctly
	*/
	return(true);
}

boolean test_handle_wait_command() {
	/*
	- check if set amount of time is waited
	*/
	return(true);
}

boolean test_handle_times_commands() {
	/*
	- checks error message if weekday is invalid
	- checks if invalid commands are caught
	*/
	return(true);
}
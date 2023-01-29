#include "tests.h"

// tests for workflow.cpp

boolean test_deleding_workflow() {
	/*
	- checks if file is deleted correctly
	- checks if error message is correct when file does not exist
	- checks if no other file is deleted
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test file
	LittleFS.begin();
	File file = LittleFS.open("/signals/test_signal.json", "w");
	file.close();
	LittleFS.end();

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

	LittleFS.begin();
	// tests if no other file is deleted
	if (LittleFS.exists("/signals/test_signal.json") == false) {
		Serial.println("\e[0;31mtest_deleting_workflow: FAILED");
		Serial.println("function deleted wrong file");
		Serial.println("expected: could not find " + test_directory + ": " + name_fake);
		Serial.println("actual: " + output1 + "\e[0;37m");
		LittleFS.end();
		clean_LittleFS();
		return(false);
	}

	// tests if file is deleted correctly
	String output2 = deleting_workflow(test_directory, name);
	if (output2 != "successfully deleted " + test_directory + ": " + name) {
		Serial.println("\e[0;31mtest_deleting_workflow: FAILED");
		Serial.println("function did not return correct message when file was deleted");
		Serial.println("expected: deleted " + test_directory + ": " + name);
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_deleting_workflow: PASSED\e[0;37m");
	LittleFS.end();
	clean_LittleFS();
	return(true);
}

boolean test_recording_workflow() {
	/*
	- checks if error message is correct when nothing was recorded
	- checks if no file is written when nothing was recorded
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test data
	String test_name1 = "test_signal";

	// tests if error message is correct when nothing was recorded
	String output1 = recording_workflow(test_name1);
	if (output1 != "failed to record signal") {
		Serial.println("\e[0;31mtest_recording_workflow: FAILED");
		Serial.println("function did not return correct error message when nothing was recorded");
		Serial.println("expected: failed to record signal");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	LittleFS.begin();
	// tests if no file is written when nothing was recorded
	if (LittleFS.exists("/signals/test_signal.json") == true) {
		Serial.println("\e[0;31mtest_recording_workflow: FAILED");
		Serial.println("function wrote file when nothing was recorded");
		Serial.println("expected: failed to record signal");
		Serial.println("actual: " + output1 + "\e[0;37m");
		LittleFS.end();
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_recording_workflow: PASSED\e[0;37m");
	LittleFS.end();
	clean_LittleFS();
	return(true);
}

boolean test_sending_workflow() {
	/*
	- check if error message is correct when file does not exist
	- check if sequence can be correctly sent
	- check if error messages of send_signal are shown correctly (1 example)
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test data
	LittleFS.begin();
	File file1 = LittleFS.open("/signals/test_signal.json", "w");
	DynamicJsonDocument doc1(512);
	doc1["length"] = 3;
	doc1["sequence"] = "1234, 5678, 412";
	doc1.shrinkToFit();
	serializeJson(doc1, file1);
	file1.close();

	File file2 = LittleFS.open("/signals/test_signal2.json", "w");
	DynamicJsonDocument doc2(512);
	doc2["length"] = 3;
	doc2["sequence"] = "1234, 5678, 412, 123";
	doc2.shrinkToFit();
	serializeJson(doc2, file2);
	file2.close();
	LittleFS.end();

	// tests if error message is correct when file does not exist
	String output1 = sending_workflow("test_signal3");
	if (output1 != "could not find signal: test_signal3") {
		Serial.println("\e[0;31mtest_sending_workflow: FAILED");
		Serial.println("function did not return correct error message when file does not exist");
		Serial.println("expected: could not find signal: test_signal3");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if sequence can be correctly sent
	String output2 = sending_workflow("test_signal");
	if (output2 != "successfully sent signal: test_signal") {
		Serial.println("\e[0;31mtest_sending_workflow: FAILED");
		Serial.println("function did not return correct message when sequence was sent");
		Serial.println("expected: successfully sent signal: test_signal");
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if error messages of send_signal are shown correctly
	String output3 = sending_workflow("test_signal2");
	if (output3 != "Error: length of sequence does not match length in JSON document! Please save signal again.") {
		Serial.println("\e[0;31mtest_sending_workflow: FAILED");
		Serial.println("function did not return correct error message when sequence could not be sent");
		Serial.println("expected: Error: length of sequence does not match length in JSON document! Please save signal again.");
		Serial.println("actual: " + output3 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_sending_workflow: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

boolean test_adding_workflow() {
	/*
	- checks if functions return value is correct
	- checks if program code is correctly written to file
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test data
	String test_name1 = "test_program";
	String test_code1 = "test_code";

	// tests if functions return value is correct
	String output1 = adding_workflow(test_name1, test_code1);
	if (output1 != "successfully saved program: test_program") {
		Serial.println("\e[0;31mtest_adding_workflow: FAILED");
		Serial.println("function did not return correct message when program was added");
		Serial.println("expected: successfully saved program: test_program");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if program code is correctly written to file
	LittleFS.begin();
	File file = LittleFS.open("/programs/test_program.json", "r");
	String output2 = file.readString();
	file.close();
	LittleFS.end();

	if (output2 != test_code1) {
		Serial.println("\e[0;31mtest_adding_workflow: FAILED");
		Serial.println("function did not write correct program code to file");
		Serial.println("expected: test_code");
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_adding_workflow: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

boolean test_playing_workflow() {
	/*
	- check if error message is correct when file does not exist
	- check if program can be correctly executed
	- check if error messages of program_parser are shown correctly (1 example)
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test data
	String program_name1 = "test_program";	// correct program
	String program_name2 = "test_program2";	// faulty program
	String program_name3 = "test_program3";	// program that does not exist

	// correct program
	LittleFS.begin();
	File file1 = LittleFS.open("/programs/test_program.json", "w");
	file1.println("wait 500");
	file1.close();

	// faulty program
	File file2 = LittleFS.open("/programs/test_program2.json", "w");
	file2.println("play abc");
	file2.close();
	LittleFS.end();

	// tests if error message is correct when file does not exist
	String output1 = playing_workflow(program_name3);

	if (output1 != "could not find program: test_program3") {
		Serial.println("\e[0;31mtest_playing_workflow: FAILED");
		Serial.println("function did not return correct error message when file does not exist");
		Serial.println("expected: could not find program: test_program3");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if program can be correctly executed
	String output2 = playing_workflow(program_name1);

	if (output2 != "successfully played program: test_program") {
		Serial.println("\e[0;31mtest_playing_workflow: FAILED");
		Serial.println("function did not return correct message when program was played");
		Serial.println("expected: successfully played program: test_program");
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if error messages of program_parser are shown correctly
	String output3 = playing_workflow(program_name2);

	if (output3 != "could not find signal: abc") {
		Serial.println("\e[0;31mtest_playing_workflow: FAILED");
		Serial.println("function did not return correct error message when program could not be played");
		Serial.println("expected: could not find signal: abc");
		Serial.println("actual: " + output3 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_playing_workflow: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

boolean test_program_parser() {
	/*
	- check if program can be executed correctly
	- check every error message of program_parser and called functions
	*/

	// create test data
	String code1 = "play test_signal \nwait 100 \nloop 3 \n \nplay test _-signal \nwait 100 \nend \n";	// correct program
	String code2 = "blah blah \n";	// faulty program
	String code3 = "play abc \n"; // faulty signal
	String code4 = "play test signal \n"; // correct program

	// create test signals
	LittleFS.begin();
	File file1 = LittleFS.open("/signals/test_signal.json", "w");
	DynamicJsonDocument doc1(512);
	doc1["name"] = "test_signal";
	doc1["length"] = 3;
	doc1["sequence"] = "1234, 5678, 412";
	doc1.shrinkToFit();
	serializeJson(doc1, file1);
	file1.close();

	File file2 = LittleFS.open("/signals/test _-signal.json", "w");
	DynamicJsonDocument doc2(512);
	doc2["name"] = "test _-signal";
	doc2["length"] = 3;
	doc2["sequence"] = "1234, 5678, 412";
	doc2.shrinkToFit();
	serializeJson(doc2, file2);
	file2.close();
	LittleFS.end();

	// tests if program can be executed correctly
	String output1 = program_parser(code1);

	if (output1 != "success") {
		Serial.println("\e[0;31mtest_program_parser: FAILED");
		Serial.println("function did not return correct message when program 1 was played");
		Serial.println("expected: success");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if error message is correct when program is faulty
	String output2 = program_parser(code2);

	if (output2 != "invalid command: blah blah") {
		Serial.println("\e[0;31mtest_program_parser: FAILED");
		Serial.println("function did not return correct error message when program 2 is faulty");
		Serial.println("expected: could not find command: blah");
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if error message is correct when signal is faulty
	String output3 = program_parser(code3);

	if (output3 != "could not find signal: abc") {
		Serial.println("\e[0;31mtest_program_parser: FAILED");
		Serial.println("function did not return correct error message when signal from program 3 is faulty");
		Serial.println("expected: could not find signal: abc");
		Serial.println("actual: " + output3 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_program_parser: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}

boolean test_handle_wait_command() {
	/*
	- check if set amount of time is waited
	- checks if return message is correct
	*/

	// create test data
	unsigned long test_times[4] = {1, 10, 100, 1000};

	String output;
	// tests if set amount of time is waited
	for (int i = 0; i < 4; i++) {
		unsigned long start_time = millis();
		output = handle_wait_command(test_times[i]);
		unsigned long end_time = millis();

		if (output != "success") {
			Serial.println("\e[0;31mtest_handle_wait_command: FAILED");
			Serial.println("function did not return correct message");
			Serial.println("expected: success");
			Serial.println("actual: " + output + "\e[0;37m");
			return(false);
		}

		if (end_time - start_time < test_times[i]) {
			Serial.println("\e[0;31mtest_handle_wait_command: FAILED");
			Serial.println("function did not wait long enough");
			Serial.println("expected: " + String(test_times[i]));
			Serial.println("actual: " + String(end_time - start_time) + "\e[0;37m");
			return(false);
		}

		if (end_time - start_time > test_times[i] + 200) {
			Serial.println("\e[0;31mtest_handle_wait_command: FAILED");
			Serial.println("function waited too long");
			Serial.println("expected: " + String(test_times[i]));
			Serial.println("actual: " + String(end_time - start_time) + "\e[0;37m");
			return(false);
		}
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_handle_wait_command: PASSED\e[0;37m");
	return(true);
}

boolean test_handle_times_commands() {
	/*
	- checks error message if weekday is invalid
	- checks if invalid commands are caught
	*/

	// clean LittleFS
	clean_LittleFS();

	// create test data
	String command1 = "ednesday 12:00:00 ___test";
	String command2 = "Wednesday 12:00:00 ___test2";

	// create test signal
	LittleFS.begin();
	File file = LittleFS.open("/signals/___test.json", "w");
	file.close();
	LittleFS.end();

	// tests error message if weekday is invalid
	String output1 = handle_times_commands(command1, true);

	if (output1 != "weekday in command " + command1 + " is not valid") {
		Serial.println("\e[0;31mtest_handle_times_commands: FAILED");
		Serial.println("function did not return correct error message when weekday is invalid");
		Serial.println("expected: weekday in command " + command1 + " is not valid");
		Serial.println("actual: " + output1 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// tests if invalid commands are caught
	String output2 = handle_times_commands(command2, true);

	if (output2 != "signal in command " + command2 + " not found") {
		Serial.println("\e[0;31mtest_handle_times_commands: FAILED");
		Serial.println("function did not return correct error message when command is invalid");
		Serial.println("expected: signal in command " + command2 + " not found");
		Serial.println("actual: " + output2 + "\e[0;37m");
		clean_LittleFS();
		return(false);
	}

	// print success message and return true
	Serial.println("\e[0;32mtest_handle_times_commands: PASSED\e[0;37m");
	clean_LittleFS();
	return(true);
}
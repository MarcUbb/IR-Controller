#include "tests.h"

// tests for workflow.cpp

boolean test_deleding_workflow() {
	/*
	- checks if file is deleted correctly
	*/
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
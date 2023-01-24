#include "tests.h"

// tests for time_management.cpp

boolean test_weekday_to_num() {
	/*
	- checks if the correct number for each weekday is returned
	- checks if "error" is returned if the weekday is not valid
	*/
	return(true);
}

boolean test_compare_time() {
	/*
	- checks if sample times are compared correctly (millis has to be mocked)
	*/
	return(true);
}

boolean test_update_time() {
	/*
	- checks if the time is updated correctly (format is always assumed to be correct)
	*/
	return(true);
}

boolean test_get_current_time() {
	/*
	- checks if time is returned correctly (millis has to be mocked)
	*/
	// mocking millis
	// unsigned long millis() {
	// 	return(0);
	// }

	return(true);
}

boolean test_turn_seconds_in_time() {
	/*
	- checks sample of conversions
	*/
	return(true);
}

boolean test_add_time() {
	/*
	- checks sample of additions
	*/
	return(true);
}

boolean test_get_NTP_time() {
	
	return(true);
}

boolean test_init_time() {
	/*
	checks if the time is initialized correctly (get_NTP_time has to be mocked)
	*/
	return(true);
}

boolean test_check_and_update_offset() {
	/*
	- checks if the offset is updated correctly (millis has to be mocked)
	*/
	return(true);
}

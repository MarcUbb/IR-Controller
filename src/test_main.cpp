#include "tests.h"

// runs all tests for filesystem.cpp
boolean run_all_filesystem_tests() {
  Serial.println("\nTesting filesystem.cpp");

  boolean check = true;

  check = test_capture_signal();
  if(check != true) return check;

  check = test_save_signal();
  if(check != true) return check;

  check = test_save_json();
  if(check != true) return check;

  check = test_load_json();
	if(check != true) return check;

	check = test_send_signal();
	if(check != true) return check;

	check = test_get_files();
	if(check != true) return check;

	check = test_check_if_file_exists();
	if(check != true) return check;

	check = test_read_program();
	if(check != true) return check;

	check = test_control_led_output();
	if(check != true) return check;

	check = test_check_if_string_is_alphanumeric();
	if(check != true) return check;

  return check;
}


// runs all tests for time_management.cpp
boolean run_all_time_management_tests() {
  Serial.println("\nTesting time_management.cpp");

  boolean check = true;
  
	check = test_weekday_to_num();
	if(check != true) return check;

	check = test_compare_time();
	if(check != true) return check;

	check = test_update_time();
	if(check != true) return check;

	check = test_get_current_time();
	if(check != true) return check;

	check = test_turn_seconds_in_time();
	if(check != true) return check;

	check = test_add_time();
	if(check != true) return check;

	check = test_get_NTP_time();
	if(check != true) return check;

	check = test_init_time();
	if(check != true) return check;

	check = test_check_and_update_offset();
	if(check != true) return check;

  return check;
}


// runs all tests for workflows.cpp
boolean run_all_workflows_tests() {
  Serial.println("\nTesting workflows.cpp");

  boolean check = true;
  
	check = test_deleding_workflow();
	if(check != true) return check;

	check = test_recording_workflow();
	if(check != true) return check;

	check = test_sending_workflow();
	if(check != true) return check;

	check = test_adding_workflow();
	if(check != true) return check;

	check = test_playing_workflow();
	if(check != true) return check;

	check = test_program_parser();
	if(check != true) return check;

	check = test_handle_wait_command();
	if(check != true) return check;

	check = test_handle_times_commands();
	if(check != true) return check;

  return check;
}


// runs all tests (called in main.cpp)
void run_all_tests() {
  Serial.println("\nRunning all tests...");

  boolean check = true;

	// only run tests if above tests passed
  check = run_all_filesystem_tests();
  if(check == true) {check = run_all_time_management_tests();}
  if(check == true) {check = run_all_workflows_tests();}
  

  if(check != true) {
    Serial.println("\n\e[0;31m----------------------------------------");
    Serial.println("At least one test failed!");
    Serial.println("Your code will not be executed. Please fix the errors and try again.\e[0;37m\n");
    while(true) {
      delay(1000);
    }
  }
  else {
    Serial.println("\n\e[0;32m----------------------------------------");
    Serial.println("All tests passed!");
    Serial.println("Your code will now continue to run.\e[0;37m\n");
  }
}
/**
 * @file test_main.cpp
 * @author Marc Ubbelohde
 * @brief main file for all tests
 * 
 * @details Here you find the functions that execute all tests for each file 
 * and finally a function that executes all tests for all files. 
 * 
 */

#include "tests.h"

/**
 * @brief runs all tests for filesystem.cpp
 * 
 * @param stop_on_error - if true, the function stops after the first failed test if false, the function continues to run all following tests
 * 
 * @return boolean - true if all tests passed, false if at least one test failed
 * 
 * @callgraph
 * 
 * @callergraph
 */
boolean run_all_filesystem_tests(boolean stop_on_error) {
  Serial.println("\nTesting filesystem.cpp");

  boolean check = true;
	boolean set_check = true;

  check = test_capture_signal();
  if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

  check = test_save_signal();
  if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

  check = test_save_json();
  if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

  check = test_load_json();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_send_signal();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_get_files();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_check_if_file_exists();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_read_program();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_control_led_output();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_check_if_string_is_alphanumeric();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

  return set_check;
}


/**
 * @brief runs all tests for time_management.cpp
 * 
 * @param stop_on_error - if true, the function stops after the first failed test if false, the function continues to run all following tests
 * 
 * @return boolean - true if all tests passed, false if at least one test failed
 * 
 * @callgraph
 * 
 * @callergraph
 */
boolean run_all_time_management_tests(boolean stop_on_error) {
  Serial.println("\nTesting time_management.cpp");

  boolean check = true;
	boolean set_check = true;
  
	check = test_weekday_to_num();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_compare_time();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_update_time();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_get_current_time();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_turn_seconds_in_time();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_add_time();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_get_NTP_time();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_check_and_update_offset();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

  return set_check;
}


/**
 * @brief runs all tests for workflows.cpp
 * 
 * @param stop_on_error - if true, the function stops after the first failed test if false, the function continues to run all following tests
 * 
 * @return boolean - true if all tests passed, false if at least one test failed
 * 
 * @callgraph
 * 
 * @callergraph
 */
boolean run_all_workflows_tests(boolean stop_on_error) {
  Serial.println("\nTesting workflows.cpp");

  boolean check = true;
	boolean set_check = true;
  
	check = test_deleting_workflow();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_recording_workflow();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_sending_workflow();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_adding_workflow();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_playing_workflow();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_program_parser();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_handle_wait_command();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

	check = test_handle_times_commands();
	if(!check && stop_on_error) return check;
	if(!check) {set_check = false;}

  return set_check;
}


/**
 * @brief runs all tests for all files
 * 
 * @param stop_on_error - if true, the function stops after the first failed test if false, the function continues to run all following tests
 * 
 * @callgraph
 * 
 * @callergraph
 */
void run_all_tests(boolean stop_on_error) {
  Serial.println("\nRunning all tests...");

  boolean check = true;
	boolean set_check = true;

	// only run tests if above tests passed
  check = run_all_filesystem_tests(stop_on_error);
	if(check == false) {set_check = false;}
  if(check == true || !stop_on_error) {check = run_all_time_management_tests(stop_on_error);}
	if(check == false) {set_check = false;}
  if(check == true || !stop_on_error) {check = run_all_workflows_tests(stop_on_error);}
	if(check == false) {set_check = false;}
  

  if(set_check != true) {
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

/**
 * @brief runs all empirical tests
 * 
 * @param stop_on_error - if true, the function stops after the first failed test if false, the function continues to run all following tests
 * 
 * @callgraph
 * @callergraph
 */
void run_all_empirical_tests(boolean stop_on_error) {
	Serial.println("\nRunning empirical tests...");

	boolean check = true;
	boolean set_check = true;

	check = empirical_test_get_NTP_time();
	if(!check) {set_check = false;}


	if(set_check != true) {
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
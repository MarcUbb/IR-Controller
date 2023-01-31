/**
 * @file tests.h
 * @author Marc Ubbelohde
 * @brief Header File for all test files
 * 
 * @details This file provides access to the source code for 
 * all unit test funcitons. It also provides access to the
 * unit test functions for the functions in test_main.cpp.
 * 
 */


#include "base.h"
#include "workflows.h"

void clean_LittleFS();

boolean test_capture_signal();
boolean test_save_signal();
boolean test_save_json();
boolean test_load_json();
boolean test_send_signal();
boolean test_get_files();
boolean test_check_if_file_exists();
boolean test_read_program();
boolean test_control_led_output();
boolean test_check_if_string_is_alphanumeric();

boolean test_weekday_to_num();
boolean test_compare_time();
boolean test_update_time();
boolean test_get_current_time();
boolean test_turn_seconds_in_time();
boolean test_add_time();
boolean test_get_NTP_time();
boolean test_check_and_update_offset();

boolean test_deleting_workflow();
boolean test_recording_workflow();
boolean test_sending_workflow();
boolean test_adding_workflow();
boolean test_playing_workflow();
boolean test_program_parser();
boolean test_handle_wait_command();
boolean test_handle_times_commands();

boolean run_all_filesystem_tests(boolean stop_on_error);
boolean run_all_time_management_tests(boolean stop_on_error);
boolean run_all_workflows_tests(boolean stop_on_error);
void run_all_tests(boolean stop_on_error);
void run_all_empirical_tests(boolean stop_on_error);

boolean empirical_test_get_NTP_time();
#include <assert.h>
#include <Arduino.h>
#include "base.h"
#include "workflows.h"

// tests for filesystem.cpp

boolean test_capture_signal() {
  /*
  Tests capture_signal() function
  - checks if execution time is normal
  - checks if return value is correct (no random noise signal is received)
  */
  unsigned long start_time = millis();
  String return_val = capture_signal();
  unsigned long end_time = millis();

  unsigned long elapsed_time = end_time - start_time;

  if (return_val == "no_signal" && elapsed_time > 10000 && elapsed_time < 12000) {
    Serial.println("\e[0;32mtest_capture_signal: PASSED\e[0;37m");
    return true; 
  }
  else {
    Serial.println("\e[0;31mtest_capture_signal: FAILED");
    Serial.println("return value: " + return_val + " , elapsed time: " + elapsed_time + "\e[0;37m");)
    return false;
  }
}

boolean run_all_filesystem_tests() {
  Serial.println("\nTesting filesystem.cpp");
  boolean check = true;
  check =  test_capture_signal();
  // ...

  return check;
}

// tests for time_management.cpp

boolean run_all_time_management_tests() {
  Serial.println("\nTesting time_management.cpp");
  boolean check = true;
  // ...

  return check;
}

// tests for workflows.cpp

boolean run_all_workflows_tests() {
  Serial.println("\nTesting workflows.cpp");
  boolean check = true;
  // ...

  return check;
}

// run all tests (calles in main.cpp)
void run_all_tests() {
  boolean check = true;
  Serial.println("\nRunning all tests...");

  check = run_all_filesystem_tests();
  check = run_all_time_management_tests();
  check = run_all_workflows_tests();

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
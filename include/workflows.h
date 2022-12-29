#include "base.h"

String deleting_workflow(String directory, String command_name);

String recording_workflow(String command_name);
String sending_workflow(String command_name);

String adding_workflow(String program_name, String program_code);
String playing_workflow(String program_name);

String program_parser(String code);

String handle_wait(unsigned long waiting_time);
String handle_time(String time_command);
String handle_day(String day_command);
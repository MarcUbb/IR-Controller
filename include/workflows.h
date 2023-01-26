#include "base.h"
#include "Regexp.h"

// forward declarations
String deleting_workflow(String directory, String command_name);

String recording_workflow(String command_name);
String sending_workflow(String command_name);

String adding_workflow(String program_name, String program_code);
String playing_workflow(String program_name);

String program_parser(String code);

String handle_wait_command(unsigned long waiting_time);
String handle_times_commands(String command, boolean day_included);
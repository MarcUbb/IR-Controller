#include "main.h"


void setup() {
  Serial.begin(115200);
  test();

  // WiFiManager (is skipped if credentials are saved)
  //ESP.eraseConfig();
  WiFiManager wifiManager;
  wifiManager.autoConnect("IR-Remote");
  
  // start mDNS
  if (MDNS.begin("irr") == false) {
    Serial.println("Error setting up MDNS responder!");
    control_led_output("no_mDNS");
    // stalls program execution if mDNS fails
    while (1) { delay(1000); }
  }
  Serial.println("mDNS responder started!");

  // initiate time via NTP
  if (init_time() == false){
    message = "Error initiating time!";
  }
  else {
    message = "Time initiated!";
  }

  // declare handler functions
  server.on("/", handleRoot);
  server.on("/form", handleForm);
  server.on("/files", send_files);
  server.on("/program", handleProgram);
  server.on("/error", handleError);
  server.on("/time", handleTime);
  server.onNotFound(handleNotFound);

  // start server
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  MDNS.update();
  server.handleClient();
}



//------------------ handlers ------------------//

// serves index.html
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// serves 404 error
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// the website is designed with form elements that trigger forwarding to /form. This is a problem because we can only communicate on that channel.
// we have to communicate a back to the website to update the page. So in order to still be able to send data back to the website, on every reload 
// the website will send a get request on /program. This will trigger handleProgram() which will send the program name and code back to the website.
void handleProgram() {
  String programcode = read_program(programname);
  server.send(200, "text/plain", programname + ";" + programcode);
}

// Similarly to handleProgram(), the website will send a get request on /error to update the error message on reload.
// The message is again written by handleForm().
void handleError() {
  server.send(200, "text/plain", message);
}

// sends a list of all files in /signals and /programs on reload
void send_files() {
  String files = get_files("/signals", "/programs");
  Serial.println(files);
  server.send(200, "text/plane", files);
}

// gets the time from the client via Date() and saves it together with the millis() offset to a file
// the offset is important because only with millis() we can calculate the time that has passed between the synchronisation and the time of program execution.
// this enabled the ESP to execute timed programs even if the wifi connection is lost.
void handleTime() {
  String timezone = server.arg("time_dummy");
  update_timezone(timezone.toInt());
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}

// handles all form elements on the website (signals and programs)
// also updates the error message and programname.
// all the logic is handled here and the functions from workflows.h are called.
void handleForm() {
  String signal = server.arg("signal");
  String send_signal_button = server.arg("send_signal_button"); 
  String delete_signal_button = server.arg("delete_signal_button"); 
  String signal_name = server.arg("signal_name"); 
  String add_signal_button = server.arg("add_signal_button");
  String program = server.arg("program");
  String play_program_button = server.arg("play_program_button"); 
  String program_code = server.arg("program_code");
  String program_name = server.arg("program_name"); 
  String add_program_button = server.arg("add_program_button");
  String delete_programs_button = server.arg("delete_program_button");
  String edit_program_button = server.arg("edit_program_button");

  /*
  Serial.println("signal: " + signal);
  Serial.println("send_signal_button: " + send_signal_button);
  Serial.println("delete_signal_button: " + delete_signal_button);
  Serial.println("signal_name: " + signal_name);
  Serial.println("add_signal_button: " + add_signal_button);
  Serial.println("program: " + program);
  Serial.println("play_program_button: " + play_program_button);
  Serial.println("program_code: " + program_code);
  Serial.println("program_name: " + program_name);
  Serial.println("add_program_button: " + add_program_button);
  Serial.println("delete_programs_button: " + delete_programs_button);
  Serial.println("edit_program_button: " + edit_program_button);
  */

  // signal logic:
  if (signal_name != "" && add_signal_button != "") {
    message = recording_workflow(signal_name);
  }

  else if (signal_name == "" && add_signal_button != "") {
    message = "no signal name given";
  }

  else if (signal != "") {
    if (send_signal_button != "") {
      message = sending_workflow(signal);
    }
    else if (delete_signal_button != "") {
      message = deleting_workflow("signals", signal);
    }
  }

  else if (signal == "" && send_signal_button != "") {
    message = "no signal selected";
  }

  else if (signal == "" && delete_signal_button != "") {
    message = "no signal selected";
  }

  // program logic:
  else if (program_name != "" && add_program_button != "" && program_code != "") {
    message = adding_workflow(program_name, program_code);
  }

  else if (program_name == "" && add_program_button != "") {
    message = "no program name given";
  }

  else if (program_code == "" && add_program_button != "") {
    message = "no program code given";
  }

  else if (program != "") {
    if (play_program_button != "") {
      message = playing_workflow(program);
    }
    else if (delete_programs_button != "") {
      message = deleting_workflow("programs", program);
    }
    // provides data for separate Get request
    else if (edit_program_button != "") {
      programname = program;
      message = "successfully loaded program: " + program;
    }
  }

  else if (program == "" && play_program_button != "") {
    message = "no program selected";
  }

  else if (program == "" && delete_programs_button != "") {
    message = "no program selected";
  }

  else if (program == "" && edit_program_button != "") {
    message = "no program selected";
  }

  if (edit_program_button == "") {
    programname = "";
  }
  
  // sends user back to the website
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated– Press Back Button");
}
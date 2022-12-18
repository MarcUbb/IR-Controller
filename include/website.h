#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>IR Remote</title>
  <style>
    html {font-family: Arial; display: inline-block;}
    body {max-width: 400px; margin: 10%; padding-bottom: 25px;} 
    label {padding: 8px 8px 8px 0px; display: inline-block;}
    select {width: 100%; margin: 0px 0px 10px; padding: 10px 10px 10px; border: none; border-radius: 4px; background-color: #f1f1f1;}
    textarea {width: 100%; height: 150px; padding: 12px 20px; box-sizing: border-box; border: 2px solid #ccc; border-radius: 4px; background-color: #f8f8f8; font-size: 16px; resize: none;}
    input[type=text] {padding:5px; border:2px solid #ccc; -webkit-border-radius: 5px; border-radius: 5px;}
    input[type=submit] {padding:5px 15px; background:#ccc; border:0 none; cursor:pointer; -webkit-border-radius: 5px; border-radius: 5px;}
    h2 {text-align: center;}
    button {padding:5px 15px; background:#ccc; border:0 none; cursor:pointer; -webkit-border-radius: 5px; border-radius: 5px;}
  </style>
</head>
<body onload="getData()">
  <h2>IR-Remote Hub</h2>
  <br>
  <h3>Signals:</h3>

  <form action="/get">
    <label for="seqName">Signal name:</label> <br>
    <input type="text" id="seqName" name="seqName" placeholder="signal name">
    <input type="submit" name="add_sequence_button" value="add">
    <button onclick="infoSequence()">help</button>
  </form>

  <form action="/get">
    <label for="sequence">Choose your signal and action:</label> <br>
    <select id="sequences" name="sequence"></select>
    <input type="submit" name="send_sequence_button" value="send">
    <input type="submit" name="delete_sequence_button" value="delete">
  </form>
  <br><br>

  <h3>Programs:</h3>

  <form action="/get">
    <label for="progName">Program name:</label><br>
    <input type="text" id="progName" name="progName" placeholder="program name">
    <input type="submit" name="add_program_button" value="add">
    <button onclick="infoProgram()">help</button> <br>
    <label for="progCode">Program code:</label><br>
    <textarea name="progCode" placeholder="write your code here"></textarea><br>
  </form>

  <form action="/get">
    <label for="sequence">Choose your sequence and action:</label> <br>
    <select id="programs" name="program"></select>
    <input type="submit" name="play_program_button" value="play">
    <input type="submit" name="delete_program_button" value="delete">
  </form>

  <script>
    // https://stackoverflow.com/questions/3364493/how-do-i-clear-all-options-in-a-dropdown-box
    function removeOptions(selectElement) {
      var i, L = selectElement.options.length - 1;
      for(i = L; i >= 0; i--) {
          selectElement.remove(i);
      }
    }

    // https://circuitdigest.com/microcontroller-projects/ajax-with-esp8266-dynamic-web-page-update-without-reloading
    
    function getData() {
      if (localStorage.getItem("hasCodeRunBefore") === null) {
          alert("Welcome to the IR-Remote Hub! Here you can read and play infrared sequences and write small programs which play your sequences automatically.")
          localStorage.setItem("hasCodeRunBefore", true);
      }

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          removeOptions(document.getElementById("sequences"));
          removeOptions(document.getElementById("programs"));

          // preprocessing of string data
          var files = this.responseText;
          var fileArray = files.split(";");
          console.log(fileArray);
          
          var sequences = fileArray[0].split(",");
          var programs = fileArray[1].split(",");
          console.log(sequences);
          console.log(programs);

          // adds sequences to dropdown
          for (const val of sequences) {
            var option = document.createElement("option");
            option.value = val;
            option.text = val;
            document.getElementById("sequences").appendChild(option)
          }

          // adds programs to dropdown
          for (const val of programs) {
            var option = document.createElement("option");
            option.value = val;
            option.text = val;
            document.getElementById("programs").appendChild(option)
          }
        }
      };
      xhttp.open("GET", "files", true);
      xhttp.send();
    }

    function infoSequence (){
      alert("To add an infrared signal you have to enter a name and then click on 'add'. A red LED lights up on the device, which signals that the signal can now be read in. Hold your remote control to the device and press the button to be read in. If an error has occurred, the LED flashes 3 times. Then please try again. Otherwise the signal is now saved!");
    }

    function infoProgram (){
      alert("A program is a sequence of infrared signals that you can program. To do this, give the program a name and write the code in the field provided. The code is structured as follows: Command Value. Currently the following commands are supported: play and wait.");
    }

    </script>
</body>
</html>)rawliteral";
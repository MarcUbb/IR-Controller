# IR-Controller
This repository contains the software for an infrared controller, the purpose of which is to be able to control devices with an infrared interface using a smartphone.

### JSON-Format:
```
Philips TV.txt:
{
  "name":"Philips TV",
  "length": 9, 
  "sequence": "21,24,71,34,74,73,21,60,32"
}

```
### Setup:
#### WifiManager
- always starts on startup to either connect to wifi or configure new wifi
- saves Network credentials at same location as WPS (examples already do that automatically)

#### WPS Interrupt
- alternatively to WifiManager its possible to connect via WPS
- an Interrupt is triggered on pressing the WPS Button
- saves Network credentials at same location as WifiManager (examples already do that automatically)

### Methodes:
#### read_signal(str name)
1. turns Signal LED on
2. reads incoming IR-Signal with defined minimum length

  2.1 IR-Signal is received -> 3.
  
  2.2 No IR-Signal is received -> Signal LED Blinks and Timeout is sent
  
3. turns Signal LED off
4. creates new entry in data.json and writes name and sequence to file

#### play_signal(str name)
1. looks for entry with given name

  1.1 Entry is found -> 2.
  
  1.2 Entry is not found -> try to delete Entry from frontend
  
2. plays entered sequence

####

### Webserver:
- Asynchronous Webserver
1. provides single page website

#### Buttons:
1. send HTTP Get Request to URL
2. MC saves submitted name and recorded IR-Signal
3. 

### Programs:

#### Commands:

1. play
  - plays a signal
2. wait
  - waits a specific amount of ms
3. time
  - plays a signal at the next time a specified time is reached
4. date
  - plays a signal the next time a specific date and time is reached
5. skip
  - skips a specified amount of days
6. loop
  - loops a part of the program a specified amount of times or indefinitely
7. end
  - marks the end of a loop

### Annotations:
- #include <WiFiManager.h> in main.cpp otherwise error in library occures
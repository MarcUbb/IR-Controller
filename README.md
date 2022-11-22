# IR-Controller
This repository contains the software for an infrared controller, the purpose of which is to be able to control devices with an infrared interface using a smartphone.

### JSON-Format:
```
data.json:
{
  {
    "name":"Philips TV", 
    "sequence": [21,24,71,34,74,73,21,60,32]
  }
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


### Annotations:
- #include <WiFiManager.h> in main.cpp otherwise error in library occures
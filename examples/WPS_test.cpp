#include <ESP8266WiFi.h>
#define WPS D6 //Pin für den WPS Taster


//Startet die WPS Konfiguration
bool startWPS() {
  Serial.println("WPS Konfiguration gestartet");
  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) {
      // Muss nicht immer erfolgreich heißen! Nach einem Timeout bist die SSID leer
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) {
        // Nur wenn eine SSID gefunden wurde waren wir erfolgreich 
        Serial.printf("WPS fertig. Erfolgreich angemeldet an SSID '%s'\n", newSSID.c_str());
      } else {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}

//Setup Funktion
void setup() {
  Serial.begin(74880); //mit 74880 sind auch die Meldungen beim Start sichtbar
  Serial.setDebugOutput(true); //Wenn true werden zusätzliche Debug Informationen ausgegeben
  delay(1000);
  Serial.printf("\nVersuche Verbindung mit gespeicherter SSID '%s'\n", WiFi.SSID().c_str());
  pinMode(WPS, INPUT_PULLUP); //Taster Eingang aktivieren

  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // letzte gespeicherte Zugangsdaten
  int cnt = 0;
  //Wir versuchen eine Anmeldung
  while ((WiFi.status() == WL_DISCONNECTED) && (cnt < 10)){
    delay(500);
    Serial.print(".");
    cnt++;
  }

  wl_status_t status = WiFi.status();
  if(status == WL_CONNECTED) {
    Serial.printf("\nErfolgreich angemeldet an SSID '%s'\n", WiFi.SSID().c_str());
  } else {
    //Wir waren nicht erfolgreich starten daher WPS
    Serial.printf("\nKann keine WiFi Verbindung herstellen. Status ='%d'\n", status); 
    Serial.println("WPS Taste am Router drücken.\n WPS Taste am ESP drücken!");
    while (digitalRead(WPS)!=0) {yield();}   
    if(!startWPS()) {
       Serial.println("Keine Verbindung über WPS herstellbar");  
    }
  } 
}


void loop() {
  // Code fürs Programm

}
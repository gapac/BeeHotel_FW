/*                                                                                 
     _/_/_/            _/  _/  _/                  _/        _/_/    _/            
  _/          _/_/    _/      _/_/_/    _/  _/_/          _/    _/      _/  _/_/   
 _/        _/    _/  _/  _/  _/    _/  _/_/      _/      _/_/_/_/  _/  _/_/        
_/        _/    _/  _/  _/  _/    _/  _/        _/      _/    _/  _/  _/           
 _/_/_/    _/_/    _/  _/  _/_/_/    _/        _/      _/    _/  _/  _/            
                                                                                   
  ColibriAir - Sensors to Ubidots
*/

// Vključitev povezanih knjižnic
#include "WiFi.h"

// Zakasnitev med posameznimi pregledi omrežja
const int delayTime = 5000; //ms

// Funkcija Setup se izvede le enkrat ko resetiramo napravo
void setup()
{
    // Inicializacija serijskega vmesnika
    Serial.begin(115200);

    // Vključitev WiFi Station Mode in prekinitev morebitne prejšnje povezave
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

}

// Funkcija Loop se izvaja vedno znova in znova
void loop()
{
    Serial.print("Začetek pregleda...");

    // Funkcija WiFi.scanNetworks vrne število najdenih omrežij WiFi
    int n = WiFi.scanNetworks();
    Serial.println("pregled zaključen.");
    if (n == 0) {
        Serial.println("Ni najdenih omrežij Wi-Fi.");
    } else {
        Serial.print("Najdenih omrežij: ");
        Serial.println(n);
                
        for (int i = 0; i < n; ++i) {
            // Izpis SSID in RSSI za vsako najdeno omrežje
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print("dBm)");
            Serial.println((WiFi.encryptionType(i) == 0)?" OPEN":" ENCRYPTED");
            delay(10);
        }
    }
    Serial.println("");

    // Zakasnitev med posameznim pregledom
    delay(delayTime);
}

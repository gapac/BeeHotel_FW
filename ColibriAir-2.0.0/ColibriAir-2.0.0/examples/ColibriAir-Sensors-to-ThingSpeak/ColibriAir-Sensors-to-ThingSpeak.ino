/*                                                                                 
     _/_/_/            _/  _/  _/                  _/        _/_/    _/            
  _/          _/_/    _/      _/_/_/    _/  _/_/          _/    _/      _/  _/_/   
 _/        _/    _/  _/  _/  _/    _/  _/_/      _/      _/_/_/_/  _/  _/_/        
_/        _/    _/  _/  _/  _/    _/  _/        _/      _/    _/  _/  _/           
 _/_/_/    _/_/    _/  _/  _/_/_/    _/        _/      _/    _/  _/  _/            
                                                                                   
  ColibriAir - Sensors to ThingSpeak
*/

// Prijava na https://thingspeak.com/
// Ustvari kanal (Channels > New Channel)
// Dodaj ime (ColibriAir), Field 1 (LightLevel), Field 2 (Temperature), Field 3 (Humidity), Field 4 (VOC)
// Deli kanal (Channel > Sharing > Share channel view with everyone)
// Kopiraj ThingSpeak Token: Channel > API Keys > Write API Key


/****************************************
 * NASTAVITEV PARAMETROV
 ****************************************/
 
#define WIFI_SSID "TVOJ_WIFI"  // Ime tvojega Wi-Fi
#define WIFI_PASSWORD "TVOJE_WIFI_GESLO"  // Tvoje WI-Fi geslo
String THINGSPEAK_API_KEY = "TVOJ_THINGSPEAK_API_KEY"; // ThingSpeak Token: Channel > API Keys > Write API Key


/****************************************
 * Vključitev povezanih knjižnic
 ****************************************/

#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_VCNL4040.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SGP40.h>
#include <HTTPClient.h>


/****************************************
 * Definiranje spremenljivk
 ****************************************/

// ThingSpeak URL
String ThingSpeakURL = String("https://api.thingspeak.com/update?api_key=" + THINGSPEAK_API_KEY);


// Kreiranje objektov
Adafruit_VCNL4040 vcnl = Adafruit_VCNL4040();
Adafruit_SHT31 sht = Adafruit_SHT31();
Adafruit_SGP40 sgp;
HTTPClient http;

// Določitev pinov kamor so vezane LED
const int LED_BLUE = 25;
const int LED_GREEN = 26;
const int LED_RED = 27;

// Zakasnitev med posameznimi meritvami
const int delayTime = 60; //sec

// Offset parameters
const int offsetTemperature = 5;
const int offsetHumidity = 13;

// Deklaracija spremenljivk
long ALS = 0;
float T = 0;
float RH = 0;
int VOC = 0;

int httpGetCode;
String DataPacket;


const char* ca = \ 
"-----BEGIN CERTIFICATE-----\n" \  
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \  
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \  
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \  
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \  
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \  
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \  
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \  
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \  
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \  
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \  
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \  
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \  
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \  
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \  
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \  
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \  
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \  
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \  
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \  
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \  
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \  
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \  
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \  
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \  
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \  
"-----END CERTIFICATE-----\n";

/****************************************
 * Funkcija Setup
 ****************************************/
void setup() {

  // Inicializacija digitalnih izhodov
  pinMode(LED_GREEN, OUTPUT);  
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Vse LED ugasnjene
  digitalWrite(LED_GREEN, HIGH); 
  digitalWrite(LED_BLUE, HIGH); 
  digitalWrite(LED_RED, HIGH); 

  // Inicializacija I2C in serijskega vmesnika
  Wire.begin(16, 17);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for serial console to open!

  Serial.println("### ColibriAir - Sensors to TS ###");

  Serial.println();
  Serial.print("Povezava Wi-Fi se vzpostavlja .");

  // Vzpostavitev povezave Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blinkLed(LED_BLUE);
    delay(500);
  }
  
  Serial.println(" Povezava vzpostavljena!");
  Serial.print("IP naslov: "); Serial.println(WiFi.localIP());
  Serial.println();

  // Začetna povezava s senzorji
  if (!vcnl.begin()) {
    Serial.println("WARNING: VCNL4040 sensor not found!");
    while (1);
  }

  if (!sht.begin()) {
    Serial.println("WARNING: SHT31 sensor not found!");
    while (1);
  }

  if (!sgp.begin()){
    Serial.println("WARNING: SGP40 sensor not found!");
    while (1);
  }

  // Inicializacija senzorja VCNL4040
//  vcnl.setProximityLEDCurrent(VCNL4040_LED_CURRENT_200MA);
//  vcnl.setProximityLEDDutyCycle(VCNL4040_LED_DUTY_1_40);
//  vcnl4040.setAmbientIntegrationTime(VCNL4040_AMBIENT_INTEGRATION_TIME_80MS);
//  vcnl4040.setProximityIntegrationTime(VCNL4040_PROXIMITY_INTEGRATION_TIME_8T);
//  vcnl4040.setProximityHighResolution(false);
  
}

/****************************************
 * Funkcija Loop
 ****************************************/
void loop() {

  // Preberi vrednosti iz VCNL4040
  long ALS = vcnl.getLux(); 

  // Preberi vrednosti iz SHT31
  float T = sht.readTemperature() - offsetTemperature;
  float RH = sht.readHumidity() + offsetHumidity;

  // Preberi vrednosti iz SGP40
  int VOC = sgp.measureVocIndex(T, RH);
  
  // Izpis vrednosti na serijski vmesnik
  printValuesToSerial();

  // Preveri ali senzor VOC že meri
  if (VOC < 1) {  // Če vrednost VOC == 0
    blinkLed(LED_GREEN);
    delay(500);
  }
  else {  // Senzor že meri

    // Vklopi LED glede na kvaliteto zraka
    ledIndicator(VOC);
  
    // Ponovna vzpostavitev povezave Wi-Fi v primeru, da je bila prekinjena
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnect!");
      reconnectWiFi();
    }

    // Oblikovanje ThingSpeak paketa
    DataPacket = String(ThingSpeakURL + "&field1=" + ALS + "&field2=" + T + "&field3=" + RH + "&field4=" + VOC);
    Serial.println(DataPacket);
    
    // Vzpostavitev povezave HTTP
    http.begin(DataPacket, ca);
    httpGetCode = http.GET();
    Serial.println(httpGetCode);
    if (httpGetCode > 0) {
      Serial.print("Podatki so uspešno poslani! Število vpisov v bazi: ");
      Serial.print(http.getString()); Serial.print(". ");
    }
    else {
      Serial.print("Podatki so neuspešno poslani! ");
    }
    http.end();   
    
    Serial.print("Čakam "); Serial.print(delayTime); Serial.println(" sekund ...");    
    Serial.println();
    
    // Zakasnitev pred naslednjim pošiljanjem
    delay(delayTime*1000);

  }
  
}



/****************************************
 * Dodatne funkcije
 ****************************************/

// Funkcija za vklop LED
void ledIndicator(int level) {

  if (level > 300) {
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_BLUE, HIGH); 
    digitalWrite(LED_RED, LOW);    
  }
  else if (level > 200) {
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_BLUE, LOW); 
    digitalWrite(LED_RED, HIGH);       
  }
  else if (level > 0) {
    digitalWrite(LED_GREEN, LOW); 
    digitalWrite(LED_BLUE, HIGH); 
    digitalWrite(LED_RED, HIGH);       
  }
  else {
    digitalWrite(LED_GREEN, HIGH); 
    digitalWrite(LED_BLUE, HIGH); 
    digitalWrite(LED_RED, HIGH);     
  }

  delay(500);

  // Turn Off Led
  digitalWrite(LED_GREEN, HIGH); 
  digitalWrite(LED_BLUE, HIGH); 
  digitalWrite(LED_RED, HIGH);     

}

// Funkcija za utrip LED
void blinkLed(int LED) {
  digitalWrite(LED, LOW); 
  delay(50);
  digitalWrite(LED, HIGH); 
}


// Funkcija za izpis na serijski vmesnik
void printValuesToSerial() {
  // Za pregled odpri Tools > Serial Monitor ali Serial Plotter 
  Serial.print("LightLevel[lx]:");
  Serial.print(ALS);
  Serial.print("  ");
  
  Serial.print("Temperature[*C]:");
  Serial.print(T);
  Serial.print("  ");

  Serial.print("RelativeHumidity[%]:");
  Serial.print(RH);
  Serial.print("  ");

  Serial.print("VOCIndex:");
  Serial.println(VOC);
  Serial.print("  ");
  
  Serial.println();

}


// Funkcija za ponovno vzpostavitev povezave Wi-Fi
void reconnectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    blinkLed(LED_BLUE);
    delay(500);
  }
}

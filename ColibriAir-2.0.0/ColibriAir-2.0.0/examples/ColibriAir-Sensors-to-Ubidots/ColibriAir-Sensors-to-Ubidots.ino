/*                                                                                 
     _/_/_/            _/  _/  _/                  _/        _/_/    _/            
  _/          _/_/    _/      _/_/_/    _/  _/_/          _/    _/      _/  _/_/   
 _/        _/    _/  _/  _/  _/    _/  _/_/      _/      _/_/_/_/  _/  _/_/        
_/        _/    _/  _/  _/  _/    _/  _/        _/      _/    _/  _/  _/           
 _/_/_/    _/_/    _/  _/  _/_/_/    _/        _/      _/    _/  _/  _/            
                                                                                   
  ColibriAir - Sensors to Ubidots
*/

// Prijava na https://ubidots.com/stem/
// Kopiraj Ubidots Token: User > API Credentials > Token


/****************************************
 * NASTAVITEV PARAMETROV
 ****************************************/
 
#define WIFI_SSID "TVOJ_WIFI"  // Ime tvojega Wi-Fi
#define WIFI_PASSWORD "TVOJE_WIFI_GESLO"  // Tvoje WI-Fi geslo
#define UBIDOTS_TOKEN "TVOJ_UBIDOTS_TOKEN" // Ubidots Token (User > API Credentials > Token)
#define MQTT_CLIENTNAME "unikatnoime" // Unikatno ime (8-12 znakov) - najbolje naključno generirano



/****************************************
 * Vključitev povezanih knjižnic
 ****************************************/

#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_VCNL4040.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SGP40.h>
#include <PubSubClient.h>


/****************************************
 * Definiranje spremenljivk
 ****************************************/

char mqttBroker[] = "industrial.api.ubidots.com";
char payload[100];
char topic[100];
char sensor_value[10];


// Kreiranje objektov
Adafruit_VCNL4040 vcnl = Adafruit_VCNL4040();
Adafruit_SHT31 sht = Adafruit_SHT31();
Adafruit_SGP40 sgp;
WiFiClient ubidots;
PubSubClient client(ubidots);

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

  Serial.println("### ColibriAir - Sensors to Ubidots ###");

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

  // Določitev povezave MQTT
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  

  Serial.println("### ColibriAir - Sensors ###");

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
  
    // Ponovna vzpostavitev povezave MQTT v primeru, da je bila prekinjena
    if (!client.connected()) {
      reconnect();
    }
  
    // Kreiranje MQTT paketa
    sprintf(topic, "%s%s", "/v1.6/devices/", "ColibriAir");
    sprintf(payload, "%s", ""); // Pobrišemo vsebino

    dtostrf(ALS, 5, 0, sensor_value); // 5 celoštevilska vrednost, 0 decimalk; float je preslikan v sensor_value
    sprintf(payload, "{\"LightLevel\": %s", sensor_value);  // Dodamo spremenljivko
    dtostrf(T, 4, 2, sensor_value); 
    sprintf(payload, "%s, \"Temperature\": %s", payload, sensor_value);  
    dtostrf(RH, 4, 2, sensor_value); 
    sprintf(payload, "%s, \"Humidity\": %s", payload, sensor_value);  
    dtostrf(VOC, 4, 0, sensor_value); 
    sprintf(payload, "%s, \"VOC\": %s}", payload, sensor_value);  
    
    client.publish(topic, payload);
    Serial.print("Podatki poslani! ");
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


// Funkcija za pošiljanje paketa MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

// Funkcija za ponovno vzpostavitev povezave Wi-Fi
void reconnect() {
  // Poizkušanje dokler se uspešno ne poveže
  while (!client.connected()) {
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENTNAME, UBIDOTS_TOKEN, "")) {
      Serial.print("Povezava z Ubidots MQTT uspešno vzpostavljena ... ");
    } else {
      Serial.print("Neuspela povezava z Ubidots MQTT, rc=");
      Serial.print(client.state());
      Serial.println(". Ponoven poizkus čez 2 sekundi.");
      // Počakamo 2 sekundi
      delay(2000);
    }
  }
}

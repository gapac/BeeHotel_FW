/*                                                                                 
     _/_/_/            _/  _/  _/                  _/        _/_/    _/            
  _/          _/_/    _/      _/_/_/    _/  _/_/          _/    _/      _/  _/_/   
 _/        _/    _/  _/  _/  _/    _/  _/_/      _/      _/_/_/_/  _/  _/_/        
_/        _/    _/  _/  _/  _/    _/  _/        _/      _/    _/  _/  _/           
 _/_/_/    _/_/    _/  _/  _/_/_/    _/        _/      _/    _/  _/  _/            
                                                                                   
  ColibriAir - Sensors to ThingsBoard
*/

// To create an account on ThingsBoard an your own dashboard with the free trial follow the steps bellow:
// Login to https://thingsboard.cloud/login
// Device groups > All > Add Device > Give a name > Transport Type "Default" 
// Device details > Manage credentials > Select MQTT Basic > Choose a Client ID, User Name and Password
// Now you can find your data in latest telemetry
// Create a dashboard > Add New Widget > Create New Widget > Select one like "Charts" > Timeseries Live Chart > Add Datasources 
// Type --> Entity > entity alias --> Create a new one, Filter type --> Single entity, Type --> Device and select your device > Then select the data you want to show

/****************************************
* Include linked libraries
****************************************/
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoJson.h>      // Sketch > manage library > Search for ArduinoJSON librairy
#include <Adafruit_VCNL4040.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SGP40.h>
#include <PubSubClient.h>

/****************************************
* SETTING PARAMETERS
****************************************/
// WiFi PSK
#define WIFI_SSID "XXXXX"  // The name of your Wi-Fi FE-User
#define WIFI_PASSWORD "XXXXXX"  // Your WI-Fi password

// WiFi EAP/WPA2
//#include "esp_wpa2.h"
//#define WIFI_SSID "XXXXX"  // The name of your Wi-Fi FE-User
//#define WIFI_USERNAME "XXXXXXX" //Your WIFI_USERNAME 
//#define WIFI_PASSWORD "XXXXXX"  // Your WI-Fi password


// MQTT parameters
#define CLIENT_ID "XXXXX" //The client_id you define in manage credentials
#define USERNAME "XXXXX" //Your username
#define PASS "XXXXX" //Your password
#define TOPIC_PUB "v1/devices/me/telemetry" //The topic of thingsboard to publish 
int status = WL_IDLE_STATUS;


// Creating objects
Adafruit_VCNL4040 vcnl = Adafruit_VCNL4040();
Adafruit_SHT31 sht = Adafruit_SHT31();
Adafruit_SGP40 sgp;
WiFiClient wifiClient;
PubSubClient client(wifiClient);


// Specify the pins to which the LEDs are connected
const int LED_BLUE = 25;
const int LED_GREEN = 26;
const int LED_RED = 27;

// Variables 
char mqttBroker[] = "colibri.ltfe.org";  //Broker of colibri cloud platform || "thingsboard.cloud" broker of thingsboard cloud platform
long ALS = 0;
float T = 0;
float RH = 0;
int VOC = 0;
int delayTime = 10 ;  
int counter = 0 ; 

// Offset parameters
const int offsetTemperature = 5;
const int offsetHumidity = 13;


/****************************************
* Setup function
 ****************************************/
void setup() {

// Initialize I2C and serial interface
  Wire.begin(16, 17);
  
  // Initialize digital outputs
  pinMode(LED_GREEN, OUTPUT);  
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // All LEDs off
  digitalWrite(LED_GREEN, HIGH); 
  digitalWrite(LED_BLUE, HIGH); 
  digitalWrite(LED_RED, HIGH); 

  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for serial console to open!

  Serial.println("### ColibriAir - Sensors to Thingsboard ###");

  connectWifi();
  
  client.setServer(mqttBroker,1883);
  client.setCallback(callback);


  // Initial connection to sensors
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
* Main Function
 ****************************************/
void loop() {
  
  if (WiFi.status() == WL_CONNECTED) { // if we are connected to eduroam network
    // counter = 0; //reset counter
    Serial.println("Wifi is still connected with IP: ");
    Serial.println(WiFi.localIP());   //inform user about his IP address
  } else if (WiFi.status() != WL_CONNECTED) { //if we lost connection, retry
    WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  }
  
  while (WiFi.status() != WL_CONNECTED) { //during lost connection, print dots
  delay(500);
  Serial.print(".");
  blinkLed(LED_RED);
  counter++;
    if (counter >= 60) { //30 seconds timeout - reset board
      ESP.restart();
      counter = 0 ; 
    }
  }
    
  // Read senosor values from VCNL4040
  long ALS = vcnl.getLux(); 

  // Read senosor values from SHT31
  float T = sht.readTemperature() - offsetTemperature;
  float RH = sht.readHumidity() + offsetHumidity;

  // Read senosor values from SGP40
  int VOC = sgp.measureVocIndex(T, RH);
  
  // Print the value to the serial interface
  printValuesToSerial();


  // Check if the VOC sensor is already measuring
  if (VOC < 1) {  
    blinkLed(LED_BLUE);
    delay(500);
  }
  else {  // The sensor is already measuring
  
    // Turns on the LED according to the air quality
    ledIndicator(VOC); 
  
    // Restore the MQTT connection in case it was lost
    if (!client.connected()) reconnect();
    client.loop();
      
    // Sending MQTT packages
    send("LightLevel",ALS); //send("<name you give to your telemetry>",<The value to send>)
    send("Temperature",T);
    send("Humidity",RH);
    send("VOC",VOC);
    
    delay(delayTime*1000);
  } 
  
}


/****************************************
* Additional features
****************************************/

// Function to turn on the LED
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
 
// Function for LED flash
void blinkLed(int LED) {
  digitalWrite(LED, LOW); 
  delay(50);
  digitalWrite(LED, HIGH); 
}

void connectWifi() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // WiFi PSK  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // WiFi EAP/WPA2
  //WiFi.mode(WIFI_STA);
  //esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_SSID, strlen(WIFI_SSID));
  //esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME));
  //esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD));
  //esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT();
  //esp_wifi_sta_wpa2_ent_enable(&config);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    blinkLed(LED_BLUE);
    if (counter >= 60) { //after 30 seconds timeout - reset board (on unsucessful connection)
    ESP.restart();
    }
  }  
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_GREEN, LOW);
  

}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to Thingsboard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect(CLIENT_ID, USERNAME, PASS) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
      }
  }
}

// Function for printing to the serial interface
void printValuesToSerial() {

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

// Function to send MQTT packets
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}


//Function to send the sensor value to ThingsBoard
bool send(char* asset, float payload) {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connected()) {
        DynamicJsonDocument doc(256);
        char JSONmessageBuffer[256];
        doc[asset] = payload;
        serializeJson(doc, JSONmessageBuffer);
        client.publish(TOPIC_PUB, JSONmessageBuffer, false);
        Serial.println(JSONmessageBuffer);
       Serial.println("--------Message Published to ThingsBoard (JSON)---------");
       blinkLed(LED_BLUE);
    } else {
        Serial.println("Can't publish message because you're not connected to ThingsBoard");
        return false;
    }
  } else {
    Serial.println("Can't publish message because you're not connected to WiFi");
    return false;
  }
}

bool send(char *asset, char* payload);
bool send(char *asset, int payload);
bool send(char *asset, long payload);
  

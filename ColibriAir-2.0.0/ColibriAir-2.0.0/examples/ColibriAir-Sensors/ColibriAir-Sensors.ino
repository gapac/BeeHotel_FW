/*                                                                                 
     _/_/_/            _/  _/  _/                  _/        _/_/    _/            
  _/          _/_/    _/      _/_/_/    _/  _/_/          _/    _/      _/  _/_/   
 _/        _/    _/  _/  _/  _/    _/  _/_/      _/      _/_/_/_/  _/  _/_/        
_/        _/    _/  _/  _/  _/    _/  _/        _/      _/    _/  _/  _/           
 _/_/_/    _/_/    _/  _/  _/_/_/    _/        _/      _/    _/  _/  _/            
                                                                                   
  ColibriAir - Sensors
*/

// Vključitev povezanih knjižnic
#include <Wire.h>
#include <Adafruit_VCNL4040.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_SGP40.h>

// Kreiranje objektov
Adafruit_VCNL4040 vcnl = Adafruit_VCNL4040();
Adafruit_SHT31 sht = Adafruit_SHT31();
Adafruit_SGP40 sgp;

// Določitev pinov kamor so vezane LED
const int LED_BLUE = 25;
const int LED_GREEN = 26;
const int LED_RED = 27;

// Zakasnitev med posameznimi meritvami
const int delayTime = 5; //sec

// Offset parameters
const int offsetTemperature = 5;
const int offsetHumidity = 13;

// Funkcija Setup se izvede le enkrat ko resetiramo napravo
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
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for serial console to open!

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

// Funkcija Loop se izvaja vedno znova in znova
void loop() {

  // Preberi vrednosti iz VCNL4040
  long ALS = vcnl.getLux(); 

  // Preberi vrednosti iz SHT31
  float T = sht.readTemperature() - offsetTemperature;
  float RH = sht.readHumidity() + offsetHumidity;

  // Preberi vrednosti iz SGP40
  int VOC = sgp.measureVocIndex(T, RH);

  // Vklopi LED glede na kvaliteto zraka
  ledIndicator(VOC);

  // Izpis vrednosti na serijski vmesnik
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

  // Zakasnitev med meritvami
  delay(delayTime*1000);
}

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

}

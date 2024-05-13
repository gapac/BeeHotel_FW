const int analogInPin = 34; // Analog input pin on ESP32

void setup() {
  Serial.begin(115200);
}

void loop() {
  int sensorValue = analogRead(analogInPin); // Read analog input

  float voltage = sensorValue * (3.3 / 4095); // Convert ADC value to voltage (3.3V reference, 12-bit resolution)

  Serial.print("Raw ADC Value: ");
  Serial.print(sensorValue);
  Serial.print(", Voltage: ");
  Serial.print(voltage, 3); // Print voltage with 3 decimal places
  Serial.println(" V");

  delay(1000); // Delay for 1 second
}

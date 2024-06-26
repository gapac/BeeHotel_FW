#include <HardwareSerial.h>

#define RXD2 16
#define TXD2 17

HardwareSerial RAKSerial(2);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize UART communication with RAK3172
  RAKSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(1000);

  // Test communication with RAK3172
  sendATCommand("AT+VER=?", 1000);

  // Send temperature data as an example
  int temperature = 1000; // Temperature value to send
  sendTemperatureData(temperature, 1000);
}

void loop() {
  // Put your main code here, to run repeatedly:
}

void sendATCommand(String cmd, const int timeout) {
  String response = "";
  Serial.print("Sending: ");
  Serial.println(cmd);

  RAKSerial.println(cmd);
  long int time = millis();

  while ((time + timeout) > millis()) {
    while (RAKSerial.available()) {
      char c = RAKSerial.read();
      response += c;
    }
  }
  Serial.print("Response: ");
  Serial.println(response);
}

void sendTemperatureData(int temperature, const int timeout) {
  String cmd = "AT+TEMP=" + String(temperature);
  sendATCommand(cmd, timeout);
}

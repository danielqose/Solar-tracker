#define BLYNK_TEMPLATE_ID "TMPL4M6Xzb0O1"
#define BLYNK_TEMPLATE_NAME "Sun Tracking Solar Panel"

#include <WiFi.h>
#include <WiFiClient.h>
#define BLYNK_PRINT Serial
#include "BlynkEdgent.h"

#include <ESP32Servo.h>
#include "DHT.h"

// Hardware Pins
#define DHTPIN 33       
#define DHTTYPE DHT22
int LDR1 = 35;      // TOP LDR
int LDR2 = 32;      // BOTTOM LDR
int servopin = 26;  // Servo signal
const int Analog_channel_pin = 34; // Solar panel voltage

// Objects & Variables
Servo servo1;
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

int currentServoAngle = 90;
int error = 5;
bool manualControl = false;

// --- Blynk Functions ---

BLYNK_WRITE(V0) { // Manual Angle Slider
  if (manualControl) {
    currentServoAngle = param.asInt();
    servo1.write(currentServoAngle);
  }
}

BLYNK_WRITE(V5) { // Manual Toggle Switch
  manualControl = param.asInt();
}

// --- Sensor Functions ---

void sendData() {
  // DHT22
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    Blynk.virtualWrite(V2, h);
    Blynk.virtualWrite(V3, t);
  }

  // Voltage
  int ADC_VALUE = analogRead(Analog_channel_pin);
  float v_adc = (ADC_VALUE * 3.3) / 4095.0;
  float voltage_value = v_adc * ((100000.0 + 56000.0) / 56000.0);
  Blynk.virtualWrite(V4, voltage_value);
}

void trackerLogic() {
  if (!manualControl) {
    int R1 = analogRead(LDR1);
    int R2 = analogRead(LDR2);
    int diff = abs(R1 - R2);

    if (diff > error) {
      if (R1 > R2) {
        currentServoAngle = max(0, currentServoAngle - 1);
      } else if (R1 < R2) {
        currentServoAngle = min(180, currentServoAngle + 1);
      }
      servo1.write(currentServoAngle);
      Blynk.virtualWrite(V1, currentServoAngle);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  servo1.attach(servopin);
  servo1.write(currentServoAngle);

  BlynkEdgent.begin();

  // Update sensors every 2 seconds, check sun every 50ms
  timer.setInterval(2000L, sendData);
  timer.setInterval(50L, trackerLogic);
}

void loop() {
  BlynkEdgent.run();
  timer.run();
}
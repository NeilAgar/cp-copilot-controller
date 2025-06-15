#include "switch_ESP32.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

NSGamepad Gamepad;
WiFiUDP udp;

// Wifi settings:
const char *ssid = "MarioCopilot";
const char *password = "12345678";
const int UDP_PORT = 4210;

// Pins:
int PIN_TOUCH = 4; // value touch pin
int PIN_POT = 14; // potentiometer pin (0-4095)

// Variables:
int joyX = 128; // 128 is center
int joyY = 128;
bool pcButtonB = false;

int baseline = 0;
bool isPressed = false;

// Kalman Filter Class
class SimpleKalmanFilter {
  private:
    float _err_measure;
    float _err_estimate;
    float _q;
    float _last_estimate;
    float _kalman_gain;

  public:
    SimpleKalmanFilter(float mea_e, float est_e, float q) {
      _err_measure = mea_e;
      _err_estimate = est_e;
      _q = q;
    }
    float updateEstimate(float mea) {
      _kalman_gain = _err_estimate / (_err_estimate + _err_measure);
      float current_estimate = _last_estimate + _kalman_gain * (mea - _last_estimate);
      _err_estimate =  (1.0 - _kalman_gain) * _err_estimate + fabs(_last_estimate - current_estimate) * _q;
      _last_estimate = current_estimate;
      return current_estimate;
    }
};

SimpleKalmanFilter kf(5.0, 5.0, 0.01); 

void setup() {
  Serial.begin(115200);
  Gamepad.begin();
  USB.begin();

  // Start Wi-Fi
  WiFi.softAP(ssid, password);
  udp.begin(UDP_PORT);

  // Take initial readings to find ground level
  long total = 0;
  for(int i=0; i<50; i++) {
    total += touchRead(PIN_TOUCH);
    delay(10);
  }
  baseline = total / 50.0;
}

void loop() {
  // 1. Listen for PC
  int packetSize = udp.parsePacket();
  if (packetSize) {
    uint8_t incomingPacket[3];
    udp.read(incomingPacket, 3);
    joyX = incomingPacket[0];
    joyY = incomingPacket[1];
    pcButtonB = (incomingPacket[2] == 1);
  }

  // 2. Read inputs
  int rawPot = analogRead(PIN_POT);
  int rawTouch = touchRead(PIN_TOUCH);

  // Smooth
  float smoothTouch = kf.updateEstimate(rawTouch);

  // Baseline
  if (abs(smoothTouch - baseline) < 5000) {
    baseline = (baseline * 0.95) + (smoothTouch * 0.05);
  }

  // Sensitivity
  float thresholdOffset = map(rawPot, 0, 4096, 10000, 80000); 

  // Trigger
  float triggerPoint = baseline + thresholdOffset;
  float releasePoint = baseline + (thresholdOffset * 0.8); // Hysteresis

  // Button presses
  if (!isPressed) {
    if (smoothTouch > triggerPoint) { // detects for spikes
      isPressed = true;
      Gamepad.press(NSButton_A);
    }
  } 
  else {
    // dropping below = release
    if (smoothTouch < releasePoint) {
      isPressed = false;
      Gamepad.release(NSButton_A);
    }
  }

  // 3. Combine
  
  // Apply Joystick values
  Gamepad.leftXAxis(joyX);
  Gamepad.leftYAxis(joyY);

  // Apply "Run" button
  if (pcButtonB) {
    Gamepad.press(NSButton_B);
  } else {
    // Only release B if we are NOT holding it
    Gamepad.release(NSButton_B);
  }

  // Send Everything
  Gamepad.loop();
  
  delay(10);
}
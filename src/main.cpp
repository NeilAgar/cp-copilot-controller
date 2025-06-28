#include "switch_ESP32.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

NSGamepad Gamepad;
WiFiUDP udp;

// --- CONFIG ---
const char *ssid = "MarioCopilot";
const char *password = "12345678";
const int UDP_PORT = 4210;

// --- PINS ---
const int PIN_TOUCH = 4;
const int PIN_POT = 14;

// --- VARIABLES ---
// UDP Data Containers
int udp_LX = 128;
int udp_LY = 128;
int udp_RX = 128;
int udp_RY = 128;
uint8_t udp_Btns = 0; // Bitmask

unsigned long lastPacketTime = 0;
float steadyBaseline = 0;
bool isLocalPressed = false;
float smoothedTouch = 0;

// --- KALMAN FILTER ---
class SimpleKalmanFilter {
  private:
    float _err_measure, _err_estimate, _q, _last_estimate, _kalman_gain;
  public:
    SimpleKalmanFilter(float mea_e, float est_e, float q) {
      _err_measure = mea_e; _err_estimate = est_e; _q = q;
    }
    float updateEstimate(float mea) {
      _kalman_gain = _err_estimate / (_err_estimate + _err_measure);
      float current_estimate = _last_estimate + _kalman_gain * (mea - _last_estimate);
      _err_estimate = (1.0 - _kalman_gain) * _err_estimate + fabs(_last_estimate - current_estimate) * _q;
      _last_estimate = current_estimate;
      return current_estimate;
    }
};

SimpleKalmanFilter kf(5.0, 5.0, 0.01); 

void setup() {
  // 1. Init System
  Gamepad.begin();
  USB.begin();
  
  // 2. Init Network
  WiFi.softAP(ssid, password);
  udp.begin(UDP_PORT);

  // 3. Calibrate Local Sensor
  long total = 0;
  for(int i=0; i<50; i++) {
    total += touchRead(PIN_TOUCH);
    delay(5);
  }
  steadyBaseline = total / 50.0;
  smoothedTouch = steadyBaseline;
}

void loop() {
  // 1. Receive PC Data (5 Bytes)
  int packetSize = udp.parsePacket();
  if (packetSize >= 5) {
    uint8_t packet[5];
    udp.read(packet, 5);
    
    udp_LX = packet[0];
    udp_LY = packet[1];
    udp_RX = packet[2];
    udp_RY = packet[3];
    udp_Btns = packet[4];
    
    lastPacketTime = millis();
  }

  // If PC disconnects (>500ms), reset everything
  if (millis() - lastPacketTime > 500) {
    udp_LX = 128; udp_LY = 128;
    udp_RX = 128; udp_RY = 128;
    udp_Btns = 0;
  }

  // 2. Local Sensor Logic
  int rawPot = analogRead(PIN_POT);
  float rawTouch = touchRead(PIN_TOUCH);
  smoothedTouch = kf.updateEstimate(rawTouch);

  // Drift Correction
  if (abs(smoothedTouch - steadyBaseline) < 5000) {
    steadyBaseline = (steadyBaseline * 0.95) + (smoothedTouch * 0.05);
  }

  // Threshold Calculation
  float thresholdOffset = map(rawPot, 0, 4096, 1000, 8000);
  float triggerPoint = steadyBaseline + thresholdOffset;
  float releasePoint = steadyBaseline + (thresholdOffset * 0.8);

  // 3. Apply

  // A. Local User Input
  // don't override the PC if it presses A too
  if (!isLocalPressed) {
    if (smoothedTouch > triggerPoint) {
      isLocalPressed = true;
      Gamepad.press(NSButton_A);
    }
  } else {
    if (smoothedTouch < releasePoint) {
      isLocalPressed = false;
      // Only release if the PC isnt also holding A (Bit 0)
      if ((udp_Btns & 1) == 0) {
        Gamepad.release(NSButton_A);
      }
    }
  }

  // B. PC Joystick Inputs
  Gamepad.leftXAxis(udp_LX);
  Gamepad.leftYAxis(udp_LY);
  Gamepad.rightXAxis(udp_RX);
  Gamepad.rightYAxis(udp_RY);

  // C. PC Button Inputs Bitmask
  
  // Bit 0: A (Handled in Local logic above to merge signals)
  if ((udp_Btns & 1) && !isLocalPressed) Gamepad.press(NSButton_A);
  
  // Bit 1: B
  if (udp_Btns & 2) Gamepad.press(NSButton_B); else Gamepad.release(NSButton_B);
  
  // Bit 2: X
  if (udp_Btns & 4) Gamepad.press(NSButton_X); else Gamepad.release(NSButton_X);
  
  // Bit 3: Y
  if (udp_Btns & 8) Gamepad.press(NSButton_Y); else Gamepad.release(NSButton_Y);
  
  // Bit 4: L
  if (udp_Btns & 16) Gamepad.press(NSButton_LeftTrigger); else Gamepad.release(NSButton_LeftTrigger);
  
  // Bit 5: R
  if (udp_Btns & 32) Gamepad.press(NSButton_RightTrigger); else Gamepad.release(NSButton_RightTrigger);
  
  // Bit 6: ZL
  if (udp_Btns & 64) Gamepad.press(NSButton_LeftThrottle); else Gamepad.release(NSButton_LeftThrottle);
  
  // Bit 7: ZR
  if (udp_Btns & 128) Gamepad.press(NSButton_RightThrottle); else Gamepad.release(NSButton_RightThrottle);

  // 4. Send
  Gamepad.loop();
  delay(5);
}
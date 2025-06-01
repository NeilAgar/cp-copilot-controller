#include "switch_ESP32.h"
#include <Arduino.h>
NSGamepad Gamepad;

int baseline = 0;

// -- PINS --
int PIN_TOUCH = 4; // value touch pin
int PIN_POT = 14; // potentiometer pin (0-4095)

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

const int WINDOW_SIZE = 20;
float history[WINDOW_SIZE];
int histIdx = 0;
bool isPressed = false;

void setup() {
  Gamepad.begin();
  USB.begin();

  // Take initial readings to find ground level
  long total = 0;
  for(int i=0; i<50; i++) {
    total += touchRead(PIN_TOUCH);
    delay(10);
  }
  baseline = total / 50.0;

  // Fill buffer
  for(int i=0; i<WINDOW_SIZE; i++) {
    history[i] = baseline;
  }
}

// boolean pressed = false;

void loop() {
  // 1. Read
  int rawPot = analogRead(PIN_POT);
  int rawTouch = touchRead(PIN_TOUCH);

  // 2. Smooth
  float smoothTouch = kf.updateEstimate(rawTouch);

  // 3. Baseline
  if (abs(smoothTouch - baseline) < 5000) {
    baseline = (baseline * 0.95) + (smoothTouch * 0.05);
  }

  // 4. Sensitivity
  float thresholdOffset = map(rawPot, 0, 4096, 10000, 80000); 

  // 5. Trigger
  float triggerPoint = baseline + thresholdOffset;
  float releasePoint = baseline + (thresholdOffset * 0.8); // Hysteresis

  // 6. Button presses
  if (!isPressed) {
    if (smoothTouch > triggerPoint) { // detects for spikes
      isPressed = true;
      Gamepad.press(NSButton_A);
      Gamepad.loop();
    }
  } 
  else {
    // dropping below = release
    if (smoothTouch < releasePoint) {
      isPressed = false;
      Gamepad.release(NSButton_A);
      Gamepad.loop();
    }
  }

  delay(10);
}

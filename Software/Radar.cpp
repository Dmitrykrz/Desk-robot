#include "Radar.h"

Radar::Radar(int rx_pin, HardwareSerial& serialPort)
  : rxPin(rx_pin), serial(&serialPort), historyIndex(0), debounced(false), historySize(100),
    rangeSum(0), rangeCount(0), rangeAverageSize(10), rangeAverage(0) {
  // Initialize history to all false
  for (int i = 0; i < RADAR_HISTORY_SIZE_MAX; i++) {
    history[i] = false;
  }
}

void Radar::begin(long baudRate) {
  serial->begin(baudRate, SERIAL_8N1, rxPin, -1);
}

void Radar::update() {
  // Note: Radar sends data at approximately 10 Hz (~105ms interval)
  // So historySize of 100 = ~10 seconds of data

  if (serial->available()) {
    String receivedString = serial->readStringUntil('\n');
    receivedString.trim();

    if (receivedString.length() > 0) {
      // Check for presence detection (ON/OFF)
      if (receivedString.equals("ON") || receivedString.equals("OFF")) {
        bool currentReading = receivedString.equals("ON");

        // Add to circular buffer
        history[historyIndex] = currentReading;
        historyIndex = (historyIndex + 1) % historySize;

        // Check if all history is false (up to current historySize)
        bool allFalse = true;
        for (int i = 0; i < historySize; i++) {
          if (history[i]) {
            allFalse = false;
            break;
          }
        }

        // Asymmetric debouncing: fast ON, slow OFF
        if (currentReading) {
          debounced = true;
        } else if (allFalse) {
          debounced = false;
        }
      }
      // Check for range data (e.g., "Range 72")
      else if (receivedString.startsWith("Range ")) {
        int rangeValue = receivedString.substring(6).toInt();

        // Accumulate for averaging
        rangeSum += rangeValue;
        rangeCount++;

        // Calculate average when we have enough samples
        if (rangeCount >= rangeAverageSize) {
          rangeAverage = rangeSum / rangeCount;
          rangeSum = 0;
          rangeCount = 0;
        }
      }
    }
  }
}

bool Radar::check() {
  // Returns most recent reading from history
  int prevIndex = (historyIndex - 1 + historySize) % historySize;
  return history[prevIndex];
}

bool Radar::check_debounced() {
  return debounced;
}

void Radar::set_history_size(int size) {
  // Validate size
  if (size < 1) size = 1;
  if (size > RADAR_HISTORY_SIZE_MAX) size = RADAR_HISTORY_SIZE_MAX;

  historySize = size;

  // Reset history index to avoid issues with wraparound
  historyIndex = 0;

  // Clear history when changing size
  for (int i = 0; i < RADAR_HISTORY_SIZE_MAX; i++) {
    history[i] = false;
  }

  Serial.print("Radar history size set to: ");
  Serial.println(historySize);
}

int Radar::get_history_size() {
  return historySize;
}

int Radar::get_range() {
  return rangeAverage;
}

void Radar::set_range_average_size(int size) {
  if (size < 1) size = 1;
  if (size > 100) size = 100;

  rangeAverageSize = size;

  // Reset averaging counters
  rangeSum = 0;
  rangeCount = 0;

  Serial.print("Radar range average size set to: ");
  Serial.println(rangeAverageSize);
}

int Radar::get_range_average_size() {
  return rangeAverageSize;
}

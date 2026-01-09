#ifndef RADAR_H
#define RADAR_H

#include <Arduino.h>

#define RADAR_HISTORY_SIZE_MAX 200

class Radar {
private:
  HardwareSerial* serial;
  int rxPin;
  bool history[RADAR_HISTORY_SIZE_MAX];
  int historyIndex;
  bool debounced;
  int historySize;  // Current active history size (default 100 = ~10 seconds at 10Hz)

  // Range/distance tracking
  int rangeSum;           // Sum of range values for averaging
  int rangeCount;         // Number of range samples collected
  int rangeAverageSize;   // How many samples to average (default 10)
  int rangeAverage;       // Current averaged range value

public:
  Radar(int rx_pin, HardwareSerial& serialPort);
  void begin(long baudRate);
  void update();
  bool check();
  bool check_debounced();
  void set_history_size(int size);
  int get_history_size();
  int get_range();
  void set_range_average_size(int size);
  int get_range_average_size();
};

#endif // RADAR_H

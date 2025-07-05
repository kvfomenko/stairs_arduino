#include <EEPROM.h>

struct Settings {
  int work_mode;
  int random_mode;
  float lux_coef;
  float threshold_voltage[5];
};

void saveSettings(int work_mode, int random_mode, float lux_coef, float threshold_voltage[5]) {
  Settings s;
  s.work_mode = work_mode;
  s.random_mode = random_mode;
  s.lux_coef = lux_coef;
  for (int i = 0; i < 5; i++) {
    s.threshold_voltage[i] = threshold_voltage[i];
  }
  EEPROM.put(0, s);
}

void loadSettings(int& work_mode, int& random_mode, float& lux_coef, float threshold_voltage[5]) {
  Settings s;
  EEPROM.get(0, s);
  lux_coef = s.lux_coef;
  work_mode = s.work_mode;
  random_mode = s.random_mode;
  for (int i = 0; i < 5; i++) {
    threshold_voltage[i] = s.threshold_voltage[i];
  }
}

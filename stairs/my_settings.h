#include <EEPROM.h>

struct Settings {
  int work_mode;
};

void saveSettings(int work_mode) {
  Settings s;
  s.work_mode = work_mode;
  EEPROM.put(0, s);
}

void loadSettings(int& work_mode) {
  Settings s;
  EEPROM.get(0, s);
  work_mode = s.work_mode;
}


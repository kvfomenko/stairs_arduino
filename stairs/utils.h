
#pragma once           // защищает от двойного включения
#include "my_time.h"

#define DEBUG_ENABLED false
#define DEBUG_TIME true

void log_setup(int speed) {
  Serial.begin(speed);
}

void log(String message) {
  if (DEBUG_ENABLED) {
    unsigned long epoch = get_epoch_time();
    if (DEBUG_TIME) {
      String tm = getFormattedDateTime(epoch);
      Serial.println(tm + "> " + message);
    } else {
      Serial.println(message);
    }
    //log_matrix(message);
    //Serial.println(message);
  }
}

int my_random(int min, int max) {
    return min + (random() % (max - min + 1));
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int fmap(int x, int in_min, int in_max, int out_min, int out_max) {
  return (int)((float)x - (float)in_min) * ((float)out_max - (float)out_min) / ((float)in_max - (float)in_min) + (float)out_min;
}


int intdiv(int a, int b) {
    if (b == 0) return 0;
    return a / b;
}

CRGB hslToCRGB(float h, float s, float l) {
  float r, g, b;

  auto hue2rgb = [](float p, float q, float t) -> float {
    if (t < 0) t += 1;
    if (t > 1) t -= 1;
    if (t < 1.0 / 6) return p + (q - p) * 6 * t;
    if (t < 1.0 / 2) return q;
    if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
    return p;
  };

  if (s == 0) {
    r = g = b = l; // achromatic
  } else {
    float q = (l < 0.5) ? (l * (1 + s)) : (l + s - l * s);
    float p = 2 * l - q;
    r = hue2rgb(p, q, h + 1.0 / 3);
    g = hue2rgb(p, q, h);
    b = hue2rgb(p, q, h - 1.0 / 3);
  }

  return CRGB((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
}

String CRGBtoString(CRGB color) {
  String l = String(color.r) + ":" + String(color.g) + ":" + String(color.b);
  return l;
}

bool startsWithIgnoreCase(const char* text, const char* prefix) {
  while (*prefix) {
    if (!*text || tolower(*text) != tolower(*prefix)) return false;
    text++;
    prefix++;
  }
  return true;
}

bool startsWithIgnoreCase(const String& text, const String& prefix) {
  if (prefix.length() > text.length()) return false;

  for (unsigned int i = 0; i < prefix.length(); i++) {
    if (tolower(text.charAt(i)) != tolower(prefix.charAt(i))) {
      return false;
    }
  }
  return true;
}

float get_analog_voltage(int analogPin) {
  int analogValue = analogRead(analogPin);  // 0–4095
  float voltage = (float)analogValue * 3.3 / 4095; // перевод в вольты
  return voltage;
}


float get_analog_voltage_from_value(int analogValue) {
  float voltage = (float)analogValue * 3.3 / 4095; // перевод в вольты
  return voltage;
}

template <size_t N>
int findInIndex(const String& msgText, const String (&arr)[N]) {
  for (int i = 0; i < N; ++i) {
    if (arr[i] == msgText) return i;
  }
  return 0;
}

String rgb_to_str(CRGB col) {
  return String(col.r) + "." + String(col.g) + "." + String(col.b);
}

/*
int findInIndex2(const String& msgText, const String arr[]) {
  for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
    if (arr[i] == msgText) {
      return i;  // Возвращаем индекс, если нашли совпадение
    }
  }
  return 0;  // Возвращаем 0, если не нашли совпадение
}
*/
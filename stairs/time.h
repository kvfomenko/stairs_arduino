
#include "WiFiUdp.h"
#include "NTPClient.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600); // GMT+3

long seconds_after_last_update = 0; //seconds
int update_from_server_interval = 3*60*60*1000; //ms
int last_update_from_server = millis();



String getFormattedDateTime(unsigned long epochTime) {
  const int secondsPerMinute = 60;
  const int secondsPerHour = 3600;
  const int secondsPerDay = 86400;
  const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};

  // 1970-01-01
  int year = 1970;
  int month = 0;
  int day = 0;
  int hour, minute, second;

  unsigned long seconds = epochTime;

  // ---- time ----
  second = seconds % 60;
  seconds /= 60;
  minute = seconds % 60;
  seconds /= 60;
  hour = seconds % 24;
  unsigned long days = seconds / 24;

  // ---- date ----
  while (true) {
    bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    int daysInYear = leap ? 366 : 365;
    if (days >= daysInYear) {
      days -= daysInYear;
      year++;
    } else {
      break;
    }
  }

  bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
  for (int i = 0; i < 12; i++) {
    int dim = daysInMonth[i];
    if (i == 1 && leap) dim = 29;
    if (days >= dim) {
      days -= dim;
    } else {
      month = i + 1;
      day = days + 1;
      break;
    }
  }

  // ---- format ----
  char buf[20];
  sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
  return String(buf);
}

void update_time_from_server() {
  timeClient.update();
  seconds_after_last_update = millis() / 1000; // time at last timeClient.update()

  long epochTime = timeClient.getEpochTime();
  String dt = getFormattedDateTime(epochTime);
  Serial.println("update_time_from_server.. " + dt);
}

void time_setup() {
  timeClient.begin();

  for (int i=0; i<100; i++) {
    Serial.print(".");
    update_time_from_server();
    long epochTime = timeClient.getEpochTime();
    if (epochTime > 1700000000) {
      Serial.println(".");
      return;
    }
    delay(100);
  }

  Serial.println("ERROR: cannot update time from server");
}

long get_epoch_time() {
  if (millis() - last_update_from_server >= update_from_server_interval) {
    last_update_from_server = millis();
    update_time_from_server();
  }

  long epochTime = timeClient.getEpochTime();
  //Serial.println("epochTime " + String(epochTime));
  //correction
  epochTime = epochTime + (millis() / 1000 - seconds_after_last_update);
  return epochTime;
}


/*
demo

void loop() {
  long epochTime = get_epoch_time();
  String dt = getFormattedDateTime(epochTime);
  Serial.println(dt);
  delay(1000);
}
*/

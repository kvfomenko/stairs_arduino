
const double LATITUDE = 50.34; //ukraine
const double LONGITUDE = 30.29;
const int UTC_OFFSET = 2; //ukraine
const int UTC_OFFSET_SUMMER = 3; //ukraine summer

const int secondsPerMinute = 60;
const int secondsPerHour = 3600;
const int secondsPerDay = 86400;
const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};

#include "WiFiUdp.h"
#include "NTPClient.h"
#include "SolarCalculator.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0 * secondsPerHour); // GMT+0 by default - timezone correction inside get_epoch_time

long update_from_server_interval = 3 * secondsPerHour * 1000; //ms  - update from NTP each 3 hours
long last_update_from_server = millis();
int timezone_offset = 0; // hours
double transit, sunrise, sunset; // hours with decimals


String getFormattedDateTime(unsigned long epochTime) {

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

String hoursToString(double h) {
  char buf[6];

  int m = int(round(h * 60));
  int hr = (m / 60) % 24;
  int mn = m % 60;

  buf[0] = (hr / 10) % 10 + '0';
  buf[1] = (hr % 10) + '0';
  buf[2] = ':';
  buf[3] = (mn / 10) % 10 + '0';
  buf[4] = (mn % 10) + '0';
  buf[5] = '\0';
  return String(buf);
}

int weekday(int y, int m, int d) {
  // Zeller's Congruence
  if (m < 3) {
    m += 12;
    y -= 1;
  }
  int K = y % 100;
  int J = y / 100;
  int f = d + 13*(m + 1)/5 + K + K/4 + J/4 + 5*J;
  return ((f + 6) % 7) + 1; // Sunday = 1, Monday = 2, ..., Saturday = 7
}

bool isSummerTime(int year, int month, int day) {
  if (month < 3 || month > 10) return false;  // Янв–Фев и Нояб–Дек = зимнее время
  if (month > 3 && month < 10) return true;   // Апр–Сент = летнее время

  // В марте и октябре — проверяем последнее воскресенье
  int lastSunday = 31;
  while (weekday(year, month, lastSunday) != 1) {  // 1 = Sunday
    lastSunday--;
  }

  if (month == 3) return day > lastSunday;         // В марте — после последнего воскресенья = летнее
  if (month == 10) return day < lastSunday;        // В октябре — до последнего воскресенья = летнее

  return false;
}

void calc_timezone_offset() {
    time_t epochTime = timeClient.getEpochTime(); // time in seconds at last timeClient.update()
    //Serial.println("epochTime " + String(epochTime));
    struct tm* tm = localtime(&epochTime);
    int year = tm->tm_year + 1900;
    int month = tm->tm_mon + 1;
    int day = tm->tm_mday;
    //Serial.println("=== " + String(year) + "-" + String(month) + "-" + String(day));

    timezone_offset = isSummerTime(year, month, day) ? UTC_OFFSET_SUMMER : UTC_OFFSET;
    Serial.println("timezone: " + String(timezone_offset));

    calcSunriseSunset(year, month, day, LATITUDE, LONGITUDE, transit, sunrise, sunset);
    Serial.println("sunrise: " + hoursToString(sunrise + timezone_offset) + "..." + hoursToString(sunset + timezone_offset));
}

void update_time_from_server() {
  timeClient.update();
  calc_timezone_offset();

  time_t epochTime = timeClient.getEpochTime(); // time in seconds at last timeClient.update()
  epochTime = epochTime + timezone_offset * secondsPerHour;

  String dt = getFormattedDateTime(epochTime);
  Serial.println("updated time from server.. " + dt);
}

void time_setup() {
  timeClient.begin();

  for (int i=0; i<100; i++) {
    update_time_from_server();
    time_t epochTime = timeClient.getEpochTime();
    if (epochTime > 1700000000) {
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

  time_t epochTime = timeClient.getEpochTime(); // time in seconds at last timeClient.update()
  epochTime = epochTime + timezone_offset * secondsPerHour;
  return epochTime;
}

bool is_day_now(time_t epochTime) {
  // Получаем количество секунд с начала суток по локальному времени
  int secondsOfDay = epochTime % secondsPerDay;
  double hourNow = secondsOfDay / secondsPerHour;

  // Сравниваем с интервалом между рассветом и закатом
  return (hourNow >= sunrise + timezone_offset && hourNow < sunset + timezone_offset);
}

String current_time_string() {
  long epoch = get_epoch_time();
  return getFormattedDateTime(epoch);
}



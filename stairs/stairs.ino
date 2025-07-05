#pragma once           // защищает от двойного включения

#define PORT_SPEED 250000 //115200

#include "my_settings.h"

//mic
#include "mic.h"

//light
#include "light_sens.h"

#define FASTLED_INTERNAL        // Отключаем внутренние конфликты FastLED
#include <FastLED.h>

#include "ArduinoGraphics.h"
#ifdef PORT
  #undef PORT
#endif
#include "Arduino_LED_Matrix.h"
#include "utils.h"
#include "worms.h"
#include "led_anim.h"

#define LED_PIN_1  D1
#define LED_PIN_2  D2
#define LED_PIN_3  D3
#define LED_PIN_4  D4

ArduinoLEDMatrix matrix;
#define SHOW_EACH_FRAME 10

//wifi
#include "credentials.h"
#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
int status = WL_IDLE_STATUS;

//tg
#include <AsyncTelegram2.h>
WiFiSSLClient client;
AsyncTelegram2 myBot(client);
const char* token = TG_TOKEN;
ReplyKeyboard myReplyKbd;
TBMessage msg;
String msgText = "";
int last_message_id = 0;
int tg_loop_interval = 500;

int last_millis = millis();
int last_tg_millis = millis();
int last_br_millis = millis();
int last_mic_millis = millis();
int last_sensor_millis = millis();
int last_hist_millis = millis();
int last_matrix_millis = millis();


void log_matrix(String text) {
  String text2 = "  " + text + "  ";

  matrix.beginDraw();
  matrix.textScrollSpeed(40);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text2.c_str());
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void text_matrix(String text) {
  matrix.beginDraw();
  matrix.stroke(128);          // макс. яркость = 255
  matrix.text(text + "   ");
  matrix.endDraw();
}


void brightness_loop() {
  int new_brightness;
  if (LIGHT_SENSOR_ENABLE) {

    lux = get_lux();
    lux = lux * lux_coef;

    if (lux > 32) {
      new_brightness = 255;
    } else if (lux > 16) {
      new_brightness = 128;
    } else if (lux > 8) {
      new_brightness = 64;
    } else if (lux > 4) {
      new_brightness = 32;
    } else {
      new_brightness = 16;
    }

  } else {
    long epoh = get_epoch_time();
    if (is_day_now(epoh)) {
      new_brightness = DAY_BRIGHTNESS;
    } else {
      new_brightness = NIGHT_BRIGHTNESS;
    }
  }

  if (new_brightness != brightness) {
    brightness = new_brightness;
    FastLED.setBrightness(brightness);
    log("brightness:" + String(brightness) + " lux:" + String(lux));
  }
}

void wifi_setup() {

    if (WiFi.status() == WL_NO_MODULE) {
      log("WiFi module failed!");
      while (true);
    }
    
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
      log("WiFi ver: " + fv);
    }

    while (status != WL_CONNECTED) {
      log("Attempting to connect to SSID: " + String(ssid));
      //log(ssid);
      status = WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
      }
      Serial.println(".");
      log("WiFi connected");
    }
    while (WiFi.localIP() == "0.0.0.0") {
      Serial.print(".");
      delay(300);
    }
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
}

void tg_setup() {
    myBot.setUpdateTime(500);
    myBot.setTelegramToken(token);

    myBot.begin() ? log("BOT OK") : log("BOT NOK");
    Serial.print("Bot name: @");
    Serial.println(myBot.getBotName());
    log("Telegram connected");

    myReplyKbd.addButton("ALWAYS-OFF");
    myReplyKbd.addButton("ALWAYS-ON");
    myReplyKbd.addButton("NIGHT-ON");
    myReplyKbd.addButton("SENSORS");
    myReplyKbd.addButton("MUSIC");
    myReplyKbd.addRow();
    myReplyKbd.addButton("RND-OFF");
    myReplyKbd.addButton("RND-M");
    myReplyKbd.addButton("RND-C");
    myReplyKbd.addButton("RND-M-C");
    myReplyKbd.addRow();
    myReplyKbd.addButton("RANGE");
    myReplyKbd.addButton("BRIGHTNESS");
    myReplyKbd.addButton("MODE");
    myReplyKbd.addButton("COLORS");
    myReplyKbd.addRow();
    myReplyKbd.addButton("TOP");
    myReplyKbd.addButton("BOTTOM");

    myReplyKbd.enableResize();

    delay(1000); //Pause before clearing messages
    myBot.setUpdateTime(20000);

    while (myBot.getNewMessage(msg)) {
      log("Skipping old message: " + msg.text);
      delay(1000);
    }
    log("Old messages cleared!");

    //start_animation();
}


void led_setup() {
    FastLED.addLeds<WS2811, LED_PIN_1, BRG>(leds, 0,                    POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_2, BRG>(leds, POINTS_PER_STEP * 4,  POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_3, BRG>(leds, POINTS_PER_STEP * 8,  POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_4, BRG>(leds, POINTS_PER_STEP * 12, POINTS_PER_STEP * 4);

    FastLED.setBrightness(brightness);
    FastLED.clear();
    FastLED.show();
}

void tg_loop() {
    log("in tg_loop");
    myBot.setUpdateTime(tg_loop_interval);

    while (myBot.getNewMessage(msg)) {
      msgText = msg.text;
      log("Message #" + String(msg.messageID) + ": " + msgText);

      if (last_message_id != msg.messageID) {
        last_message_id = msg.messageID;
        //myBot.sendMessage(msg, ">> " + msgText, "");

        if (msgText.equalsIgnoreCase("/start")) {
          log("keyboard activated");
          myBot.sendMessage(msg, "keyboard activated", myReplyKbd);
          msgText = "";

        } else if (msgText.equalsIgnoreCase("BOTTOM")) {
          log("set BOTTOM");
          text_matrix("B");
          myBot.sendMessage(msg, "start animation BOTTOM " + String(animation_mode));
          set_direction(UP);
          start_animation();

        } else if (msgText.equalsIgnoreCase("TOP")) {
          log("set TOP");
          text_matrix("T");
          myBot.sendMessage(msg, "start animation TOP " + String(animation_mode));
          set_direction(DOWN);
          start_animation();

        } else if (msgText.equalsIgnoreCase("BRIGHTNESS")) {
          long epoch = get_epoch_time();
          String dt = getFormattedDateTime(epoch);
          brightness_loop();
          myBot.sendMessage(msg, "brightness: " + String(brightness) // + "\nLUX: " + String(lux) + "\n/lux_coef_set " + String(lux_coef)
          + "\ndatetime:" + dt + " " + timezone_offset + "TZ"
          + "\nday: " + hoursToString(sunrise + timezone_offset) + "..." + hoursToString(sunset + timezone_offset)
          + "\n/day_brightness " + DAY_BRIGHTNESS
          + "\n/night_brightness " + NIGHT_BRIGHTNESS
          );

        } else if (startsWithIgnoreCase(msgText, "/day_brightness")) {
          DAY_BRIGHTNESS = msgText.substring(strlen("/day_brightness"), strlen("/day_brightness")+4).toInt();
          long epoch = get_epoch_time();
          String dt = getFormattedDateTime(epoch);
          brightness_loop();
          myBot.sendMessage(msg, "brightness: " + String(brightness) // + "\nLUX: " + String(lux) + "\n/lux_coef_set " + String(lux_coef)
          + "\ndatetime:" + dt + " " + timezone_offset + "TZ"
          + "\nday: " + hoursToString(sunrise + timezone_offset) + "..." + hoursToString(sunset + timezone_offset)
          + "\n/day_brightness " + DAY_BRIGHTNESS
          + "\n/night_brightness " + NIGHT_BRIGHTNESS
          );
        } else if (startsWithIgnoreCase(msgText, "/night_brightness")) {
          NIGHT_BRIGHTNESS = msgText.substring(strlen("/night_brightness"), strlen("/night_brightness")+4).toInt();
          long epoch = get_epoch_time();
          String dt = getFormattedDateTime(epoch);
          brightness_loop();
          myBot.sendMessage(msg, "brightness: " + String(brightness) // + "\nLUX: " + String(lux) + "\n/lux_coef_set " + String(lux_coef)
          + "\ndatetime:" + dt + " " + timezone_offset + "TZ"
          + "\nday: " + hoursToString(sunrise + timezone_offset) + "..." + hoursToString(sunset + timezone_offset)
          + "\n/day_brightness " + DAY_BRIGHTNESS
          + "\n/night_brightness " + NIGHT_BRIGHTNESS
          );
         
        /*} else if (startsWithIgnoreCase(msgText, "/lux_coef_set")) {
          lux_coef = msgText.substring(strlen("/lux_coef_set"), strlen("/lux_coef_set")+4).toFloat();
          brightness_loop();
          myBot.sendMessage(msg, "brightness\nLEVEL: " + String(brightness) + "\nLUX: " + String(lux) + "\n/lux_coef_set " + String(lux_coef));*/

        } else if (msgText.equalsIgnoreCase("RANGE")) {
          myBot.sendMessage(msg, 
            "avgTop:           " + String(avg_voltage_pin[3]) + "V " + String(avg_voltage_pin[4]) + "V "
            + "\nlastTop:         " + String(last_start_voltage_sensor[3]) + "V " + String(last_start_voltage_sensor[4]) + "V "
            + "\n/range_set_threshold3 " + String(THRESHOLD_VOLTAGE[3])
            + "\n/range_set_threshold4 " + String(THRESHOLD_VOLTAGE[4])
            + "\n\navgBottom:   " + String(avg_voltage_pin[1]) + "V " + String(avg_voltage_pin[2]) + "V "
            + "\nlastBottom: " + String(last_start_voltage_sensor[1]) + "V " + String(last_start_voltage_sensor[2]) + "V "
            + "\n/range_set_threshold1 " + String(THRESHOLD_VOLTAGE[1])
            + "\n/range_set_threshold2 " + String(THRESHOLD_VOLTAGE[2])
            + "\n\ndistance_top:    " + String(distance_top) + "\ndistance_bottom: " + String(distance_bottom)
            + "\n\nLAST_DIRECTION: " + String(last_direction) + "\nNEXT_DIRECTION: " + String(next_direction)
            + "\n\n/track_sensors_during_animation_after " + String(track_sensors_during_animation_after)
            );

        } else if (startsWithIgnoreCase(msgText, "/track_sensors_during_animation_after")) {
          String val = msgText.substring(strlen("/track_sensors_during_animation_after"), strlen("/track_sensors_during_animation_after")+4);
          log("set " + msgText + " val:" + val);
          track_sensors_during_animation_after = val.toInt();

        /*} else if (startsWithIgnoreCase(msgText, "/range_set")) {
          String val;
          
          if (startsWithIgnoreCase(msgText, "/range_set_top_min")) {
            val = msgText.substring(strlen("/range_set_top_min"), strlen("/range_set_top_min")+4);
            log("set " + msgText + " val:" + val);
            distance_top_min = val.toInt();
          } else if (startsWithIgnoreCase(msgText, "/range_set_top_max")) {
            val = msgText.substring(strlen("/range_set_top_max"), strlen("/range_set_top_max")+4);
            log("set " + msgText + " val:" + val);
            distance_top_max = val.toInt();
          } else if (startsWithIgnoreCase(msgText, "/range_set_bottom_min")) {
            val = msgText.substring(strlen("/range_set_bottom_min"), strlen("/range_set_bottom_min")+4);
            log("set " + msgText + " val:" + val);
            distance_bottom_min = val.toInt();
          } else if (startsWithIgnoreCase(msgText, "/range_set_bottom_max")) {
            val = msgText.substring(strlen("/range_set_bottom_max"), strlen("/range_set_bottom_max")+4);
            log("set " + msgText + " val:" + val);
            distance_bottom_max = val.toInt();
          } else */
          if (startsWithIgnoreCase(msgText, "/range_set_threshold1")) {
            val = msgText.substring(strlen("/range_set_threshold1"), strlen("/range_set_threshold1")+5); // 1.35
            log("set " + msgText + " val:" + val);
            THRESHOLD_VOLTAGE[1] = val.toFloat();
          } else if (startsWithIgnoreCase(msgText, "/range_set_threshold2")) {
            val = msgText.substring(strlen("/range_set_threshold2"), strlen("/range_set_threshold2")+5); // 1.35
            log("set " + msgText + " val:" + val);
            THRESHOLD_VOLTAGE[2] = val.toFloat();
          } else if (startsWithIgnoreCase(msgText, "/range_set_threshold3")) {
            val = msgText.substring(strlen("/range_set_threshold3"), strlen("/range_set_threshold3")+5); // 1.35
            log("set " + msgText + " val:" + val);
            THRESHOLD_VOLTAGE[3] = val.toFloat();
          } else if (startsWithIgnoreCase(msgText, "/range_set_threshold4")) {
            val = msgText.substring(strlen("/range_set_threshold4"), strlen("/range_set_threshold4")+5); // 1.35
            log("set " + msgText + " val:" + val);
            THRESHOLD_VOLTAGE[4] = val.toFloat();
          }

        /*} else if (msgText.equalsIgnoreCase("HIST")) {
          String hist_top = getHistogram("top");
          String hist_bottom = getHistogram("bottom");
          myBot.sendMessage(msg, "histogram\nTOP:\n" + hist_top + "\n\nBOTTOM:\n" + hist_bottom);*/

        } else if (msgText.equalsIgnoreCase("MODE")) {
          myBot.sendMessage(msg, "WORK_MODE: " + work_modes[work_mode] + "-" + String(work_mode) 
          + "\nRND_MODE: " + rnd_modes[rnd_mode] + "-" + String(rnd_mode) 
          + "\nANIMATION_MODE: " + String(animation_mode) 
          + "\nMUSIC_MODE: " + String(music_mode)
          //+ "\nAnimation_frame: " + String(animation_frame)
          );

        } else if (msgText.equalsIgnoreCase("RND-OFF") || msgText.equalsIgnoreCase("RND-M") || msgText.equalsIgnoreCase("RND-C") || msgText.equalsIgnoreCase("RND-M-C")) {
          rnd_mode = findInIndex(msgText, rnd_modes);
          log("set rnd_mode " + String(rnd_mode));

        } else if (msgText.equalsIgnoreCase("ALWAYS-OFF") || msgText.equalsIgnoreCase("ALWAYS-ON") || msgText.equalsIgnoreCase("NIGHT-ON") || msgText.equalsIgnoreCase("SENSORS") || msgText.equalsIgnoreCase("MUSIC")) {
          work_mode = findInIndex(msgText, work_modes);
          log("set work_mode " + String(work_mode));
          if (msgText.equalsIgnoreCase("ALWAYS-ON") || msgText.equalsIgnoreCase("MUSIC")) {
            start_animation();
          } else {
            clear_all();
            FastLED.show();
            is_start_animation = false;
            finish_animation();
          }

        } else if (msgText.equalsIgnoreCase("COLORS")) {
          myBot.sendMessage(msg, "COLOR1: " + main_color1_txt //String(main_color1.r) + "." + String(main_color1.g) + "." + String(main_color1.b) 
            + "\n/RED" + "\n/GREEN" + "\n/BLUE" + "\n/CYAN" + "\n/MAGENTA" + "\n/YELLOW"
            + "\n\nCOLOR2: " + main_color2_txt //String(main_color2.r) + "." + String(main_color2.g) + "." + String(main_color2.b) 
            + "\n/red" + "\n/green" + "\n/blue" + "\n/cyan" + "\n/magenta" + "\n/yellow"
            );

        } else if (msgText.equals("/RED")) {
          main_color1 = Red;
          main_color1_txt = "Red";
        } else if (msgText.equals("/GREEN")) {
          main_color1 = Green;
          main_color1_txt = "Green";
        } else if (msgText.equals("/BLUE")) {
          main_color1 = Blue;
          main_color1_txt = "Blue";
        } else if (msgText.equals("/CYAN")) {
          main_color1 = Cyan;
          main_color1_txt = "Cyan";
        } else if (msgText.equals("/MAGENTA")) {
          main_color1 = Magenta;
          main_color1_txt = "Magenta";
        } else if (msgText.equals("/YELLOW")) {
          main_color1 = Yellow;
          main_color1_txt = "Yellow";

        } else if (msgText.equals("/red")) {
          main_color2 = Red;
          main_color2_txt = "Red";
        } else if (msgText.equals("/green")) {
          main_color2 = Green;
          main_color2_txt = "Green";
        } else if (msgText.equals("/blue")) {
          main_color2 = Blue;
          main_color2_txt = "Blue";
        } else if (msgText.equals("/cyan")) {
          main_color2 = Cyan;
          main_color2_txt = "Cyan";
        } else if (msgText.equals("/magenta")) {
          main_color2 = Magenta;
          main_color2_txt = "Magenta";
        } else if (msgText.equals("/yellow")) {
          main_color2 = Yellow;
          main_color2_txt = "Yellow";

        
        } else if (msgText.equals("1") || msgText.equals("2") || msgText.equals("3") || msgText.equals("4") || msgText.equals("5") || msgText.equals("10") || msgText.equals("11") || msgText.equals("12")) { 
          finish_animation();
          animation_mode = msgText.toInt();
          log("set animation_mode " + String(animation_mode));

        } else if (msgText.equalsIgnoreCase("M1") || msgText.equalsIgnoreCase("M2") || msgText.equalsIgnoreCase("M3") || msgText.equalsIgnoreCase("M4")) {
          music_mode = msgText.substring(1, 2).toInt();
          log("set music_mode " + String(music_mode));
          //start_animation();

        } else {
          myBot.sendMessage(msg, "/start");
        }

        //log_matrix(msgText);
      }
    }

}

void setup() {
    log_setup(PORT_SPEED);
    log("start " + FRAME_MS);

    led_setup();
    wifi_setup();
    time_setup();
    light_setup();
    mic_setup(micPin);
    matrix.begin();
    sensors_setup();
    tg_setup();

    log_matrix("Hello");
    text_matrix("a" + String(animation_mode));

}

void loop() {

    //if (isModeForMusic(animation_mode)) {
    if (work_mode == 4/*MUSIC*/) {
      if (millis() - last_mic_millis >= 1) {
        last_mic_millis = millis();
        mic_loop();
      }
      tg_loop_interval = 20000;
    } else {
      if (animation_frame == 0) {
        tg_loop_interval = 500;
        if (millis() - last_br_millis >= 60*1000) {
          last_br_millis = millis();
          brightness_loop();
        }
      } else {
        tg_loop_interval = 5 * 1000;
      }
      if (millis() - last_sensor_millis >= 10) {
        last_sensor_millis = millis();
        sensors_loop();
      }
    }

    if (work_mode != 4) {
      if (millis() - last_matrix_millis >= FRAME_MS*SHOW_EACH_FRAME) {
        last_matrix_millis = millis();
        text_matrix(String((int)(animation_frame/SHOW_EACH_FRAME)));
      }
    }

    led_animate_loop(); // also used inside ultrasonic sensor waiting loop

    if (millis() - last_tg_millis >= tg_loop_interval) {
      last_tg_millis = millis();
      tg_loop();
    }
}

void led_animate_loop() {
    if (millis() - last_millis >= FRAME_MS-1) {
        last_millis = millis();
        if (animation_frame > 0) {
          animate_loop();
          FastLED.show();
        }
    }
}



// long blocking method
long measure_echo_time(uint8_t trigPin, uint8_t echoPin) {
  // Генерация короткого импульса на TRIG
  //digitalWrite(trigPin, LOW);
  //delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); //посылаем датчику сигнал начала замера (высокий уровень на 10 мкс by datashit for HC-SR04)
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Ждём HIGH от echo, с таймаутом
  long startTime = micros();
  while (digitalRead(echoPin) == LOW) {
    led_animate_loop();
    if (micros() - startTime > TIMEOUT_MCS) return TIMEOUT_MCS;  // таймаут ожидания начала импульса
  }

  long echoStart = micros();
  while (digitalRead(echoPin) == HIGH) {
    led_animate_loop();
    if (micros() - echoStart > TIMEOUT_MCS) return TIMEOUT_MCS;  // таймаут ожидания конца импульса
  }

  return micros() - echoStart;
}

void sensors_loop() {
  long duration_top;
  long duration_bottom;

  if (work_mode == 3 /*SENSORS*/) {

    /*if (TOP_SENS == SONIC_SENSOR) {
      duration_top = measure_echo_time(TOP_TRIG_PIN, TOP_ECHO_PIN);
      if (duration_top > 0 && duration_top < TIMEOUT_MCS) {
        // Расчёт расстояния (343 м/сек) (в см)
        new_distance_top = duration_top * 343 * 100 / 1000000 / 2;
        if (new_distance_top > track_in_histogram_top_min && new_distance_top < track_in_histogram_top_max) {
          addValueToBuffer("top", new_distance_top);
        } else {
          addValueToBuffer("top", 0);
        }
      } else {
        addValueToBuffer("top", 0);
      }
      distance_top = getValFromHistogram("top");
    } else if (TOP_SENS == IR_SENSOR) {*/
      value_pin[3] = analogRead(TOP_IR_SENSOR1_PIN);  // 0–4095
      value_pin[4] = analogRead(TOP_IR_SENSOR2_PIN);  // 0–4095
      voltage_pin[3] = get_analog_voltage_from_value(value_pin[3]);
      voltage_pin[4] = get_analog_voltage_from_value(value_pin[4]);

      addValueToBuffer("top1", voltage_pin[3]);
      addValueToBuffer("top2", voltage_pin[4]);

      avg_voltage_pin[3] = getAvgVal("top1");
      avg_voltage_pin[4] = getAvgVal("top2");

      if (avg_voltage_pin[3] >= THRESHOLD_VOLTAGE[3] || avg_voltage_pin[4] >= THRESHOLD_VOLTAGE[4]) {
        distance_top = distance_top_min;
      } else {
        distance_top = 0;
      }
    //}

    /*if (BOTTOM_SENS == SONIC_SENSOR) {
      duration_bottom = measure_echo_time(BOTTOM_TRIG_PIN, BOTTOM_ECHO_PIN);
      if (duration_bottom > 0 && duration_bottom < TIMEOUT_MCS) {
        // Расчёт расстояния (343 м/сек) (в см)
        new_distance_bottom = duration_bottom * 343 * 100 / 1000000 / 2;
        if (new_distance_bottom > track_in_histogram_bottom_min && new_distance_bottom < track_in_histogram_bottom_max) {
          addValueToBuffer("bottom", new_distance_bottom);
        } else {
          addValueToBuffer("bottom", 0);
        }
      } else {
        addValueToBuffer("bottom", 0);
      }
      distance_bottom = getValFromHistogram("bottom");
    } else if (BOTTOM_SENS == IR_SENSOR) {*/
      value_pin[1] = analogRead(BOTTOM_IR_SENSOR1_PIN);  // 0–4095
      value_pin[2] = analogRead(BOTTOM_IR_SENSOR2_PIN);  // 0–4095
      voltage_pin[1] = get_analog_voltage_from_value(value_pin[1]);
      voltage_pin[2] = get_analog_voltage_from_value(value_pin[2]);

      addValueToBuffer("bottom1", voltage_pin[1]);
      addValueToBuffer("bottom2", voltage_pin[2]);

      avg_voltage_pin[1] = getAvgVal("bottom1");
      avg_voltage_pin[2] = getAvgVal("bottom2");

      //if (digitalRead(BOTTOM_IR_SENSOR1_PIN) == HIGH || digitalRead(BOTTOM_IR_SENSOR2_PIN) == HIGH) {
      if (avg_voltage_pin[1] >= THRESHOLD_VOLTAGE[1] || avg_voltage_pin[2] >= THRESHOLD_VOLTAGE[2]) {
        distance_bottom = distance_bottom_min;
      } else {
        distance_bottom = 0;
      }
    //}

    /*if (millis() - last_hist_millis >= 50) {
      last_hist_millis = millis();
      String hist_top = ".";
      String hist_bottom = ".";
      if (TOP_SENS == SONIC_SENSOR) {
        hist_top = getHistogram("top");
      }
      if (BOTTOM_SENS == SONIC_SENSOR) {
        hist_bottom = getHistogram("bottom");
      }

      log("::  AVGtop:" + String(avg_voltage_pin[3]) + " / " + String(avg_voltage_pin[4]) + " AVGbottom:" + String(avg_voltage_pin[1]) + " / " + String(avg_voltage_pin[2]));
    }*/

    if (animation_frame == 0) {
      if (distance_top >= distance_top_min && distance_top <= distance_top_max) {
        set_direction(DOWN);
        start_animation();
      } else if (distance_bottom >= distance_bottom_min && distance_bottom <= distance_bottom_max) {
        set_direction(UP);
        start_animation();
      }
    } else if (animation_frame >= track_sensors_during_animation_after * FPS) {
      if (distance_top >= distance_top_min && distance_top <= distance_top_max) {
        set_next_direction(DOWN);
      } else if (distance_bottom >= distance_bottom_min && distance_bottom <= distance_bottom_max) {
        set_next_direction(UP);
      }
    }

  }
}


int last_mic_log_millis = 0;
void mic_loop() {
  globalMicValue = getMicVal(micPin, micMethod);

  if (millis() - last_mic_log_millis >= 50) {
    last_mic_log_millis = millis();
    String graph = bar(globalMicValue);
    Serial.println(String(globalMicValue) + " ::: " + graph);
  }
}

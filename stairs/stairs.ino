
#define PORT_SPEED 250000 //115200

//mic
#include "mic.h"
const int micPin = A0;
const int micMethod = 1;

//light
#include "light_sens.h"
float lux_coef = 5.0;

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

//wifi
#include "WiFiS3.h"
#include "WiFiSSLClient.h"
#include "IPAddress.h"
char ssid[] = "KF";
char pass[] = "19781979";
int status = WL_IDLE_STATUS;

//tg
#include <AsyncTelegram2.h>
WiFiSSLClient client;
AsyncTelegram2 myBot(client);
const char* token = "5087541658:AAHGz90kUzEWJzo0Xh7rPMGaEyVHHCL0N2w";
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


void tg_setup() {

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

    myBot.setUpdateTime(500);
    myBot.setTelegramToken(token);

    myBot.begin() ? log("BOT OK") : log("BOT NOK");
    Serial.print("Bot name: @");
    Serial.println(myBot.getBotName());
    log("Telegram connected");

    myReplyKbd.addButton("TOP");
    myReplyKbd.addButton("BOTTOM");
    myReplyKbd.addRow();
    myReplyKbd.addButton("OFF");
    myReplyKbd.addButton("ON");
    myReplyKbd.addButton("RND-M");
    myReplyKbd.addButton("RND-C");
    myReplyKbd.addButton("RND-M-C");
    myReplyKbd.addRow();
    myReplyKbd.addButton("1");
    myReplyKbd.addButton("2");
    myReplyKbd.addButton("3");
    myReplyKbd.addButton("4");
    myReplyKbd.addButton("5");
    myReplyKbd.addButton("10");
    myReplyKbd.addButton("11");
    myReplyKbd.addRow();
    myReplyKbd.addButton("6");
    myReplyKbd.addButton("7");
    myReplyKbd.addButton("8");
    myReplyKbd.addButton("9");
    myReplyKbd.addRow();
    myReplyKbd.addButton("RANGE");
    myReplyKbd.addButton("HIST");
    myReplyKbd.addButton("BRIGHTNESS");
    myReplyKbd.addButton("MODE");
    myReplyKbd.addButton("COLORS");
    myReplyKbd.enableResize();

    delay(1000); //Pause before clearing messages
    while (myBot.getNewMessage(msg)) {
      log("Skipping old message: " + msg.text);
      delay(100);
    }
    log("Old messages cleared!");
}


void tg_loop() {
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
          myBot.sendMessage(msg, "start animation BOTTOM " + String(animation_mode));
          set_direction(UP);
          start_animation();

        } else if (msgText.equalsIgnoreCase("TOP")) {
          log("set TOP");
          myBot.sendMessage(msg, "start animation TOP " + String(animation_mode));
          set_direction(DOWN);
          start_animation();

        } else if (msgText.equalsIgnoreCase("BRIGHTNESS")) {
          brightness_loop();
          myBot.sendMessage(msg, "brightness\nLEVEL: " + String(brightness) + "\nLUX: " + String(lux) + "\n/lux_coef_set " + String(lux_coef));
          
        } else if (startsWithIgnoreCase(msgText, "/lux_coef_set")) {
          lux_coef = msgText.substring(strlen("/lux_coef_set"), strlen("/lux_coef_set")+4).toFloat();
          brightness_loop();
          myBot.sendMessage(msg, "brightness\nLEVEL: " + String(brightness) + "\nLUX: " + String(lux) + "\n/lux_coef_set " + String(lux_coef));

        } else if (msgText.equalsIgnoreCase("RANGE")) {
          myBot.sendMessage(msg, "distance\nTOP: " + String((int)distance_top) + "cm\nBOTTOM: " + String((int)distance_bottom) + "cm\n"
            + "LAST_TOP: " + String((int)last_start_distance_top) + "\nLAST_BOTTOM: " + String((int)last_start_distance_bottom) + "\nLAST_DIRECTION: " + String(last_direction) + "\n\n"
            + "/range_set_top_min " + String(distance_top_min) + "\n/range_set_top_max " + String(distance_top_max) + "\n/range_set_bottom_min " + String(distance_bottom_min) + "\n/range_set_bottom_max " + String(distance_bottom_max));

        } else if (msgText.equalsIgnoreCase("HIST")) {
          String hist_top = getHistogram("top");
          String hist_bottom = getHistogram("bottom");
          myBot.sendMessage(msg, "histogram\nTOP:\n" + hist_top + "\n\nBOTTOM:\n" + hist_bottom);

        } else if (startsWithIgnoreCase(msgText, "/range_set")) {
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
          }

        } else if (msgText.equalsIgnoreCase("MODE")) {
          myBot.sendMessage(msg, "WORK_MODE: " + work_modes[work_mode] + "\nMODE: " + String(animation_mode));

        } else if (msgText.equalsIgnoreCase("COLORS")) {
          myBot.sendMessage(msg, "COLOR1: " + main_color1_txt //String(main_color1.r) + "." + String(main_color1.g) + "." + String(main_color1.b) 
            + "\n/RED" + "\n/GREEN" + "\n/BLUE" + "\n/CYAN" + "\n/MAGENTA" + "\n/YELLOW\n"
            + "\nCOLOR2: " + main_color2_txt //String(main_color2.r) + "." + String(main_color2.g) + "." + String(main_color2.b) 
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

        } else if (msgText.equalsIgnoreCase("OFF") || msgText.equalsIgnoreCase("ON") || msgText.equalsIgnoreCase("RND-M") || msgText.equalsIgnoreCase("RND-C") || msgText.equalsIgnoreCase("RND-M-C")) {
          work_mode = findWorkModeIndex(msgText);
          log("set work_mode " + String(work_mode));
        
        } else if (msgText.equals("1") || msgText.equals("2") || msgText.equals("3") || msgText.equals("4") || msgText.equals("5") || msgText.equals("6") || msgText.equals("10") || msgText.equals("11")) { 
          finish_animation();
          animation_mode = msgText.toInt();
          log("set animation_mode " + String(animation_mode));

        } else if (msgText.equals("6") || msgText.equals("7") || msgText.equals("8") || msgText.equals("9")) {
          animation_mode = msgText.toInt();
          log("set music_mode " + String(animation_mode));
          start_animation();

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

    FastLED.addLeds<WS2811, LED_PIN_1, BRG>(leds, 0,                    POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_2, BRG>(leds, POINTS_PER_STEP * 4,  POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_3, BRG>(leds, POINTS_PER_STEP * 8,  POINTS_PER_STEP * 4);
    FastLED.addLeds<WS2811, LED_PIN_4, BRG>(leds, POINTS_PER_STEP * 12, POINTS_PER_STEP * 4);

    FastLED.setBrightness(brightness);
    FastLED.clear();
    FastLED.show();

    light_setup();
    mic_setup(micPin);
    matrix.begin();
    sensors_setup();
    tg_setup();

    //log_matrix("Mode " + animation_mode);
}

void loop() {

    if (isModeForMusic(animation_mode)) {
      if (millis() - last_mic_millis >= 1) {
        last_mic_millis = millis();
        mic_loop();
      }
      tg_loop_interval = 30000;
    } else {
      if (animation_frame == 0) {
        tg_loop_interval = 500;
        if (millis() - last_br_millis >= 2*1000) {
          last_br_millis = millis();
          brightness_loop();
        }
      }
      if (millis() - last_sensor_millis >= 10) {
        last_sensor_millis = millis();
        sensors_loop();
      }
    }

    main_animate_loop();

    if (millis() - last_tg_millis >= tg_loop_interval) {
      last_tg_millis = millis();
      tg_loop();
    }

}

void main_animate_loop() {
    if (millis() - last_millis >= FRAME_MS-1) {
        last_millis = millis();
        if (animation_frame > 0) {
          animate_loop();
          FastLED.show();
          tg_loop_interval = 30000;
        }
    }
}

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

void brightness_loop() {
  lux = get_lux();
  lux = lux * lux_coef;

  int new_brightness;
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

  if (new_brightness != brightness) {
    brightness = new_brightness;
    FastLED.setBrightness(brightness);
    log("brightness:" + String(brightness) + " lux:" + String(lux));
  }
}

void sensors_setup() {
  pinMode(TOP_TRIG_PIN, OUTPUT);
  pinMode(TOP_ECHO_PIN, INPUT);
  pinMode(BOTTOM_TRIG_PIN, OUTPUT);
  pinMode(BOTTOM_ECHO_PIN, INPUT);
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
    main_animate_loop();
    if (micros() - startTime > TIMEOUT_MCS) return TIMEOUT_MCS;  // таймаут ожидания начала импульса
  }

  long echoStart = micros();
  while (digitalRead(echoPin) == HIGH) {
    main_animate_loop();
    if (micros() - echoStart > TIMEOUT_MCS) return TIMEOUT_MCS;  // таймаут ожидания конца импульса
  }

  return micros() - echoStart;
}

void sensors_loop() {
  long duration_top;
  long duration_bottom;

  duration_top = measure_echo_time(TOP_TRIG_PIN, TOP_ECHO_PIN);
  if (duration_top > 0 && duration_top < TIMEOUT_MCS) {
    // Расчёт расстояния (343 м/сек) (в см)
    new_distance_top = duration_top * 343 * 100 / 1000000 / 2;
    if (new_distance_top > track_in_histogram_top_min && new_distance_top < track_in_histogram_top_max) {
      addValueToHistogram("top", new_distance_top);
    } else {
      addValueToHistogram("top", 0);
    }
  } else {
    addValueToHistogram("top", 0);
  }
  distance_top = getValFromHistogram("top");

  duration_bottom = measure_echo_time(BOTTOM_TRIG_PIN, BOTTOM_ECHO_PIN);
  if (duration_bottom > 0 && duration_bottom < TIMEOUT_MCS) {
    // Расчёт расстояния (343 м/сек) (в см)
    new_distance_bottom = duration_bottom * 343 * 100 / 1000000 / 2;
    if (new_distance_bottom > track_in_histogram_bottom_min && new_distance_bottom < track_in_histogram_bottom_max) {
      addValueToHistogram("bottom", new_distance_bottom);
    } else {
      addValueToHistogram("bottom", 0);
    }
  } else {
    addValueToHistogram("bottom", 0);
  }
  distance_bottom = getValFromHistogram("bottom");

  if (millis() - last_hist_millis >= 20) {
    last_hist_millis = millis();
    String hist_top = getHistogram("top");
    String hist_bottom = getHistogram("bottom");
    log("::" + distance_top_bar + " . " + distance_bottom_bar + "  " + hist_top + " " + hist_bottom + "  " + String((int)distance_top) + " / " + String((int)distance_bottom));
  }

  if (distance_top >= distance_top_min && distance_top <= distance_top_max) {
    distance_top_bar = distanceBar(distance_top);
    distance_bottom_bar = distanceBar(0.0);
    set_direction(DOWN);
    if (animation_frame == 0) {
      start_animation();
    }
  } else if (distance_bottom >= distance_bottom_min && distance_bottom <= distance_bottom_max) {
    distance_top_bar = distanceBar(0.0);
    distance_bottom_bar = distanceBar(distance_bottom);
    set_direction(UP);
    if (animation_frame == 0) {
      start_animation();
    }
  } else {
    distance_top_bar = distanceBar(0.0);
    distance_bottom_bar = distanceBar(0.0);
  }

}


int last_mic_log_millis = 0;
void mic_loop() {
  globalMicValue = getMicVal(micPin, micMethod);

  if (millis() - last_mic_log_millis >= 200) {
    last_mic_log_millis = millis();
    String graph = bar(globalMicValue);
    //Serial.println(String(globalMicValue) + " ::: " + graph);
  }
}


#define SONIC_SENSOR 1
#define IR_SENSOR 2

//ultrasonic sensors HC-SR04
#define TOP_TRIG_PIN D9
#define TOP_ECHO_PIN D10
#define BOTTOM_TRIG_PIN D11
#define BOTTOM_ECHO_PIN D12

const int TIMEOUT_MCS = 50000;  //max 857 cm
float distance_top = 0;
float distance_bottom = 0;
float new_distance_top = 0;
float new_distance_bottom = 0;
float distance_threshold = 5;
float last_start_distance_top = 0;
float last_start_distance_bottom = 0;

//IR sensor HC-SR501
#define BOTTOM_IR_SENSOR1_PIN A2  //D11
#define BOTTOM_IR_SENSOR2_PIN A3  //D12
float THRESHOLD_VOLTAGE = 0.30;
float voltage_pin1 = 0;
float voltage_pin2 = 0;
float avg_voltage_pin1 = 0;
float avg_voltage_pin2 = 0;
float last_start_voltage_sensor1 = 0;
float last_start_voltage_sensor2 = 0;
int value_pin1;
int value_pin2;
int last_start_value_sensor1 = 0;
int last_start_value_sensor2 = 0;

int distance_top_min = 30;
int distance_top_max = 80;
int distance_bottom_min = 30;
int distance_bottom_max = 40;

const int num_bins = 50;
const int max_value = 500;
const int value_step = max_value / num_bins;
const int window_size = 10;

int histogram_top[num_bins] = { 0 };
int histogram_bottom[num_bins] = { 0 };
int values_top[window_size];  // Скольжение по window_size значений
int values_bottom[window_size];
float values_bottom1[window_size];
float values_bottom2[window_size];
int current_index_top = 0;
int current_index_bottom = 0;
int current_index_bottom1 = 0;
int current_index_bottom2 = 0;
bool filled_top = false;  // Чтобы знать, когда набрали 100 значений
bool filled_bottom = false;
bool filled_bottom1 = false;
bool filled_bottom2 = false;
float track_in_histogram_top_min = 10;
float track_in_histogram_top_max = 200;
float track_in_histogram_bottom_min = 10;
float track_in_histogram_bottom_max = 200;


const char* levels[] = {
  "_",  // 0
  "▁",  // U+2581
  "▂",  // U+2582
  "▃",  // U+2583
  "▄",  // U+2584
  "▅",  // U+2585
  "▆",  // U+2586
  "▇",  // U+2587
  "█"   // U+2588
};
// ▁ ▂ ▃ ▄ ▅ ▆ ▇ █

String distance_top_bar = "";
String distance_bottom_bar = "";

String distanceBar(float distance) {
  // Ограничим значения в допустимом диапазоне
  distance = constrain(distance, 0.0, 200.0);

  // Преобразуем расстояние в количество символов (0–20)
  int barLength = round((distance / 200.0) * 20);

  // Соберём строку из нужного количества символов █
  String bar = "";
  for (int i = 0; i < barLength; i++) {
    bar += "█";
  }
  for (int i = 0; i < 20 - barLength; i++) {
    bar += "_";
  }

  return bar;
}

// Печать гистограммы top/bottom
String getHistogram(String sensor) {
  String hist = "";
  //Serial.println("Histogram: " + sensor);
  int one_histogram_height = window_size / 2;
  for (int i = 0; i < num_bins; i++) {
    if (sensor == "top") {
      int level = map(constrain(histogram_top[i], 0, one_histogram_height), 0, one_histogram_height, 0, 8);
      hist += String(levels[level]);
    }
    if (sensor == "bottom") {
      int level = map(constrain(histogram_bottom[i], 0, one_histogram_height), 0, one_histogram_height, 0, 8);
      hist += String(levels[level]);
    }
  }
  return hist;
}

// Поиск максимума по гистограмме top/bottom
float getValFromHistogram(String sensor) {
  float max_freq = 0;
  int found_i = 0;
  // skip zero values
  for (int i = 1 /*0*/; i < num_bins; i++) {
    if (sensor == "top") {
      if (histogram_top[i] > max_freq) {
        max_freq = histogram_top[i];
        found_i = i;
      }
    }
    if (sensor == "bottom") {
      if (histogram_bottom[i] > max_freq) {
        max_freq = histogram_bottom[i];
        found_i = i;
      }
    }
  }

  return found_i * value_step;
}

// Обновление гистограммы по последним 100 значениям
void updateHistogram(String sensor) {
  // Сброс
  if (sensor == "top") {
    for (int i = 0; i < num_bins; i++) {
      histogram_top[i] = 0;
    }

    // Подсчёт
    int count = filled_top ? window_size : current_index_top;
    for (int i = 0; i < count; i++) {
      int bin_index = values_top[i] / value_step;
      if (bin_index >= num_bins) bin_index = num_bins - 1;
      histogram_top[bin_index]++;
    }
  }
  if (sensor == "bottom") {
    for (int i = 0; i < num_bins; i++) {
      histogram_bottom[i] = 0;
    }

    // Подсчёт
    int count = filled_bottom ? window_size : current_index_bottom;
    for (int i = 0; i < count; i++) {
      int bin_index = values_bottom[i] / value_step;
      if (bin_index >= num_bins) bin_index = num_bins - 1;
      histogram_bottom[bin_index]++;
    }
  }
}


// get average value from values
float getAvgVal(String sensor) {
  float avg_val = 0;
  float sum_val = 0;
  int count = 0;

  if (sensor == "bottom1") {
    count = filled_bottom1 ? window_size : current_index_bottom1;
    for (int i = 0; i < count; i++) {
      sum_val += values_bottom1[i];
    }

  } else if (sensor == "bottom2") {
    count = filled_bottom2 ? window_size : current_index_bottom2;
    for (int i = 0; i < count; i++) {
      sum_val += values_bottom2[i];
    }
  }

  if (count > 0) {
    avg_val = sum_val / count;
  }

  return avg_val;
}

void addValueToHistogram(String sensor, float value) {
  if (sensor == "top") {
    values_top[current_index_top] = value;
    current_index_top = (current_index_top + 1) % window_size;
    if (current_index_top == 0) filled_top = true;

  } else if (sensor == "bottom") {
    values_bottom[current_index_bottom] = value;
    current_index_bottom = (current_index_bottom + 1) % window_size;
    if (current_index_bottom == 0) filled_bottom = true;

  } else if (sensor == "bottom1") {
    values_bottom1[current_index_bottom1] = value;
    current_index_bottom1 = (current_index_bottom1 + 1) % window_size;
    if (current_index_bottom1 == 0) filled_bottom1 = true;

  } else if (sensor == "bottom2") {
    values_bottom2[current_index_bottom2] = value;
    current_index_bottom2 = (current_index_bottom2 + 1) % window_size;
    if (current_index_bottom2 == 0) filled_bottom2 = true;
  }

  updateHistogram(sensor);
}

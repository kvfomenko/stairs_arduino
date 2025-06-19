
#define EXP 1.2                   // степень усиления сигнала (для более "резкой" работы) (по умолчанию 1.4)
#define MAX_COEF 1.8              // коэффициент громкости (максимальное равно срднему * этот коэф) (по умолчанию 1.8)
const int LOW_PASS = 100;
const float SMOOTH = 0.5;         // коэффициент сглаживания (0,01 макс сглаживание....1 нет сглаживания)
const float SMOOTH_AVER = 0.006;  // коэффициент сглаживания для автогромкости (0,01 макс сглаживание....1 нет сглаживания)
const int sampleWindow = 20;      // Sample window width in ms (50 ms = 20Hz)
const int micPin = A0;
const int micMethod = 1;

int globalMicRawValue = 0;
int globalMicValue = 0;
float smoothValue = 0;
float averageLevel = 0;

void mic_setup(int analogMicPin) {
   pinMode(analogMicPin, INPUT);
}

String bar(int value) {
  int count = value / 10;
  String result = "";
  for (int i = 0; i < count; i++) {
    result += "█";
  }
  if (result != "") {
    result += " ";
  }
  return result;
}

int getMicVal(int analogMicPin, int method) {
  long startMillis = millis();  // Start of sample window
  float peakToPeak = 0;   // peak-to-peak level
  int sample;

  if (method == 1) {
    // collect data for N mS
    int signalMax = 0;
    int signalMin = 1023;
    while (millis() - startMillis < sampleWindow) {
      sample = analogRead(analogMicPin);
      if (sample > signalMax) {
        signalMax = sample;  // save just the max levels
      }
      if (sample < signalMin) {
        signalMin = sample;  // save just the min levels
      }
      //Serial.println(" " + String(sample) + " " + String(signalMin) + "..." + String(signalMax));
    }
    // max - min = peak-peak amplitude
    peakToPeak = signalMax - signalMin;
    
  } else if (method == 2) {
    peakToPeak = 0;
    for (int i = 0; i < 50; i++) {                    // делаем N измерений
      sample = analogRead(analogMicPin);
      if (peakToPeak < sample) peakToPeak = sample;   // ищем максимальное
    }  
  }
  globalMicRawValue = peakToPeak;

  //double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
  //Serial.println(String(peakToPeak) + " " + String(signalMin) + "..." + String(signalMax));

  // фильтруем по нижнему порогу шумов
  peakToPeak = map(peakToPeak, LOW_PASS, 1000, 0, 100);

  // ограничиваем диапазон
  peakToPeak = constrain(peakToPeak, 0, 100);

  // возводим в степень (для большей чёткости работы)
  peakToPeak = pow(peakToPeak, EXP);

  // Фильтр очень медленный, сделано специально для получения среднего уровня в качестве базы для автогромкости
  //averageLevel = SMOOTH_AVER * peakToPeak + (1 - SMOOTH_AVER) * averageLevel;
 
  // фильтр сглаживания
  smoothValue = SMOOTH * peakToPeak + (1.0 - SMOOTH) * smoothValue;

  // принимаем максимальную громкость шкалы как среднюю, умноженную на некоторый коэффициент MAX_COEF
  //maxLevel = averageLevel * MAX_COEF;

  smoothValue = constrain(smoothValue, 0, 100);

  return smoothValue;
}

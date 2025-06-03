
#include <BH1750.h>
BH1750 lightMeter;
int brightness = 64;
float lux = 0;
// используются пины A4 белый DAT и A5 коричневый SCL

void light_setup() {
    Wire.begin(); //required for lightMeter.begin();

    if (lightMeter.begin()) {
      Serial.println("BH1750 ready.");
    } else {
      Serial.println("Error: BH1750 not configured!");
      while (1); // Остановить выполнение
    }
}

float get_lux() {
  return lightMeter.readLightLevel();
}


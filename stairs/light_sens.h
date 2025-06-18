
#include <BH1750.h>
BH1750 lightMeter;
int brightness = 64;
float lux = 1;  //1..65535
// используются пины A4 белый DAT и A5 коричневый SCL

void light_setup() {
  //itit i2c on A4/A5
  // Wire.begin(); //required for lightMeter.begin();

  //itit i2c on SDA/SCL
  Wire1.begin();

  // use Wire
  //if (lightMeter.begin()) {
  // use Wire1
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire1)) {
    Serial.println("BH1750 ready.");
  } else {
    Serial.println("Error: BH1750 not configured!");
    while (1); // Остановить выполнение
  }
}

float get_lux() {
  float val = lightMeter.readLightLevel(); //0..65535
  if (val < 1) {
    val = 1;
  }
  return val;
}


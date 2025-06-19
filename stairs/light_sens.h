#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

bool LIGHT_SENSOR_ENABLE = false;
int brightness = 64;
float lux_default = 4.0;
float lux = lux_default;  //1..65535
float lux_coef = 4.0;
// используются пины A4 белый DAT и A5 коричневый SCL

void light_setup() {
  if (LIGHT_SENSOR_ENABLE) {
    //itit i2c on A4/A5
    Wire.begin(); //required for lightMeter.begin();

    //itit i2c on SDA/SCL
    //Wire1.begin();

    // use Wire
    if (lightMeter.begin()) {
    // use Wire1
    //if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire1)) {
      Serial.println("BH1750 ready.");
    } else {
      Serial.println("Error: BH1750 not configured!");
      LIGHT_SENSOR_ENABLE = false;
      Serial.println("LIGHT_SENSOR disabled");
    }
  } else {
    Serial.println("LIGHT_SENSOR disabled");
  }
}

float get_lux() {
  float val;
  if (LIGHT_SENSOR_ENABLE) {  
    val = lightMeter.readLightLevel(); //0..65535
  } else {
    val = lux_default;
  }
  if (val < 1) {
    val = 1;
  }
  return val;
}


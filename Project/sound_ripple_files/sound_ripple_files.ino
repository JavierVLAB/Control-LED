#include <FastLED.h>

int ledMode = 0;

void setup() {
  Serial.begin(115200);
  setupEEPROM();
  WiFisetup();
  ledMode = actualLedMode();
  setupFastLEDs();
}

void loop() {

  switch (ledMode) {
    case 1:
      ledReactControl();
      Serial.print("1");
      break;
    case 2:
      Serial.print("2");
      break;
    case 3:
      Serial.print("3");
      break;
    default:
      break;
  }

  FastLED.show();

}

void ledReactControl() {
  getSample();

  EVERY_N_MILLISECONDS(20) {
    ripple();
  }
}

#include <EEPROM.h>

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Empezar");
  setupEEPROM();
  //checkEEPROM();
  writeEEPROM(1, "me cago en ...");
  Serial.println("word1= " + readEEPROM(1));
  writeEEPROM(2, "coño");
  Serial.println("word4= " + readEEPROM(2));
  writeEEPROM(3, "osuudisa");
  Serial.println("word3= " + readEEPROM(3));
}

void loop() {


}

#include <EEPROM.h>

//#define MAX_CHAR_LENGTH;

void setupEEPROM() {
  EEPROM.begin(512);
  delay(20);
}



void modeEEPROM(int mod, int *memoryPosition, int *memoryLength) {
  switch (mod) {
    case 1: //SSID
      *memoryPosition = 0;
      *memoryLength = 32;
      break;
    case 2: //Password
      *memoryPosition = 32;
      *memoryLength = 32;
      break;
    case 3: //test3
      *memoryPosition = 64;
      *memoryLength = 10;
      break;
    default:
      Serial.println("default");
      break;
  }
}


String readEEPROM(int mod) {

  int memoryPos;
  int memoryLength;
  modeEEPROM(mod, &memoryPos, &memoryLength);

  String memory = "";
  for (int i = 0; i < memoryLength; ++i)
  {
    memory += char(EEPROM.read(memoryPos + i));
  }
  return memory;
}



void writeEEPROM(int mod, String wordToSave) {

  int memoryPos;
  int memoryLength;
  modeEEPROM(mod, &memoryPos, &memoryLength);

  if (wordToSave.length() > 0)
  {

    for (int i = 0; i < memoryLength + 1; ++i)
    {
      EEPROM.write(memoryPos + i, 0);
      
    }

    for (int i = 0; i < wordToSave.length(); ++i)
    {
      EEPROM.write(memoryPos + i, wordToSave[i]);
      Serial.print(wordToSave[i]);
    }
    EEPROM.commit();
  }
  Serial.println("");
}

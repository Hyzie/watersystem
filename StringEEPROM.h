#include <EEPROM.h>

#define EEPROM_SIZE 64



void WriteStringToEEPROM(int addr, const String &str) {
  int len = str.length();
  Serial.print("Writing string: ");
  Serial.print(str);
  Serial.print(" at address: ");
  Serial.println(addr);

  EEPROM.write(addr, len); 
  for (int i = 0; i < len; i++) {
    EEPROM.write(addr + 1 + i, str[i]); 
  }
  EEPROM.commit();
}

String ReadStringFromEEPROM(int addr) {
  int len = EEPROM.read(addr); 
  char data[len + 1]; 
  for (int i = 0; i < len; i++) {
    data[i] = EEPROM.read(addr + 1 + i); 
  }
  data[len] = '\0'; 

  Serial.print("Read string of length: ");
  Serial.print(len);
  Serial.print(" from address: ");
  Serial.println(addr);

  return String(data);
}

char* stringToCharArray(const String& str) {
  char* charArray = new char[str.length() + 1];
  str.toCharArray(charArray, str.length() + 1);
  return charArray;
}

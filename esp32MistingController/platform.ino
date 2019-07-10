#ifdef PLATFORM_ARDUINO

#include "platform.h"

unsigned char getPersistentByte(int address)
{
  EEPROM.begin(512);
  unsigned char val = EEPROM.read(address);
  EEPROM.end();
  return val;
}

int getPersistentBytes(int address, char *getConfig, int len)
{
  EEPROM.begin(512);
  for(int  i = 0; i < len; i++) {
    getConfig[i] = EEPROM.read(address);
    //Serial.printf("read: %c|%x\r\n", getConfig[i], getConfig[i]);
    address++;
  }
  EEPROM.end();
  return 0;
}

unsigned short getPersistentShort(int address)
{
  EEPROM.begin(512);
  unsigned char lsb = EEPROM.read(address);
  unsigned char msb = EEPROM.read(address + 1);
  EEPROM.end();
  unsigned short value = (((msb << 8) | lsb) & 0xFFFF);
  return value;
}

int setPersistentByte(int address, unsigned char value)
{
  EEPROM.begin(512);
  EEPROM.write(address, value);
  EEPROM.end();
  return 0;
}

int setPersistentShort(int address, unsigned short value)
{
  unsigned char lsb = value & 0xFF;
  unsigned char msb = (value & 0xFF00) >> 8;
  EEPROM.begin(512);
  EEPROM.write(address, lsb);
  EEPROM.write(address + 1, msb);
  EEPROM.end();
  return 0;
}

int setPersistentBytes(int address, char *value, int len)
{
  EEPROM.begin(512);
  for (int i = 0; i < len; i++) {
    EEPROM.write(address, value[i]);
    //Serial.printf("wrote: %c|%x\r\n", value[i], value[i]);
    address++;
  }
  EEPROM.end();
  return 0;
}

int setPersistentBytesWithDataByte(int address, int len, unsigned char dataByte)
{
  EEPROM.begin(512);
  for (int i = 0; i < len; i++) {
    EEPROM.write(address, dataByte);
    address++;
  }
  EEPROM.end();
  return 0;
}

#endif //PLATFORM_ARDUINO

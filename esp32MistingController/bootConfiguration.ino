#include "bootConfiguration.h"
int gEpBootCfgStart = 0;
int gEpBootCfgLength = 13;

int isFirstBoot()
{
  int address = 0;
  int ret = 0;
  unsigned char val = getPersistentByte(address);
  if(val != 0xA5){
    setPersistentByte(address, 0xA5);
    ret = 1;
  }
  return ret;
}

int isNetworkConfigPresent()
{
  int address = 1;
  int ret = 0;
  unsigned char val = getPersistentByte(address);
  if(val == 0xB5){
    ret = 1;
  }
  return ret;
}

int isDeviceIdPresent()
{
  int address = 3;
  int ret = 0;
  unsigned char val = getPersistentByte(address);
  if(val == 0xD5){
    ret = 1;
  }
  return ret;
}

int isMotorConfigPresent()
{
  int address = 2;
  int ret = 0;
  unsigned char val = getPersistentByte(address);
  if(val == 0xC5){
    ret = 1;
  }
  return ret;
}

void setMotorConfigPresence()
{
  int address = 2;
  setPersistentByte(address, 0xC5);
}
void clearMotorConfigPresence()
{
  int address = 2;
  setPersistentByte(address, 0x00);
}

void setNetworkConfigPresence()
{
  int address = 1;
  setPersistentByte(address, 0xB5);
}

void clearNetworkConfigPresence()
{
  int address = 1;
  setPersistentByte(address, 0x00);
}


void setDeviceIdPresence()
{
  int address = 3;
  setPersistentByte(address, 0xD5);
}

void clearDeviceIdPresence()
{
  int address = 3;
  setPersistentByte(address, 0x00);
}


void flushBootConfig()
{
  Serial.print("Flushing the boot config...\n");
  int startAddress = gEpBootCfgStart;
  unsigned char dataByte = 0x00;
  setPersistentBytesWithDataByte(startAddress, gEpBootCfgLength, dataByte); //set the eeprom with a data byte from a specific address
}

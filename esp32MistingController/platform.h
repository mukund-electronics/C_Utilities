#ifndef __PLATFORM_H__
#define __PLATFORM_H__

//Persistent Memory APIs
unsigned char getPersistentByte(int address);
int setPersistentByte(int address, unsigned char value);
int setPersistentBytes(int address, char *value, int len);
int getPersistentBytes(int address, char *getConfig, int len);
int setPersistentBytesWithDataByte(int address, int len, unsigned char dataByte);

#endif //__PLATFORM_H__

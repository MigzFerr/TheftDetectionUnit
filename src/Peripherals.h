#ifndef PERIHPHERALS_H
#define PERIHPHERALS_H
#define GPSDataEntries 8640
#include <Arduino.h>
extern volatile bool scanDone;
extern volatile int foundRSSI;
extern unsigned long phoneDetectedAt;

struct GPSData
{
  int32_t lat;
  int32_t lon;
  uint8_t hdop;
  uint8_t dayIndex;
  uint32_t secondsSinceMidnight;
};

GPSData buffer[GPSDataEntries];
uint16_t head = 0;
bool GPSDataFilled = false;
void setupBLE();
void powerModem();
void initModemAndGPS();
void externalGPSData();
void internalGPSData();
void setupBMI();
float getBMIData();
void printBMIData();
#endif
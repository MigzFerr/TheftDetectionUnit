#ifndef PERIHPHERALS_H
#define PERIHPHERALS_H
#include <Arduino.h>
#include <TinyGPS++.h>
extern TinyGPSPlus gps;
extern volatile bool scanDone;
extern volatile int foundRSSI;
extern unsigned long phoneDetectedAt;
extern volatile bool bikeAlarmed;

void setupBLE();
void powerModem();
void initModemAndGPS();
void externalGPSData();
void internalGPSData();
void setupBMI();
float getBMIData();
void printBMIData();
#endif
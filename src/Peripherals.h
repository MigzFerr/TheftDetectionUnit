#ifndef PERIHPHERALS_H
#define PERIHPHERALS_H
#include <Arduino.h>
#include <TinyGPS++.h>
#include "SystemManager.h"
extern TinyGPSPlus gps;
extern volatile bool scanDone;
extern volatile int foundRSSI;
extern unsigned long phoneDetectedAt;
extern volatile bool bikeAlarmed;
void sendNtfy(char* title, char* time, char* message, char* gpsPos, char* priority );
void testAlert();
void setupBLE();
void powerModem();
void initModemAndGPS();
void updateGPS();
GPSData externalGPSData();
void setupBMI();
float getBMIData();
void printBMIData();
float getBattVoltage();
void setupADC();
#endif
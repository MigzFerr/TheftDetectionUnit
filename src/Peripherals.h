#ifndef PERIHPHERALS_H
#define PERIHPHERALS_H

extern volatile bool scanDone;
extern volatile int foundRSSI;
extern unsigned long phoneDetectedAt;
void setupBLE();
void powerModem();
void setupGPS();
void  externalGPSData();
void internalGPSData();
void setupBMI();
float getBMIData();
void printBMIData();
#endif
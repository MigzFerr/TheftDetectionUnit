
#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H
#include <Arduino.h>
#include <TinyGPS++.h>
#pragma once
#pragma pack(1)
extern bool enableMovementDetection;
extern unsigned long GPSCheckDelay;
extern TinyGPSPlus gps;
struct GPSData
{ int32_t lat;
  int32_t lon;
  uint32_t secondsSinceMidnight;
  uint8_t sats;
  uint8_t hdop;
  uint8_t dayIndex;
  bool isValid;
};
bool ownerNear();
void calibrateBMI();
void checkBMIMovement();
void checkGPSMovement();
void movementDetection();
void checkPresence();
void checkToAdd(GPSData &GPSPosition);
void sendAlert(int);
void updateGPSCheckSpeed();
#endif
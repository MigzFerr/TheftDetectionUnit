
#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H
#include <Arduino.h>
#define GPSDataEntries 8640
extern bool enableMovementDetection;
struct GPSData
{
  int32_t lat;
  int32_t lon;
  uint8_t hdop;
  uint8_t dayIndex;
  uint32_t secondsSinceMidnight;
};
void calibrateBMI();
void checkBMIMovement();
void checkGPSMovement();
void movementDetection();
#endif
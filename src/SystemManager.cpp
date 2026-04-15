#include "SystemManager.h"
#include <TinyGPS++.h>
#include "peripherals.h"

#define PHONEDETECTIONCOOLDOWN 5000
#define MESSAGEDELAY 60000
#define BikeMovedCooldown 300000
GPSData buffer[GPSDataEntries];
uint16_t head = 0;
bool GPSDataFilled = false;
float SHAKETOLERANCE = 0.08;
unsigned long phoneDetectedAt = 0;
unsigned long messageSentAt = 0;
float offsetCalib = 0;
int GPSSampleDelay = 30000;
bool bikeMovedGPS = false;
bool IMUMoved = false;
void calibrateBMI()
{
    offsetCalib = 0;
    for (int avgCount = 0; avgCount < 100; avgCount++)
    {
        offsetCalib += getBMIData();
        delay(10);
    }
    offsetCalib = ((offsetCalib / 100) - 1);
}

void securityControl()
{
    // first check if the bike has ... Finish another day
}

void sendAlert()
{
}

bool ignition()
{
    return false;
}
bool ownerNear()
{
    return (millis() - phoneDetectedAt < PHONEDETECTIONCOOLDOWN);
}

void movementDetection()
{
    if (!ownerNear())
    { // sentry mode- if no phone near  check for movement- if movement then alert depending on if bike is on or off!

        static float magAccelSum = 0;
        static int loopCount=0;
            magAccelSum += getBMIData();
            loopCount++;
       
        if(loopCount>=30)
        {
            float avg = magAccelSum/30.0;
        if ((fabs(avg  - 1 - offsetCalib) > SHAKETOLERANCE) )
        {
           IMUMoved=true;
           
            Serial.println("Movement threshold met");
        }
    enableMovementDetection=false;
    loopCount=0;
    magAccelSum=0;     
    }
}
}

void checkBMIMovement()
{
    if (!enableMovementDetection && (fabs(getBMIData() - 1 - offsetCalib) > SHAKETOLERANCE))
    {
        enableMovementDetection=true;
    }
}
void checkGPSMovement()
{

    GPSData GPSPosition = getGPSReading();
    if (head == 0 && !GPSDataFilled)
    {
        checkToAdd(GPSPosition);
        return;
    }
    uint16_t last = (head == 0) ? (GPSDataEntries - 1) : (head - 1);
    if (gps.distanceBetween(buffer[last].lat, buffer[last].lon, GPSPosition.lat, GPSPosition.lon) > 5)
    {
        bikeAlarmed = true;
        checkToAdd(GPSPosition);
        bikeMovedGPS = true;
        return;
    }
    checkToAdd(GPSPosition);
}

GPSData getGPSReading()
{
}
void sendGPSData(GPSData &GPSPosition)
{
}

void retrieveGPSData(int day)
{
}

void checkToAdd(GPSData &GPSPosition)
{
    if (head == 0 && !GPSDataFilled)
    {
        buffer[head] = GPSPosition;
        head = 1;
        return;
    }
    uint16_t last = (head == 0) ? (GPSDataEntries - 1) : (head - 1);
    if (gps.distanceBetween(buffer[last].lat, buffer[last].lon, GPSPosition.lat, GPSPosition.lon) < 5 && !bikeAlarmed)
    {
        buffer[last] = GPSPosition;
    }
    else
    {
        buffer[head] = GPSPosition;
        head = ((head + 1) % GPSDataEntries);
        if (head == 0)
        {
            GPSDataFilled = true;
        }
    }
}
#include "SystemManager.h"
#include <TinyGPS++.h>
#include "peripherals.h"

#define PHONEDETECTIONCOOLDOWN 5000
#define IMUCOOLDOWN 300000
#define GPSCOOLDOWN 300000
#define MESSAGEDELAY 60000

#define GPSDataEntries 8640
GPSData buffer[GPSDataEntries];
uint16_t head = 0;
bool GPSDataFilled = false;
float SHAKETOLERANCE = 0.08;
unsigned long phoneDetectedAt = 0;
unsigned long messageSentAt = 0;
float offsetCalib = 0;
static unsigned long IMUMovement = 0;
static unsigned long GPSEvent = 0;
 static int gpsMoveCounter = 0;
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

bool ignition()
{
    return false;
}
bool ownerNear()
{
    return (millis() - phoneDetectedAt < PHONEDETECTIONCOOLDOWN);
}
bool IMUMoved(){
    return (millis()- IMUMovement < IMUCOOLDOWN);
}
bool GPSMoved(){
     return (millis()- GPSEvent < GPSCOOLDOWN);
}

void checkPresence()
{
    if (ownerNear())
    {
        enableMovementDetection = false;
        return;
    }
        enableMovementDetection = true;
}

void sendAlert(int level)
{
    switch(level){
        
    }
}

void securityControl()
{
    if (!ownerNear()){
    if(IMUMoved()){

        if(!ignition){
        sendAlert(1);
        }else{
        sendAlert(0);
        }
    }
    if(GPSMoved()){
        if(!ignition){
    sendAlert(4);
    }else{sendAlert(3);
    }


    }
}
}

void updateGPSCheckSpeed(){
    GPSCheckDelay = ownerNear() ? 100000 : (GPSMoved() ? 500 : 30000);
}


void movementDetection()
{
   
     // sentry mode- if no phone near  check for movement- if movement then alert depending on if bike is on or off!

        static float magAccelSum = 0;
        static int loopCount = 0;
        magAccelSum += getBMIData();
        loopCount++;

        if (loopCount >= 30)
        {
            float avg = magAccelSum / 30.0;
            if ((fabs(avg - 1 - offsetCalib) > SHAKETOLERANCE))
            {
                IMUMovement = millis();

                Serial.println("Movement threshold met");
            }
            enableMovementDetection = false;
            loopCount = 0;
            magAccelSum = 0;
        }
    }

void checkBMIMovement()
{
    if (!enableMovementDetection && (fabs(getBMIData() - 1 - offsetCalib) > SHAKETOLERANCE))
    {
        enableMovementDetection = true;
    }
}

void checkGPSMovement()
{

    GPSData GPSPosition = externalGPSData();
    if (!GPSPosition.isValid || GPSPosition.sats<4)
    {
        return;
    }
    if (head == 0 && !GPSDataFilled)
    {
        checkToAdd(GPSPosition);
        return;
    }
    uint16_t last = (head == 0) ? (GPSDataEntries - 1) : (head - 1);
    float dist = gps.distanceBetween(buffer[last].lat/1e7, buffer[last].lon/1e7, GPSPosition.lat/1e7, GPSPosition.lon/1e7);

    if (dist > 5)
    {
        gpsMoveCounter++;
    }
    else{
         gpsMoveCounter = 0;
    }
    if (gpsMoveCounter >= 3)
{
    GPSEvent = millis();
    gpsMoveCounter = 0;
}
    checkToAdd(GPSPosition);
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
    if (gps.distanceBetween(buffer[last].lat/1e7, buffer[last].lon/1e7, GPSPosition.lat/1e7, GPSPosition.lon/1e7) < 2)
    {
        buffer[last].dayIndex = GPSPosition.dayIndex;
           buffer[last].secondsSinceMidnight = GPSPosition.secondsSinceMidnight;
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
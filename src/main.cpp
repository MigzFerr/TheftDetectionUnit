
#include <Arduino.h>
#include "peripherals.h"
#include "pins.h"
 #include <DFRobot_BMI160.h>
 #include "BLEDevice.h"
 #include <BLEAdvertising.h>
 #include <BLEScan.h>   
#include <BLEAdvertisedDevice.h> 

#define PHONEDETECTIONCOOLDOWN 120000
#define MESSAGEDELAY 60000
float SHAKETOLERANCE = 0.1;
 unsigned long phoneDetectedAt=0;
unsigned long messageSentAt=0;
float offsetCalib = 0;
bool ignition=true;//change to method detection once proto/built



bool ownerNear(){
return(millis() - phoneDetectedAt < PHONEDETECTIONCOOLDOWN);
}

void calibrateBMI(){
  offsetCalib = 0;
for (int avgCount= 0; avgCount<100; avgCount++){
offsetCalib+=getBMIData();
delay(10);
}
offsetCalib=((offsetCalib/100)-1);
}

void sendAlert(){
}



void setup() {
    SerialMon.begin(115200);
    delay(1000);
  setupBLE();
  setupGPS();
  setupBMI();
  calibrateBMI();
}
void loop() {
//sentry mode- if no phone near and bike isn't on, check for movement- if movement then alert!
  if(ignition==false && !phonePresent){
    float magAccelAvg=0;
for (int avgCount= 0; avgCount<30; avgCount++){
magAccelAvg+=getBMIData();
delay(10);
}
magAccelAvg=magAccelAvg/30;

if(abs(magAccelAvg-1-offsetCalib)>SHAKETOLERANCE){
sendAlert();
}
Serial.println("acceleration I think");
Serial.println(abs(magAccelAvg-1-offsetCalib));
  }
/*
if phone detected once, have a cooldown period between phone detection and theft detection- dont alert user for a couple minutes
If we detect that the phone is nearby, dont worry about bike being shaken or ignition starting 
Otherwise if phone isnt nearby and bike is started then dont worry about shaking but send alert to owner 
If bike is shaken and phone nor key are present then send special alert to user 


*/


//externalGPSData();
}


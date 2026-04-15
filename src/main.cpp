

    //TOWRITE: AT COMMANDS ETC FOR CONNECTING MODEM TO NETWORK, THEN WRITE METHOD/CLASS FOR SENDING ALERTS
    //POLISH AND TEST SYSTEM MANAGER STUFF
#include "peripherals.h"
#include "SystemManager.h"
#include "pins.h"
static unsigned long lastBMICheck = 0;
static unsigned long lastGPSCheck = 0;
static unsigned long validateBMICheck = 0;
 bool enableMovementDetection=false;
void setup()
{
  SerialMon.begin(115200);
  delay(1000);
  setupBLE();
  initModemAndGPS();
  setupBMI();
  calibrateBMI();
}

void loop()
{   
    if (millis() - lastBMICheck > 100)
    {
        checkBMIMovement();
        lastBMICheck = millis();
    }
     if (millis() - lastGPSCheck > 600000)
    {
        checkGPSMovement();
        lastGPSCheck = millis();
    }
    if(enableMovementDetection){
if(millis() -validateBMICheck>10){
      movementDetection();
    validateBMICheck=millis();
    }
}
}

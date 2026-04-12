
#include <Arduino.h>
#include "modem.h"
#include "pins.h"
 #include <DFRobot_BMI160.h>
//GYRO SETUP
DFRobot_BMI160 bmi160;
void setup() {
    SerialMon.begin(115200);
    delay(1000);
   
      Wire.begin(I2C_SDA, I2C_SCL);
 if (bmi160.I2cInit(BMI160_ADDR)!= BMI160_OK) {
        Serial.println("Gyro init bad");
        while(1);
    }

  setupGPS();
}
void loop() {
 int i = 0;
  int16_t accelGyro[6]={0};
  int rslt = bmi160.getAccelGyroData(accelGyro);
//  get both accel and gyro data from bmi160
//  parameter accelGyro is the pointer to store the data
 
  if(rslt == 0){
    for(i=0;i<6;i++){
      if (i<3){
        //the first three are gyro datas
        Serial.print(accelGyro[i]*PI/180.0);Serial.print("\t");
      }else{
        //the following three data are accel datas
        Serial.print(accelGyro[i]/16384.0);Serial.print("\t");
      }
    }
    Serial.println();
  }else{
    Serial.println("err");
  }
 delay(500);
  //externalGPSData();
}

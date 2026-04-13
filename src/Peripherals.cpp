#include "Peripherals.h"
#include "pins.h"
#include <Arduino.h>
#include <TinyGPS++.h>
#include "BLEDevice.h"
 #include <BLEAdvertising.h>
 #include <BLEScan.h>   
  #include <DFRobot_BMI160.h>
TinyGPSPlus gps;
BLEScan* pBLEScan;
volatile bool phonePresent = false;
volatile bool scanDone = false;
volatile int foundRSSI = 0;


void powerModem() { 
    // Keep BOARD_POWER_ON HIGH
    pinMode(BOARD_POWER_ON, OUTPUT);
    digitalWrite(BOARD_POWER_ON, HIGH);

    // PWR_KEY pulse
    pinMode(MODEM_PWR_PIN, OUTPUT);
    digitalWrite(MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(MODEM_PWR_PIN, HIGH);
    delay(1200); // 1.2s HIGH pulse
    digitalWrite(MODEM_PWR_PIN, LOW);

    // DTR
    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, LOW);
}   

// SAT SETUP
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_DEBUG SerialMon
#include <TinyGsm.h>
TinyGsm modem(SerialAT);
HardwareSerial GPS2Serial(2);
DFRobot_BMI160 bmi160;
void setupGPS() {
    GPS2Serial.begin(9600, SERIAL_8N1, 41, 42);

    SerialMon.println("Starting modem...");
    powerModem();

    SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RXD_PIN, MODEM_TXD_PIN);

    SerialMon.print("Testing AT commands... ");
    if (modem.testAT(2000)) {
        SerialMon.println("Modem responded!");
        SerialMon.print("Modem Info: ");
        SerialMon.println(modem.getModemInfo());
    } else {
        SerialMon.println("No response. Check RX/TX and power.");
    }

    // Turn on internal GPS
    modem.sendAT("+CGNSSPWR=1");
    modem.waitResponse(1000);
}

void externalGPSData() {
    SerialMon.println("--- External GPS (M10Q) ---");
    unsigned long start = millis();
    while (millis() - start < 5000) {
        while (GPS2Serial.available()) {
            gps.encode(GPS2Serial.read()); // feed NMEA chars to TinyGPS++
        }
    }
    if (gps.location.isValid()) {
        SerialMon.print("Lat: ");       SerialMon.println(gps.location.lat(), 6);
        SerialMon.print("Lng: ");       SerialMon.println(gps.location.lng(), 6);
        SerialMon.print("Satellites: "); SerialMon.println(gps.satellites.value());
        SerialMon.print("Speed km/h: "); SerialMon.println(gps.speed.kmph());
        SerialMon.print("Altitude m: "); SerialMon.println(gps.altitude.meters());
    } else {
        SerialMon.println("No fix yet...");
    }
}

void internalGPSData() {
    SerialMon.println("--- Internal GPS (A7608) ---");
    SerialAT.println("AT+CGNSSINFO");

    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (modem.stream.available()) {
            String line = modem.stream.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;

            if (line.startsWith("+CGNSSINFO:")) {
                SerialMon.println(line);

                int firstComma  = line.indexOf(',');
                int secondComma = line.indexOf(',', firstComma + 1);
                int thirdComma  = line.indexOf(',', secondComma + 1);

                if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
                    int gpsS = line.substring(firstComma + 1, secondComma).toInt();
                    int glo  = line.substring(secondComma + 1, thirdComma).toInt();
                    int bds  = line.substring(thirdComma + 1, line.indexOf(',', thirdComma + 1)).toInt();

                    if (gpsS + glo + bds > 0) {
                        SerialMon.print("Satellites: ");
                        SerialMon.print("GPS=");     SerialMon.print(gpsS);
                        SerialMon.print(" GLONASS="); SerialMon.print(glo);
                        SerialMon.print(" BEIDOU=");  SerialMon.println(bds);
                    }
                }
            } else if (line.startsWith("OK") || line.startsWith("ERROR")) {
                // ignore
            } else {
                SerialMon.println("Raw: " + line);
            }
        }
    }
}

class ScanCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == "NP3A-Miguel") {
            phonePresent = true;
            foundRSSI = advertisedDevice.getRSSI();
    phoneDetectedAt = millis();  // update timestamp
}
        }
    };


void scanComplete(BLEScanResults results) {
    scanDone = true;  // signal loop() to print
    pBLEScan->clearResults();
    pBLEScan->start(5, scanComplete, false);
}


void setupBLE() {
    SerialMon.println("BLE init");
    BLEDevice::init("");
    SerialMon.println("BLE get scan");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new ScanCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(80);
    pBLEScan->setWindow(30);
    SerialMon.println("BLE starting scan");
    pBLEScan->start(5, scanComplete, false);
    SerialMon.println("BLE scan started");
}

void setupBMI(){
      Wire.begin(I2C_SDA, I2C_SCL);
 if (bmi160.I2cInit(BMI160_ADDR)!= BMI160_OK) {
        Serial.println("Gyro init bad");
        while(1);
    }
}

void printBMIData(){
    int i = 0;
  int16_t accelGyro[6]={0};
  int rslt = bmi160.getAccelGyroData(accelGyro);
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
}


float getBMIData(){
    int i = 0;
    float accelMag=0;
  int16_t accelGyro[6]={0};
  int rslt = bmi160.getAccelGyroData(accelGyro);
  if(rslt == 0){
    for(i=0;i<3;i++){
        //the following three data are accel datas
        //Serial.print(accelGyro[i+3]/16384.0);Serial.print("\t");
        float val = accelGyro[i+3]/16384.0;
        accelMag+=(val * val);
      
    }
    return sqrt(accelMag);
    //Serial.println();
  }else{
    Serial.println("err");
    return 100000;
  }
}

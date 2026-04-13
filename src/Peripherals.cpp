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
         SerialMon.print("Time: ");SerialMon.println(gps.date.day()); SerialMon.println(gps.time.hour()); SerialMon.println(gps.time.minute());SerialMon.println(gps.time.second());
         SerialMon.print("link quality: "); SerialMon.println(gps.location.FixQuality(),6);
    } else {
        SerialMon.println("No fix yet...");
    }
}
void internalGPSData() {

    //COURTESY OF AI, TEST CODE TO ENSURE INTERNAL CAN PROVIDE USEFUL DATA COMPARED TO EXTERNAL
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

                // --- Split CSV fields ---
                // we will parse manually step-by-step
                int idx[20];
                int count = 0;

                idx[count++] = line.indexOf(':');
                for (int i = 0; i < 18; i++) {
                    int next = line.indexOf(',', idx[count - 1] + 1);
                    if (next == -1) break;
                    idx[count++] = next;
                }

                auto field = [&](int n) -> String {
                    int start = (n == 0) ? line.indexOf(':') + 1 : idx[n - 1] + 1;
                    int end   = (n < count) ? idx[n] : line.length();
                    return line.substring(start, end);
                };

                // --- Extract key values ---
                int gpsSats = field(1).toInt();
                int gloSats = field(2).toInt();
                int bdsSats = field(3).toInt();

                String lat  = field(4);
                String ns   = field(5);
                String lon  = field(6);
                String ew   = field(7);

                float speed = field(8).toFloat();
                float course = field(9).toFloat();

                String date = field(10); // usually DDMMYY
                String time = field(11); // HHMMSS.S

                float hdop = field(12).toFloat();
                float alt  = field(13).toFloat();

                int fix = field(14).toInt();

                // --- Print clean output ---
                SerialMon.println("---- Parsed GNSS ----");

                SerialMon.printf("Satellites: GPS=%d GLONASS=%d BEIDOU=%d\n",
                                 gpsSats, gloSats, bdsSats);

                SerialMon.print("Lat: "); SerialMon.print(lat); SerialMon.println(ns);
                SerialMon.print("Lon: "); SerialMon.print(lon); SerialMon.println(ew);

                SerialMon.print("Speed (km/h): "); SerialMon.println(speed);
                SerialMon.print("Course (deg): "); SerialMon.println(course);

                SerialMon.print("Date: "); SerialMon.println(date);
                SerialMon.print("Time: "); SerialMon.println(time);

                SerialMon.print("HDOP: "); SerialMon.println(hdop);
                SerialMon.print("Altitude: "); SerialMon.println(alt);

                SerialMon.print("Fix: "); SerialMon.println(fix);

                // --- Example: useful derived logic ---
                if (fix > 0 && gpsSats + gloSats + bdsSats >= 4 && hdop < 5.0) {
                    SerialMon.println("✔ GOOD FIX");
                } else {
                    SerialMon.println("✖ WEAK FIX");
                }
            }
        }
    }
}

class ScanCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == "NP3A-Miguel") {
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

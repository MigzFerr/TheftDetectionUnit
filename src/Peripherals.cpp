#include "Peripherals.h"
#include "pins.h"
#include <Arduino.h>
#include <TinyGPS++.h>
#include "BLEDevice.h"
#include <DFRobot_BMI160.h>
TinyGPSPlus gps;
BLEScan *pBLEScan;
volatile bool scanDone = false;
volatile int foundRSSI = 0;

void powerModem()
{
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
}

// SAT SETUP
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_DEBUG SerialMon
#include <TinyGsm.h>
TinyGsm modem(SerialAT);
HardwareSerial ExtGpsSerial(2);
DFRobot_BMI160 bmi160;

void initModemAndGPS()
{
    ExtGpsSerial.begin(9600, SERIAL_8N1, 41, 42);
    SerialAT.begin(UART_BAUD, SERIAL_8N1, MODEM_RXD_PIN, MODEM_TXD_PIN);
    SerialMon.println("Powering Modem");
    powerModem();
    delay(2000);

    for (int attempt = 0; attempt < 5; attempt++)
    {
        if (attempt == 2)
        {
            powerModem();
            delay(5000);
        }
        SerialMon.print("Attempting to connect to modem, attempt: ");
        SerialMon.println(attempt);
        if (modem.testAT(2000))
        {
            SerialMon.println("Modem connected");
            SerialMon.print("Modem Info: ");
            SerialMon.println(modem.getModemInfo());
            return;
        }
        SerialMon.println("No response. Retrying connection");
    }
    ESP.restart();
}
void updateGPS()
{
    while (ExtGpsSerial.available())
    {
        gps.encode(ExtGpsSerial.read());
    }
}
GPSData externalGPSData()
{
    GPSData sendData;
    if(!gps.location.isValid()){
        sendData.isValid=false; 
        return sendData;
    }
    SerialMon.println("--- External GPS (M10Q) ---");
      
    
        SerialMon.print("Lat: ");
        SerialMon.println(gps.location.lat(), 6);
        sendData.lat=(gps.location.lat()* 1e7);
        SerialMon.print("Lng: ");
        SerialMon.println(gps.location.lng(), 6);
        sendData.lon=(gps.location.lng()* 1e7);
        SerialMon.print("Satellites: ");
        SerialMon.println(gps.satellites.value());
         sendData.sats=(gps.satellites.value());
        SerialMon.print("Time: ");
        SerialMon.println(gps.date.day());
        SerialMon.println(gps.time.hour());
        SerialMon.println(gps.time.minute());
        SerialMon.println(gps.time.second());
        sendData.dayIndex=(gps.date.day());
        sendData.secondsSinceMidnight=(gps.time.hour() * 3600  + gps.time.minute() * 60 + gps.time.second());
        SerialMon.print("HDOP: ");
        SerialMon.println(gps.hdop.value());
        sendData.hdop=gps.hdop.value();
        return sendData;
}

class ScanCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.getName() == "NP3A-Miguel")
        {
            foundRSSI = advertisedDevice.getRSSI();
            phoneDetectedAt = millis(); // update timestamp
        }
    }
};

void scanComplete(BLEScanResults results)
{
    scanDone = true; // signal loop() to print
    pBLEScan->clearResults();
    pBLEScan->start(5, scanComplete, false);
}

void setupBLE()
{
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

void setupBMI()
{
    Wire.begin(I2C_SDA, I2C_SCL);
    if (bmi160.I2cInit(BMI160_ADDR) != BMI160_OK)
    {
        Serial.println("Gyro init bad");
        while (1)
            ;
    }
}

void printBMIData()
{
    int i = 0;
    int16_t accelGyro[6] = {0};
    int rslt = bmi160.getAccelGyroData(accelGyro);
    if (rslt == 0)
    {
        for (i = 0; i < 6; i++)
        {
            if (i < 3)
            {
                // the first three are gyro datas
                Serial.print(accelGyro[i] * PI / 180.0);
                Serial.print("\t");
            }
            else
            {
                // the following three data are accel datas
                Serial.print(accelGyro[i] / 16384.0);
                Serial.print("\t");
            }
        }
        Serial.println();
    }
    else
    {
        Serial.println("err");
    }
}

float getBMIData()
{
    int i = 0;
    float accelMag = 0;
    int16_t accelGyro[6] = {0};
    int rslt = bmi160.getAccelGyroData(accelGyro);
    if (rslt == 0)
    {
        for (i = 0; i < 3; i++)
        {
            float val = accelGyro[i + 3] / 16384.0;
            accelMag += (val * val);
        }
        return sqrt(accelMag);
        // Serial.println();
    }
    else
    {
        Serial.println("err");
        return 100000;
    }
}

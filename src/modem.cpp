#include "modem.h"
#include "pins.h"
#include <Arduino.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

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

void doBS(){

    
}
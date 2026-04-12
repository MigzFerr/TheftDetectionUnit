
#include <Arduino.h>
#include "modem.h"
#include "pins.h"


#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#define TINY_GSM_DEBUG SerialMon
#include <TinyGsm.h>
TinyGsm modem(SerialAT);
 HardwareSerial GPS2Serial(2);
 

void setup() {
    SerialMon.begin(115200);
    
    delay(1000);
   
    GPS2Serial.begin(9600, SERIAL_8N1, 41, 42); // RX=41, TX=42


    SerialMon.println("Starting modem test...");
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

    // Turn on GPS
    modem.sendAT("+CGNSSPWR=1");
    modem.waitResponse(1000);
}
void loop() {
   
     SerialMon.println("external gps data");
     unsigned long start = millis();
     while (millis() - start < 5000) { // read for 5 seconds
        if (GPS2Serial.available()) {
            String line = GPS2Serial.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;

            if (line.startsWith("+CGNSSINFO:")) {
                SerialMon.println(line); // just print the raw line

                // Optional: parse satellites
                int firstComma = line.indexOf(',');
                int secondComma = line.indexOf(',', firstComma + 1);
                int thirdComma = line.indexOf(',', secondComma + 1);

                if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
                    int gps  = line.substring(firstComma + 1, secondComma).toInt();
                    int glo  = line.substring(secondComma + 1, thirdComma).toInt();
                    int bds  = line.substring(thirdComma + 1, line.indexOf(',', thirdComma + 1)).toInt();

                    if (gps + glo + bds > 0) {
                        SerialMon.print("Satellites: ");
                        SerialMon.print("GPS=");
                        SerialMon.print(gps);
                        SerialMon.print(" GLONASS=");
                        SerialMon.print(glo);
                        SerialMon.print(" BEIDOU=");
                        SerialMon.println(bds);
                    }
                }
            } else if (line.startsWith("OK") || line.startsWith("ERROR")) {
                // ignore OK/ERROR lines
            } else {
                SerialMon.println("Raw: " + line);
            }
        }
    }
    SerialMon.println("--------\n\n internal gps data");
      SerialAT.println("AT+CGNSSINFO");
    unsigned long start2 = millis();
    while (millis() - start2 < 5000) { // read for 5 seconds
        if (modem.stream.available()) {
            String line = modem.stream.readStringUntil('\n');
            line.trim();
            if (line.length() == 0) continue;

            if (line.startsWith("+CGNSSINFO:")) {
                SerialMon.println(line); // just print the raw line

                // Optional: parse satellites
                int firstComma = line.indexOf(',');
                int secondComma = line.indexOf(',', firstComma + 1);
                int thirdComma = line.indexOf(',', secondComma + 1);

                if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
                    int gps  = line.substring(firstComma + 1, secondComma).toInt();
                    int glo  = line.substring(secondComma + 1, thirdComma).toInt();
                    int bds  = line.substring(thirdComma + 1, line.indexOf(',', thirdComma + 1)).toInt();

                    if (gps + glo + bds > 0) {
                        SerialMon.print("Satellites: ");
                        SerialMon.print("GPS=");
                        SerialMon.print(gps);
                        SerialMon.print(" GLONASS=");
                        SerialMon.print(glo);
                        SerialMon.print(" BEIDOU=");
                        SerialMon.println(bds);
                    }
                }
            } else if (line.startsWith("OK") || line.startsWith("ERROR")) {
                // ignore OK/ERROR lines
            } else {
                SerialMon.println("Raw: " + line);
            }
        }
    }

    delay(3000); // wait 3 seconds before next request
}

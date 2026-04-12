//DEFINES FOR WHAT PINS CORRESPOND TO MODEM/GPS/BMI
#ifndef PINS_H
#define PINS_H
//Modem pin defines 
#define UART_BAUD 115200
#define MODEM_DTR_PIN 7
#define MODEM_TXD_PIN 17
#define MODEM_RXD_PIN 18
#define MODEM_PWR_PIN 15
#define BOARD_POWER_ON 12

//Serial ouput defines
#define SerialAT Serial1
#define SerialMon Serial

//I2c pins
#define I2C_SDA          8
#define I2C_SCL          9
#define BMI160_ADDR      0x68

#endif
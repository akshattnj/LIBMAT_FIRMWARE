#ifndef DEFINATIONS_H
#define DEFINATIONS_H

#define SDA_0_PIN CONFIG_I2C_0_SDA
#define SCL_0_PIN CONFIG_I2C_0_SCL
#define I2C_0_CLOCK CONFIG_I2C_0_SPEED
#define I2C_WRITE_BUFFER 10
#define I2C_READ_BUFFER 20

#define AHT_ADDRESS 0x38

#define AHT_ENABLED 0b10000000
#define AHT_BUSY    0b10000000
#define AHT_CALIB   0b00001000

#define ADS_ADDRESS 0x48
#define ADS_DATA_RATE 0x0080

#define MAIN_TAG "MAIN"
#define I2C_TAG "I2C"

// #define SCANNER_MODE

#endif

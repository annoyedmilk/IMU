/*
 * LSM9DS1Driver.c
 *
 * Created: 15.05.2017 10:33:07
 *  Author: mburger
 *  Modified by: annoyedmilk
 */

#include <avr/io.h>
#include "LSM9DS1Driver.h" 
#include "LSM9DS1Defines.h"
#include "twiMaster.h"

int16_t gx = 0;
int16_t gy = 0;
int16_t gz = 0;
int16_t ax = 0;
int16_t ay = 0;
int16_t az = 0;
int16_t mx = 0;
int16_t my = 0;
int16_t mz = 0;
float temperature = 0;

void LSM9DS1Init(void) {  
    initI2C();  
    // Initialization code for Gyro, Accel, and Mag
}

void readACCData(void) {
    uint8_t temp[6];
    i2cRead(GYROACCADDRESS, OUT_X_L_XL, 6, temp);
    ax = (temp[1] << 8)| temp[0];
    ay = (temp[3] << 8)| temp[2];
    az = (temp[5] << 8)| temp[4];
}

void readGyroData(void) {
    uint8_t temp[6];
    i2cRead(GYROACCADDRESS, OUT_X_L_G, 6, temp);
    gx = (temp[1] << 8)| temp[0];
    gy = (temp[3] << 8)| temp[2];
    gz = (temp[5] << 8)| temp[4];
}

void readMagData(void) {
    uint8_t temp[6];
    i2cRead(MAGADDRESS, OUT_X_L_M, 6, temp);
    mx = (temp[1] << 8)| temp[0];
    my = (temp[3] << 8)| temp[2];
    mz = (temp[5] << 8)| temp[4];
}

void readTempData(void) {
    uint8_t temp[2];
    int16_t tempValue = 0;
    i2cRead(GYROACCADDRESS, OUT_TEMP_L, 2, temp);
    tempValue = (((int16_t) temp[1] << 12) | temp[0] << 4 ) >> 4;
    temperature *= 0.9;
    temperature += ((float)tempValue / 16)+25;
}

int16_t getGyroData(int axis) {
    switch(axis) {
        case X_AXIS:
            return gx;
        case Y_AXIS:
            return gy;
        case Z_AXIS:
            return gz;
    }
    return 0;
}

int16_t getACCData(int axis) {
    switch(axis) {
        case X_AXIS:
            return ax;
        case Y_AXIS:
            return ay;
        case Z_AXIS:
            return az;
    }
    return 0;
}

int16_t getMagData(int axis) {
    switch(axis) {
        case X_AXIS: 
            return mx;
        case Y_AXIS:
            return my;
        case Z_AXIS: 
            return mz;
    }
    return 0;
}

float getTemperatureData() {
    return temperature / 10;
}
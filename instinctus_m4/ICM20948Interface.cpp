/**
 * ICM20948Interface.cpp - ICM20948 9-DOF IMU Implementation
 * 
 * This file contains the concrete implementation of the IMUInterface for the
 * ICM20948 sensor. It handles all the hardware-specific details of communicating
 * with the ICM20948 chip via I2C and the Adafruit library.
 * 
 * Key Implementation Details:
 * - Uses Adafruit_ICM20948 library for hardware communication
 * - Configures sensor ranges and update rates in initialize()
 * - Extracts accelerometer and gyroscope data from sensor events
 * - Returns data in standard units (m/s² for accel, rad/s for gyro)
 * - Handles sensor read failures gracefully
 * 
 * Sensor Configuration:
 * - Accelerometer: 4G range, ~1.1 Hz rate
 * - Gyroscope: 500 DPS range, ~17.8 Hz rate  
 * - Magnetometer: 10 Hz rate (not used in current implementation)
 * 
 * Error Handling:
 * - initialize() returns false if I2C communication fails
 * - readSensors() returns false if sensor event reading fails
 * - Caller should check return values and handle failures appropriately
 */

#include "ICM20948Interface.h"
#include <InstinctusKit.h>

ICM20948Interface::ICM20948Interface(TwoWire* i2c_bus, uint8_t address)
    : wire(i2c_bus), i2cAddress(address) {
    // Constructor - store I2C bus and address for later use
}

// Map plain int config values to Adafruit library enums
static icm20948_accel_range_t accelRangeEnum(uint8_t g) {
    switch (g) {
        case 2:  return ICM20948_ACCEL_RANGE_2_G;
        case 4:  return ICM20948_ACCEL_RANGE_4_G;
        case 8:  return ICM20948_ACCEL_RANGE_8_G;
        case 16: return ICM20948_ACCEL_RANGE_16_G;
        default: return ICM20948_ACCEL_RANGE_4_G;
    }
}

static icm20948_gyro_range_t gyroRangeEnum(uint16_t dps) {
    switch (dps) {
        case 250:  return ICM20948_GYRO_RANGE_250_DPS;
        case 500:  return ICM20948_GYRO_RANGE_500_DPS;
        case 1000: return ICM20948_GYRO_RANGE_1000_DPS;
        case 2000: return ICM20948_GYRO_RANGE_2000_DPS;
        default:   return ICM20948_GYRO_RANGE_500_DPS;
    }
}

static ak09916_data_rate_t magRateEnum(uint8_t hz) {
    switch (hz) {
        case 10:  return AK09916_MAG_DATARATE_10_HZ;
        case 20:  return AK09916_MAG_DATARATE_20_HZ;
        case 50:  return AK09916_MAG_DATARATE_50_HZ;
        case 100: return AK09916_MAG_DATARATE_100_HZ;
        default:  return AK09916_MAG_DATARATE_10_HZ;
    }
}

bool ICM20948Interface::initialize() {
    // Initialize I2C communication with ICM20948 on specified bus
    if (!icm.begin_I2C(i2cAddress, wire)) {
        return false;
    }

    // Configure sensor ranges and rates from centralized config
    icm.setAccelRange(accelRangeEnum(Config::IMU_ACCEL_RANGE_G));
    icm.setGyroRange(gyroRangeEnum(Config::IMU_GYRO_RANGE_DPS));
    icm.setAccelRateDivisor(Config::IMU_ACCEL_RATE_DIVISOR);
    icm.setGyroRateDivisor(Config::IMU_GYRO_RATE_DIVISOR);
    icm.setMagDataRate(magRateEnum(Config::IMU_MAG_RATE_HZ));

    return true;
}

bool ICM20948Interface::readSensors(float& accelX, float& accelY, float& accelZ,
                                   float& gyroX, float& gyroY, float& gyroZ) {
    // Get sensor events
    sensors_event_t accel, gyro, mag, temp;
    
    if (!icm.getEvent(&accel, &gyro, &mag, &temp)) {
        return false;
    }
    
    // Transform raw sensor axes into robot frame (X=forward, Y=left, Z=up)
    applyTransform(Config::BALANCE_IMU_TRANSFORM,
                   accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
                   accelX, accelY, accelZ);

    applyTransform(Config::BALANCE_IMU_TRANSFORM,
                   gyro.gyro.x, gyro.gyro.y, gyro.gyro.z,
                   gyroX, gyroY, gyroZ);
    
    return true;
}
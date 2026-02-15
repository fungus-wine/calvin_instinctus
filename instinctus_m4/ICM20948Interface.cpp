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

ICM20948Interface::ICM20948Interface(TwoWire* i2c_bus, uint8_t address)
    : wire(i2c_bus), i2cAddress(address) {
    // Constructor - store I2C bus and address for later use
}

bool ICM20948Interface::initialize() {
    // Initialize I2C communication with ICM20948 on specified bus
    if (!icm.begin_I2C(i2cAddress, wire)) {
        return false;
    }
    
    // Configure sensor ranges and rates
    // Note: These should eventually come from HardwareConfig.h
    icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
    icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);
    icm.setAccelRateDivisor(4095);  // ~1.1 Hz
    icm.setGyroRateDivisor(255);    // ~17.8 Hz
    icm.setMagDataRate(AK09916_MAG_DATARATE_10_HZ);
    
    return true;
}

bool ICM20948Interface::readSensors(float& accelX, float& accelY, float& accelZ,
                                   float& gyroX, float& gyroY, float& gyroZ) {
    // Get sensor events
    sensors_event_t accel, gyro, mag, temp;
    
    if (!icm.getEvent(&accel, &gyro, &mag, &temp)) {
        return false;
    }
    
    // Extract accelerometer data (already in m/s²)
    accelX = accel.acceleration.x;
    accelY = accel.acceleration.y;
    accelZ = accel.acceleration.z;
    
    // Extract gyroscope data (already in rad/s)
    gyroX = gyro.gyro.x;
    gyroY = gyro.gyro.y;
    gyroZ = gyro.gyro.z;
    
    return true;
}
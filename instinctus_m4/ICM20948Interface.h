#ifndef ICM20948_INTERFACE_H
#define ICM20948_INTERFACE_H

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "IMUInterface.h"

/**
 * ICM20948Interface.h - ICM20948 9-DOF IMU Implementation
 * 
 * This file implements the IMUInterface for the ICM20948 9-DOF IMU sensor.
 * The ICM20948 includes a 3-axis accelerometer, 3-axis gyroscope, and 3-axis
 * magnetometer, though this implementation only uses the accelerometer and
 * gyroscope for balance control.
 * 
 * Hardware Details:
 * - Communication: I2C (Wire library)
 * - Library: Adafruit ICM20X/ICM20948 libraries
 * - Accelerometer range: Configurable (default 4G)
 * - Gyroscope range: Configurable (default 500 DPS)
 * - Update rates: Configurable for power/performance balance
 * 
 * Configuration:
 * - Sensor ranges and rates are set in initialize()
 * - Currently uses hardcoded values, but could be moved to config file
 * - I2C address is auto-detected by Adafruit library
 * 
 * Usage:
 *   ICM20948Interface imu;
 *   if (imu.initialize()) {
 *       float ax, ay, az, gx, gy, gz;
 *       if (imu.readSensors(ax, ay, az, gx, gy, gz)) {
 *           // Use sensor data...
 *       }
 *   }
 */
class ICM20948Interface : public IMUInterface {
private:
    Adafruit_ICM20948 icm;
    TwoWire* wire;
    uint8_t i2cAddress;

public:
    /**
     * Constructor
     * @param i2c_bus - pointer to I2C bus (e.g., &Wire, &Wire1)
     * @param address - I2C address (default 0x69)
     */
    ICM20948Interface(TwoWire* i2c_bus = &Wire, uint8_t address = 0x69);

    /**
     * Initialize the ICM20948 sensor
     * @return true if initialization successful, false otherwise
     */
    bool initialize() override;
    
    /**
     * Read accelerometer and gyroscope data from ICM20948
     * @param accelX, accelY, accelZ - acceleration in m/s²
     * @param gyroX, gyroY, gyroZ - angular velocity in rad/s
     * @return true if read successful, false otherwise
     */
    bool readSensors(float& accelX, float& accelY, float& accelZ,
                    float& gyroX, float& gyroY, float& gyroZ) override;
};

#endif // ICM20948_INTERFACE_H
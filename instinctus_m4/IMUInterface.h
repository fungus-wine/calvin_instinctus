#ifndef IMU_INTERFACE_H
#define IMU_INTERFACE_H

/**
 * IMUInterface.h - Hardware Abstraction Layer for IMU Sensors
 * 
 * This file defines a simple, clean interface that any IMU hardware can implement.
 * The interface focuses only on the essential functions needed for balance control:
 * initialization and reading accelerometer/gyroscope data.
 * 
 * Design Goals:
 * - Hardware agnostic - works with any IMU chip (ICM20948, MPU6050, etc.)
 * - Simple interface - only 2 methods to implement
 * - Easy to swap - new IMU chips require minimal code changes
 * - No unnecessary complexity - no templates, complex abstractions
 * 
 * Usage:
 * 1. Create a concrete implementation (e.g., ICM20948Interface)
 * 2. Implement initialize() and readSensors() methods
 * 3. Pass to BalanceIMU constructor for dependency injection
 * 
 * Example:
 *   ICM20948Interface imuHardware;
 *   BalanceIMU balanceIMU(&imuHardware);
 */
class IMUInterface {
public:
    /**
     * Initialize the IMU hardware
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * Read accelerometer and gyroscope data
     * @param accelX, accelY, accelZ - acceleration in m/s²
     * @param gyroX, gyroY, gyroZ - angular velocity in rad/s
     * @return true if read successful, false otherwise
     */
    virtual bool readSensors(float& accelX, float& accelY, float& accelZ,
                            float& gyroX, float& gyroY, float& gyroZ) = 0;
    
    virtual ~IMUInterface() = default;
};

#endif // IMU_INTERFACE_H
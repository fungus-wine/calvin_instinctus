#ifndef BALANCE_IMU_H
#define BALANCE_IMU_H

#include <Arduino.h>
#include "IMUInterface.h"

// Forward declaration
class BalanceObserver;

/**
 * BalanceIMU.h - Main Balance Control IMU System
 * 
 * This is the core balance control class that processes IMU sensor data and
 * calculates tilt angles for balance control. It combines accelerometer and
 * gyroscope data using a complementary filter to provide smooth, accurate
 * tilt measurements for real-time balance control.
 * 
 * Key Features:
 * - Hardware abstraction via IMUInterface (works with any IMU chip)
 * - Complementary filter for smooth, drift-free tilt calculation
 * - Observer pattern for real-time event notifications
 * - Optimized for 100Hz update rate (10ms per cycle)
 * - Emergency tilt detection for safety systems
 * 
 * Technical Details:
 * - Uses complementary filter (98% gyro, 2% accelerometer)
 * - Calculates tilt angle from Y-axis (forward/backward for balance robot)
 * - Notifies multiple observers on significant changes (>1°) and emergencies (>45°)
 * - Supports multiple observer types (motor control, event broadcasting, logging, etc.)
 * - Maintains minimal state for fast processing
 * 
 * Coordinate System (X-Forward Convention):
 * - X: Forward/Backward (primary balance axis)
 * - Y: Left/Right (not used for balance)
 * - Z: Up/Down (gravity reference)
 * - Tilt angle: Positive = forward tilt, Negative = backward tilt
 * 
 * Usage Pattern:
 *   ICM20948Interface imuHardware;
 *   MotorController motorController;
 *   BalanceEventObserver eventObserver;
 *   
 *   BalanceIMU balanceIMU(&imuHardware);
 *   balanceIMU.addObserver(&motorController);
 *   balanceIMU.addObserver(&eventObserver);
 *   
 *   balanceIMU.initialize();
 *   while (true) {
 *       balanceIMU.update();  // Call at 100Hz
 *       delay(10);
 *   }
 * 
 * Performance:
 * - update() should be called every 10ms (100Hz) for best results
 * - Each update cycle takes <1ms on Arduino GIGA R1 WiFi
 * - Observer callbacks are synchronous (called within update())
 */
class BalanceIMU {
private:
    IMUInterface* imu;
    BalanceObserver* _observer;

    // Current sensor readings
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;

    // Calculated balance values
    float currentTiltAngle;

    // Configuration
    static constexpr float TILT_ALPHA = 0.98; // Complementary filter coefficient

    unsigned long lastUpdateTime;

    // Internal calculation methods
    float calculateTiltFromAccel();
    float applyComplementaryFilter(float accelTilt, float gyroRate, float deltaTime);

public:
    /**
     * Constructor - only IMU hardware required, observer set separately
     * @param imuHardware - pointer to IMU hardware implementation
     */
    BalanceIMU(IMUInterface* imuHardware);

    /**
     * Set the balance observer to receive balance events
     * @param observer - pointer to observer object
     */
    void setObserver(BalanceObserver* observer);
    
    
    /**
     * Initialize the IMU system
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * Update sensor readings and calculate balance state
     * Call this regularly (e.g., every 10ms) for real-time balance control
     */
    void update();
    
    /**
     * Get current tilt angle in degrees
     * @return tilt angle (-90 to +90 degrees, 0 = upright)
     */
    float getTiltAngle() const;
    
    /**
     * Get raw accelerometer readings
     */
    void getAcceleration(float& x, float& y, float& z) const;
    
    /**
     * Get raw gyroscope readings  
     */
    void getAngularVelocity(float& x, float& y, float& z) const;
};

#endif // BALANCE_IMU_H
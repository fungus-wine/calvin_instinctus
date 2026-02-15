#ifndef DRIVE_COORDINATOR_H
#define DRIVE_COORDINATOR_H

#include "MotorInterface.h"

/**
 * DriveCoordinator.h - Two-Motor Drive System Controller
 * 
 * This class coordinates left and right motors for a balance robot drive system.
 * It provides high-level control methods that operate on both motors together,
 * handling the common patterns needed for balance and steering control.
 * 
 * Key Features:
 * - Independent left/right motor speed control
 * - Synchronized stop functionality
 * - Status monitoring for both motors
 * - Hardware abstraction via MotorInterface
 * 
 * Usage Patterns:
 * 1. Balance Control: Both motors same speed (forward/backward)
 * 2. Steering: Different motor speeds (turning)
 * 3. Emergency Stop: Immediate halt of both motors
 * 
 * Example:
 *   ODriveS1Interface leftMotor(0x01);
 *   ODriveS1Interface rightMotor(0x02);
 *   DriveCoordinator drive(&leftMotor, &rightMotor);
 *   
 *   // Balance: both motors forward
 *   drive.setMotorSpeeds(100.0, 100.0);
 *   
 *   // Turn right: left faster than right
 *   drive.setMotorSpeeds(120.0, 80.0);
 *   
 *   // Emergency stop
 *   drive.stop();
 */
class DriveCoordinator {
private:
    MotorInterface* leftMotor;
    MotorInterface* rightMotor;
    
public:
    /**
     * Constructor - requires both motor interfaces
     * @param left - pointer to left motor controller
     * @param right - pointer to right motor controller  
     */
    DriveCoordinator(MotorInterface* left, MotorInterface* right);
    
    /**
     * Initialize both motor controllers
     * @return true if both motors initialize successfully
     */
    bool initialize();
    
    /**
     * Set individual motor speeds
     * @param leftRpm - left motor velocity in RPM
     * @param rightRpm - right motor velocity in RPM
     */
    void setMotorSpeeds(float leftRpm, float rightRpm);
    
    /**
     * Emergency stop both motors immediately
     */
    void stop();
    
    /**
     * Check if both motors are ready for commands
     * @return true if both motors are ready, false if either has issues
     */
    bool bothMotorsReady();
    
    /**
     * Get current velocities of both motors
     * @param leftRpm - reference to store left motor velocity
     * @param rightRpm - reference to store right motor velocity
     */
    void getMotorSpeeds(float& leftRpm, float& rightRpm);
    
    /**
     * Get current positions of both motors (for odometry)
     * @param leftPos - reference to store left motor position
     * @param rightPos - reference to store right motor position
     */
    void getMotorPositions(float& leftPos, float& rightPos);
};

#endif // DRIVE_COORDINATOR_H
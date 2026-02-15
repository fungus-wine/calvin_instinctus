#ifndef MOTOR_INTERFACE_H
#define MOTOR_INTERFACE_H

/**
 * MotorInterface.h - Hardware Abstraction Layer for Motor Controllers
 * 
 * This file defines a simple, clean interface that any motor controller can implement.
 * The interface focuses on the essential functions needed for balance robot control:
 * velocity control, position feedback, and status monitoring.
 * 
 * Design Goals:
 * - Hardware agnostic - works with any motor controller (ODrive, VESC, etc.)
 * - Simple interface - only essential methods for balance control
 * - Easy to swap - new motor controllers require minimal code changes
 * - Real-time friendly - fast, non-blocking operations
 * 
 * Usage:
 * 1. Create a concrete implementation (e.g., ODriveS1Interface)
 * 2. Implement all virtual methods for specific hardware
 * 3. Pass to DriveCoordinator for high-level control
 * 
 * Example:
 *   ODriveS1Interface leftMotor(0x01);
 *   ODriveS1Interface rightMotor(0x02);
 *   DriveCoordinator drive(&leftMotor, &rightMotor);
 */
class MotorInterface {
public:
    /**
     * Initialize the motor controller hardware
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;
    
    /**
     * Set motor velocity
     * @param rpm - target velocity in revolutions per minute
     */
    virtual void setVelocity(float rpm) = 0;
    
    /**
     * Get current motor velocity
     * @return current velocity in RPM
     */
    virtual float getVelocity() = 0;
    
    /**
     * Get current motor position
     * @return position in revolutions (for odometry)
     */
    virtual float getPosition() = 0;
    
    /**
     * Check if motor controller is ready for commands
     * @return true if ready, false if in error state or not initialized
     */
    virtual bool isReady() = 0;
    
    /**
     * Emergency stop - immediately halt motor
     */
    virtual void stop() = 0;
    
    virtual ~MotorInterface() = default;
};

#endif // MOTOR_INTERFACE_H
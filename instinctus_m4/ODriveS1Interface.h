#ifndef ODRIVE_S1_INTERFACE_H
#define ODRIVE_S1_INTERFACE_H

#include <Arduino.h>
#include "MotorInterface.h"

/**
 * ODriveS1Interface.h - ODrive S1 Motor Controller Implementation
 * 
 * This file implements the MotorInterface for ODrive S1 motor controllers
 * using CAN bus communication. The ODrive S1 is a high-performance motor
 * controller with built-in velocity control, position feedback, and safety features.
 * 
 * Hardware Details:
 * - Communication: CAN bus (faster and more robust than UART)
 * - Control Mode: Velocity control (perfect for balance robots)
 * - Feedback: Built-in encoder position and velocity estimation
 * - Safety: Overcurrent protection, error state monitoring
 * 
 * CAN Protocol:
 * - Each ODrive has unique CAN ID (0x01 for left, 0x02 for right)
 * - Standard ODrive CAN commands for velocity control
 * - Real-time feedback via CAN status messages
 * 
 * Configuration:
 * - ODrive must be pre-configured for velocity control mode
 * - CAN bitrate: typically 250kbps or 500kbps
 * - Motor calibration completed before use
 * 
 * Usage:
 *   ODriveS1Interface leftMotor(0x01);   // CAN ID 0x01
 *   ODriveS1Interface rightMotor(0x02);  // CAN ID 0x02
 *   
 *   if (leftMotor.initialize()) {
 *       leftMotor.setVelocity(100.0);  // 100 RPM
 *   }
 */
class ODriveS1Interface : public MotorInterface {
private:
    uint8_t canID;           // CAN bus identifier for this ODrive
    
    // Motor state tracking
    float currentVelocity;   // Last reported velocity (RPM)
    float currentPosition;   // Last reported position (revolutions)
    bool motorReady;         // ODrive status flag
    
    // CAN communication timing
    unsigned long lastStatusTime;
    static constexpr unsigned long STATUS_TIMEOUT = 100;  // ms
    
    // CAN message IDs (based on ODrive CAN protocol)
    uint32_t getVelocityCommandID();
    uint32_t getStatusRequestID();
    uint32_t getStatusResponseID();
    
    // Internal CAN communication methods
    bool sendVelocityCommand(float rpm);
    bool requestStatus();
    bool readStatusResponse();
    void updateMotorStatus();
    
public:
    /**
     * Constructor - specify CAN ID for this ODrive
     * @param id - CAN bus ID (typically 0x01, 0x02, etc.)
     */
    ODriveS1Interface(uint8_t id);
    
    /**
     * Initialize CAN communication and verify ODrive connection
     * @return true if ODrive responds and is ready
     */
    bool initialize() override;
    
    /**
     * Set motor velocity via CAN command
     * @param rpm - target velocity in revolutions per minute
     */
    void setVelocity(float rpm) override;
    
    /**
     * Get current motor velocity from last status update
     * @return current velocity in RPM
     */
    float getVelocity() override;
    
    /**
     * Get current motor position from last status update
     * @return position in revolutions (for odometry)
     */
    float getPosition() override;
    
    /**
     * Check if ODrive is ready and responding
     * @return true if motor is ready, false if error or timeout
     */
    bool isReady() override;
    
    /**
     * Emergency stop - send immediate stop command via CAN
     */
    void stop() override;
    
    /**
     * Update method to be called regularly for CAN communication
     * Requests status updates and processes incoming messages
     */
    void update();
};

#endif // ODRIVE_S1_INTERFACE_H
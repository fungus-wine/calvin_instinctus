/**
 * ODriveS1Interface.cpp - ODrive S1 CAN Communication Implementation
 * 
 * This file implements CAN bus communication with ODrive S1 motor controllers
 * using the ACANFD_GIGA_R1 library and the official ODrive CAN protocol.
 * 
 * CAN Protocol Details:
 * - Message ID format: Node ID (6 bits) << 5 | Command ID (5 bits)
 * - Velocity command: 0x00D (Set Input Vel)
 * - Encoder feedback: 0x009 (Get Encoder Estimates)
 * - Data format: 32-bit IEEE 754 floats, little-endian
 * 
 * Hardware Requirements:
 * - Arduino GIGA R1 WiFi with built-in FDCAN controller
 * - CAN transceiver (e.g., SN65HVD230) to convert to differential signals
 * - Proper CAN bus termination (120Ω resistors)
 * 
 * Key ODrive Commands:
 * - 0x009: Get Encoder Estimates (position, velocity feedback)
 * - 0x00D: Set Input Vel (velocity command + torque feedforward)
 */

#include "ODriveS1Interface.h"
#include <ACANFD_GIGA_R1.h>

// Global CAN object for GIGA R1
// Note: ACANFD_GIGA_R1 typically uses a singleton pattern

ODriveS1Interface::ODriveS1Interface(uint8_t id) 
    : canID(id), currentVelocity(0.0), currentPosition(0.0), 
      motorReady(false), lastStatusTime(0) {
}

bool ODriveS1Interface::initialize() {
    // Configure CAN bus settings
    ACANFD_GIGA_R1_Settings settings(250000); // 250 kbps - common for ODrive
    
    // Configure for standard CAN (not CAN FD) to match ODrive
    settings.mRequestedMode = ACANFD_GIGA_R1_Settings::Normal;
    
    // Initialize the CAN controller
    const uint32_t errorCode = ACANFD_GIGA_R1::begin(settings);
    
    if (errorCode != 0) {
        Serial.print("CAN initialization failed, error code: 0x");
        Serial.println(errorCode, HEX);
        return false;
    }
    
    Serial.print("ODrive CAN ID 0x");
    Serial.print(canID, HEX);
    Serial.println(" initialized successfully");
    
    // Request initial status from ODrive
    delay(100); // Allow ODrive to boot
    requestStatus();
    
    motorReady = true;
    return true;
}

void ODriveS1Interface::setVelocity(float rpm) {
    // Convert RPM to radians per second (ODrive velocity units)
    // RPM * (2π / 60) = rad/s
    float velocityRadPerSec = rpm * (2.0 * PI / 60.0);
    
    sendVelocityCommand(velocityRadPerSec);
}

float ODriveS1Interface::getVelocity() {
    return currentVelocity;
}

float ODriveS1Interface::getPosition() {
    return currentPosition;
}

bool ODriveS1Interface::isReady() {
    unsigned long currentTime = millis();
    bool statusRecent = (currentTime - lastStatusTime) < STATUS_TIMEOUT;
    
    return motorReady && statusRecent;
}

void ODriveS1Interface::stop() {
    setVelocity(0.0);
}

void ODriveS1Interface::update() {
    // Request encoder estimates periodically
    static unsigned long lastRequest = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastRequest > 50) {  // Request every 50ms (20Hz)
        requestStatus();
        lastRequest = currentTime;
    }
    
    // Process any incoming messages
    readStatusResponse();
    updateMotorStatus();
}

// Private methods - CAN communication implementation

uint32_t ODriveS1Interface::getVelocityCommandID() {
    // ODrive CAN protocol: can_id = axis_id << 5 | cmd_id
    // Command 0x00D = Set Input Vel
    return (canID << 5) | 0x00D;
}

uint32_t ODriveS1Interface::getStatusRequestID() {
    // Command 0x009 = Get Encoder Estimates
    return (canID << 5) | 0x009;
}

uint32_t ODriveS1Interface::getStatusResponseID() {
    // Same as request ID for encoder estimates
    return (canID << 5) | 0x009;
}

bool ODriveS1Interface::sendVelocityCommand(float velocityRadPerSec) {
    uint32_t messageID = getVelocityCommandID();
    
    // Create CAN message using ACANFD library
    CANMessage message;
    message.id = messageID;
    message.len = 8;  // 8 bytes: velocity (4) + torque feedforward (4)
    
    // Prepare data payload:
    // Bytes 0-3: Input Velocity (float, little-endian)
    // Bytes 4-7: Torque Feedforward (float, little-endian, set to 0.0)
    union {
        float floatVal;
        uint8_t bytes[4];
    } velocity, torque;
    
    velocity.floatVal = velocityRadPerSec;
    torque.floatVal = 0.0;  // No torque feedforward
    
    // Copy to message data (little-endian format)
    message.data[0] = velocity.bytes[0];
    message.data[1] = velocity.bytes[1];
    message.data[2] = velocity.bytes[2];
    message.data[3] = velocity.bytes[3];
    message.data[4] = torque.bytes[0];
    message.data[5] = torque.bytes[1];
    message.data[6] = torque.bytes[2];
    message.data[7] = torque.bytes[3];
    
    // Send message via ACANFD library
    const bool ok = ACANFD_GIGA_R1::tryToSend(message);
    
    if (!ok) {
        Serial.println("Failed to send velocity command");
    }
    
    return ok;
}

bool ODriveS1Interface::requestStatus() {
    uint32_t messageID = getStatusRequestID();
    
    // Create CAN message for encoder estimates request
    CANMessage message;
    message.id = messageID;
    message.len = 0;  // No data payload for request
    
    // Send request message
    const bool ok = ACANFD_GIGA_R1::tryToSend(message);
    
    if (!ok) {
        Serial.println("Failed to send status request");
    }
    
    return ok;
}

bool ODriveS1Interface::readStatusResponse() {
    CANMessage receivedMessage;
    
    // Check for incoming messages
    if (ACANFD_GIGA_R1::receive(receivedMessage)) {
        // Check if message is encoder estimates from our ODrive
        uint32_t expectedID = getStatusResponseID();
        
        if (receivedMessage.id == expectedID && receivedMessage.len == 8) {
            // Parse encoder estimates from message data
            union {
                float floatVal;
                uint8_t bytes[4];
            } position, velocity;
            
            // Extract position (bytes 0-3, little-endian)
            position.bytes[0] = receivedMessage.data[0];
            position.bytes[1] = receivedMessage.data[1];
            position.bytes[2] = receivedMessage.data[2];
            position.bytes[3] = receivedMessage.data[3];
            
            // Extract velocity (bytes 4-7, little-endian)
            velocity.bytes[0] = receivedMessage.data[4];
            velocity.bytes[1] = receivedMessage.data[5];
            velocity.bytes[2] = receivedMessage.data[6];
            velocity.bytes[3] = receivedMessage.data[7];
            
            // Store values and convert velocity from rad/s to RPM
            currentPosition = position.floatVal;
            currentVelocity = velocity.floatVal * (60.0 / (2.0 * PI));
            
            lastStatusTime = millis();
            return true;
        }
    }
    
    return false;
}

void ODriveS1Interface::updateMotorStatus() {
    unsigned long currentTime = millis();
    bool communicationOK = (currentTime - lastStatusTime) < STATUS_TIMEOUT;
    
    // Check for reasonable velocity values (basic sanity check)
    bool velocityReasonable = (abs(currentVelocity) < 10000); // < 10,000 RPM
    
    // ODrive is ready if communication is working and values are reasonable
    motorReady = communicationOK && velocityReasonable;
    
    // Debug output for troubleshooting
    if (!communicationOK) {
        static unsigned long lastWarning = 0;
        if (currentTime - lastWarning > 5000) { // Warn every 5 seconds
            Serial.print("ODrive CAN ID 0x");
            Serial.print(canID, HEX);
            Serial.println(" communication timeout");
            lastWarning = currentTime;
        }
    }
}
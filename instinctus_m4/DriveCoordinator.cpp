/**
 * DriveCoordinator.cpp - Two-Motor Drive System Implementation
 * 
 * This file implements the DriveCoordinator class that manages left and right
 * motors for a balance robot. It provides coordinated control and monitoring
 * of both motors through a clean, high-level interface.
 * 
 * Key Implementation Details:
 * - Manages two MotorInterface pointers for left/right motors
 * - Provides synchronized operations (stop, status check)
 * - Simple pass-through for individual motor commands
 * - Fail-fast design with null pointer checks
 * 
 * Error Handling:
 * - Null pointer protection in constructor
 * - Both motors must be ready for system to be ready
 * - Emergency stop affects both motors regardless of individual status
 */

#include "DriveCoordinator.h"
#include <Arduino.h>

DriveCoordinator::DriveCoordinator(MotorInterface* left, MotorInterface* right) 
    : leftMotor(left), rightMotor(right) {
    // Constructor stores motor interface pointers
    // Actual initialization happens in initialize() method
}

bool DriveCoordinator::initialize() {
    if (!leftMotor || !rightMotor) {
        return false;
    }
    
    bool leftOK = leftMotor->initialize();
    bool rightOK = rightMotor->initialize();
    
    return leftOK && rightOK;
}

void DriveCoordinator::setMotorSpeeds(float leftRpm, float rightRpm) {
    if (leftMotor) {
        leftMotor->setVelocity(leftRpm);
    }
    
    if (rightMotor) {
        rightMotor->setVelocity(rightRpm);
    }
}

void DriveCoordinator::stop() {
    if (leftMotor) {
        leftMotor->stop();
    }
    
    if (rightMotor) {
        rightMotor->stop();
    }
}

bool DriveCoordinator::bothMotorsReady() {
    if (!leftMotor || !rightMotor) {
        return false;
    }
    
    return leftMotor->isReady() && rightMotor->isReady();
}

void DriveCoordinator::getMotorSpeeds(float& leftRpm, float& rightRpm) {
    leftRpm = leftMotor ? leftMotor->getVelocity() : 0.0;
    rightRpm = rightMotor ? rightMotor->getVelocity() : 0.0;
}

void DriveCoordinator::getMotorPositions(float& leftPos, float& rightPos) {
    leftPos = leftMotor ? leftMotor->getPosition() : 0.0;
    rightPos = rightMotor ? rightMotor->getPosition() : 0.0;
}
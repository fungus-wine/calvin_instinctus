/**
 * BalanceMotorController.cpp - Real-Time Motor Control Implementation
 *
 * This file implements immediate motor responses to balance events.
 * Unlike the event queue system, these callbacks execute synchronously
 * within the BalanceIMU::update() cycle, providing <1ms response time.
 *
 * Performance:
 * - onTiltChange() called at 100Hz (every 10ms) when tilt changes >1°
 * - onBalanceEmergency() called immediately when tilt >45° detected
 * - No event queue latency
 * - Direct hardware control
 *
 * Future Enhancement:
 * - Add PID control algorithm in onTiltChange()
 * - Implement tilt-to-velocity mapping
 * - Add deadband/filtering for stable control
 */

#include "BalanceMotorController.h"
#include <Arduino.h>

BalanceMotorController::BalanceMotorController(DriveCoordinator* driveSystem)
    : motors(driveSystem), emergencyStopActive(false) {
    // TODO: Initialize PID controller parameters
    // kp = 10.0;
    // ki = 0.5;
    // kd = 2.0;
    // integral = 0.0;
    // previousError = 0.0;
}

void BalanceMotorController::onTiltChange(float angle) {
    // Don't control motors if emergency stop is active
    if (emergencyStopActive) {
        return;
    }

    // TODO: Implement PID control algorithm here
    // Currently just a placeholder - motors stay stopped until PID implemented

    // Example PID implementation (commented out):
    /*
    float error = 0.0 - angle;  // Target is 0° (upright)
    float deltaTime = 0.01;     // 100Hz update rate

    // Proportional term
    float p = kp * error;

    // Integral term
    integral += error * deltaTime;
    float i = ki * integral;

    // Derivative term
    float derivative = (error - previousError) / deltaTime;
    float d = kd * derivative;

    // Calculate motor speed
    float motorSpeed = p + i + d;

    // Apply to motors
    motors->setMotorSpeeds(motorSpeed, motorSpeed);

    // Update state
    previousError = error;
    */

    // For now, just log the tilt (debugging only - remove in production)
    // Serial.print("Motor controller: Tilt = ");
    // Serial.println(angle);
}

void BalanceMotorController::onBalanceEmergency(float angle) {
    // IMMEDIATE MOTOR STOP - NO DELAY!
    motors->stop();
    emergencyStopActive = true;

    // Log to serial (temporary - remove in production)
    Serial.print("MOTOR EMERGENCY STOP: Tilt = ");
    Serial.print(angle);
    Serial.println("°");
}

void BalanceMotorController::resetEmergencyStop() {
    emergencyStopActive = false;
    Serial.println("Emergency stop reset - motor control re-enabled");
}

bool BalanceMotorController::isEmergencyStopped() const {
    return emergencyStopActive;
}

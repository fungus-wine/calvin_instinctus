#ifndef BALANCE_MOTOR_CONTROLLER_H
#define BALANCE_MOTOR_CONTROLLER_H

#include "BalanceObserver.h"
#include "DriveCoordinator.h"

/**
 * BalanceMotorController - Direct Motor Control Observer
 *
 * This observer provides REAL-TIME motor control responses to balance events,
 * bypassing the event queue system for immediate hardware response.
 *
 * Why This Exists:
 * - Event queues add latency (must wait for queue processing)
 * - Safety-critical actions need immediate response (<1ms)
 * - Motor control needs to run at same rate as balance updates (100Hz)
 *
 * Architecture:
 * - Called directly from BalanceIMU::update() (synchronous, no queue)
 * - Runs in same 100Hz loop as balance calculation
 * - Can implement PID control or direct tilt-to-velocity mapping
 *
 * Safety Features:
 * - Immediate emergency stop on dangerous tilt (>45°)
 * - No event queue latency
 * - Direct hardware control
 *
 * Usage:
 *   BalanceMotorController motorController(&driveSystem);
 *   BalanceEventObserver eventObserver;
 *
 *   balanceIMU.addObserver(&motorController);  // Real-time motor control
 *   balanceIMU.addObserver(&eventObserver);    // Status to M7
 *
 *   // Now BOTH observers get called on every tilt change:
 *   // - motorController adjusts motors immediately
 *   // - eventObserver sends status to M7 display
 */
class BalanceMotorController : public BalanceObserver {
private:
    DriveCoordinator* motors;

    // Emergency stop state
    bool emergencyStopActive;

    // TODO: Add PID controller variables here
    // float kp, ki, kd;
    // float integral, previousError;

public:
    /**
     * Constructor
     * @param driveSystem - pointer to motor control system
     */
    BalanceMotorController(DriveCoordinator* driveSystem);

    /**
     * Called on tilt changes >1°
     * Implements balance control algorithm (currently placeholder for PID)
     */
    void onTiltChange(float angle) override;

    /**
     * Called on dangerous tilt >45°
     * Immediately stops motors for safety
     */
    void onBalanceEmergency(float angle) override;

    /**
     * Reset emergency stop state
     * Call this to re-enable motor control after emergency
     */
    void resetEmergencyStop();

    /**
     * Check if in emergency stop state
     */
    bool isEmergencyStopped() const;
};

#endif // BALANCE_MOTOR_CONTROLLER_H

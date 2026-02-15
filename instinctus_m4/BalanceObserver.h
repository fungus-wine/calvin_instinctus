#ifndef BALANCE_OBSERVER_H
#define BALANCE_OBSERVER_H

/**
 * BalanceObserver.h - Observer Interface for Balance Control Events
 * 
 * This file defines the observer interface for balance-specific events from the
 * BalanceIMU system. Unlike generic sensor observers, this interface is focused
 * specifically on balance control concepts: tilt angles and emergency conditions.
 * 
 * Design Philosophy:
 * - Balance-focused events, not raw sensor data
 * - High-level concepts (tilt, emergency) rather than low-level readings
 * - Minimal interface - only essential callbacks
 * - Real-time friendly - callbacks designed for fast execution
 * 
 * Event Types:
 * - onTiltChange: Called when tilt angle changes significantly (>1°)
 * - onBalanceEmergency: Called when robot tilts dangerously (>45°)
 * 
 * Usage Pattern:
 * 1. Create a class that implements BalanceObserver
 * 2. Implement the callback methods for your specific needs
 * 3. Pass observer to BalanceIMU constructor
 * 4. BalanceIMU will call your methods automatically during update()
 * 
 * Example:
 *   class MyBalanceController : public BalanceObserver {
 *       void onTiltChange(float angle) override {
 *           // Adjust motor output based on tilt
 *       }
 *       void onBalanceEmergency(float angle) override {
 *           // Stop motors, send alert
 *       }
 *   };
 * 
 * Performance Notes:
 * - Callbacks are called from BalanceIMU::update() (typically 100Hz)
 * - Keep callback implementations fast and non-blocking
 * - Avoid Serial.print() in production code (use for debugging only)
 */
class BalanceObserver {
public:
    /**
     * Called when tilt angle changes significantly
     * @param angle - current tilt angle in degrees (-90 to +90, 0 = upright)
     */
    virtual void onTiltChange(float angle) = 0;
    
    /**
     * Called when robot enters emergency tilt condition
     * @param angle - critical tilt angle that triggered emergency
     */
    virtual void onBalanceEmergency(float angle) = 0;
    
    virtual ~BalanceObserver() = default;
};

#endif // BALANCE_OBSERVER_H
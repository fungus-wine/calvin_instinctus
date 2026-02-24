#ifndef BALANCE_EVENT_OBSERVER_H
#define BALANCE_EVENT_OBSERVER_H

#include "BalanceObserver.h"
#include <InstinctusKit.h>

/**
 * BalanceEventObserver.h - Bridge Between Balance Events and Inter-Core Communication
 * 
 * This class implements the BalanceObserver interface and acts as a bridge between
 * the balance IMU system and the dual-core event system. It receives balance events
 * from BalanceIMU and converts them into inter-core events that can be sent to the
 * M7 core or broadcast to both cores.
 * 
 * Purpose:
 * - Convert balance-specific events into generic inter-core events
 * - Route tilt updates to M7 core for display/logging
 * - Broadcast emergency conditions to both cores for safety
 * 
 * Event Routing:
 * - onTiltChange() → EVENT_BALANCE_STATUS to M7 core (display, logging)
 * - onBalanceEmergency() → EVENT_EMERGENCY_STOP broadcast to both cores (safety shutdown)
 * 
 * Usage:
 *   BalanceEventObserver eventObserver;
 *   balanceIMU.addObserver(&eventObserver);
 *   // Now balance events automatically get sent to inter-core queues
 */
class BalanceEventObserver : public BalanceObserver {
public:
    /**
     * Called when tilt angle changes significantly
     * Sends tilt data to M7 core for display/logging
     */
    void onTiltChange(float angle) override;
    
    /**
     * Called when robot enters emergency tilt condition
     * Broadcasts emergency to both cores for immediate safety response
     */
    void onBalanceEmergency(float angle) override;
};

#endif // BALANCE_EVENT_OBSERVER_H
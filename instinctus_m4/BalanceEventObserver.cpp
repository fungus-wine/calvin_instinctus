/**
 * BalanceEventObserver.cpp - Balance to Inter-Core Event Bridge Implementation
 * 
 * This file implements the bridge between balance events and the dual-core event
 * system. It converts balance-specific events (tilt changes, emergencies) into
 * generic inter-core events that can be processed by the M7 core.
 * 
 * Event Types Used:
 * - EVENT_BALANCE_IMU_DATA: Regular tilt angle updates sent to M7
 * - EVENT_EMERGENCY_STOP: Critical tilt conditions broadcast to both cores
 * 
 * Implementation Notes:
 * - Uses EventBroadcaster for clean event routing
 * - sendToM7() for non-critical updates (display, logging)
 * - broadcastEvent() for critical safety events (both cores respond)
 * - Events include tilt angle as float data payload
 */

#include "BalanceEventObserver.h"

void BalanceEventObserver::onTiltChange(float angle) {
    // Convert float angle to string for event data
    char angleStr[16];
    sprintf(angleStr, "%.2f", angle);
    
    // Send balance status to M7 core for display/logging
    EventBroadcaster::sendToM7(EVENT_BALANCE_IMU_DATA, angleStr);
}

void BalanceEventObserver::onBalanceEmergency(float angle) {
    // Convert float angle to string for event data
    char angleStr[16];
    sprintf(angleStr, "%.2f", angle);
    
    // Broadcast emergency stop to both cores for immediate safety response
    EventBroadcaster::broadcastEvent(EVENT_EMERGENCY_STOP, angleStr);
}
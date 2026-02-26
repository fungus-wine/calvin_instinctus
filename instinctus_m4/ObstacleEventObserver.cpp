/**
 * ObstacleEventObserver.cpp - Obstacle to Inter-Core Event Bridge Implementation
 *
 * Converts obstacle proximity events into inter-core events
 * that can be processed by the M7 core.
 *
 * Event Types Used:
 * - EVENT_PROXIMITY_WARNING: Obstacle detected, sent to M7
 */

#include "ObstacleEventObserver.h"

ObstacleEventObserver::ObstacleEventObserver(const char* sensorId, float thresholdMm)
    : _sensorId(sensorId), _thresholdMm(thresholdMm) {
}

float ObstacleEventObserver::getThreshold() const {
    return _thresholdMm;
}

void ObstacleEventObserver::onObstacleDetection(float distance) {
    char msg[32];
    snprintf(msg, sizeof(msg), "%s,%.0f", _sensorId, distance);
    EventBroadcaster::sendToM7(EVENT_PROXIMITY_WARNING, msg);
}

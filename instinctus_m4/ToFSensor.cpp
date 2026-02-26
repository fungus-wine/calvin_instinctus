/**
 * ToFSensor.cpp - ToF Distance Sensor System Implementation
 *
 * Reads distance data via ToFInterface and notifies ObstacleObservers.
 * Non-blocking: update() returns immediately if no new data is available.
 */

#include "ToFSensor.h"
#include "ObstacleObserver.h"


ToFSensor::ToFSensor(ToFInterface* tofHardware)
    : _tof(tofHardware), _observer(nullptr),
      _currentDistance(-1.0f), _initialized(false) {
}

void ToFSensor::setObserver(ObstacleObserver* observer) {
    _observer = observer;
}

bool ToFSensor::initialize() {
    if (!_tof) {
        return false;
    }
    if (!_tof->initialize()) {
        return false;
    }
    if (!_tof->startRanging()) {
        return false;
    }
    _initialized = true;
    return true;
}

void ToFSensor::update() {
    if (!_initialized) {
        return;
    }

    float distance;
    if (!_tof->readDistance(distance)) {
        return; // No new data available, skip this cycle
    }

    _currentDistance = distance;

    // Notify observer if obstacle detected
    if (_observer && _currentDistance < _observer->getThreshold()) {
        _observer->onObstacleDetection(_currentDistance);
    }
}

float ToFSensor::getDistance() const {
    return _currentDistance;
}

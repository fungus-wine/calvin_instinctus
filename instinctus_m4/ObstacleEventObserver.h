#ifndef OBSTACLE_EVENT_OBSERVER_H
#define OBSTACLE_EVENT_OBSERVER_H

#include "ObstacleObserver.h"
#include <InstinctusKit.h>

/**
 * ObstacleEventObserver.h - Bridge Between Obstacle Events and Inter-Core Communication
 *
 * Implements ObstacleObserver and forwards proximity events
 * to the M7 core via the EventQueue system.
 *
 * Event Routing:
 * - onObstacleDetection() -> EVENT_PROXIMITY_WARNING to M7 core
 */
class ObstacleEventObserver : public ObstacleObserver {
private:
    const char* _sensorId;
    float _thresholdMm;

public:
    /**
     * Constructor
     * @param sensorId - identifier included in event messages (e.g., "front", "rear")
     * @param thresholdMm - distance in mm below which obstacle is detected
     */
    ObstacleEventObserver(const char* sensorId, float thresholdMm);

    void onObstacleDetection(float distance) override;
    float getThreshold() const override;
};

#endif // OBSTACLE_EVENT_OBSERVER_H

#ifndef OBSTACLE_OBSERVER_H
#define OBSTACLE_OBSERVER_H

/**
 * ObstacleObserver.h - Observer Interface for Obstacle Proximity Events
 *
 * Notified by ToF sensors when a nearby obstacle is detected.
 * Separated from CollisionObserver (IMU-based impact detection) because
 * proximity and impact come from different sensors and mean different things.
 *
 * Callbacks are called from ToFSensor::update() — keep implementations
 * fast and non-blocking.
 */
class ObstacleObserver {
public:
    /**
     * Called when obstacle is detected nearby (e.g. by ToF sensor)
     * @param distance - distance to obstacle in mm
     */
    virtual void onObstacleDetection(float distance) = 0;

    /**
     * Threshold distance in mm below which an obstacle is considered detected
     */
    virtual float getThreshold() const = 0;

    virtual ~ObstacleObserver() = default;
};

#endif // OBSTACLE_OBSERVER_H

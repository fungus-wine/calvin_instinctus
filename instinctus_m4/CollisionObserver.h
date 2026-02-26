#ifndef COLLISION_OBSERVER_H
#define COLLISION_OBSERVER_H

/**
 * CollisionObserver.h - Observer Interface for Collision Impact Events
 *
 * Notified by the IMU when a physical collision (impact) is detected
 * via accelerometer spike. Separated from ObstacleObserver (ToF-based
 * proximity detection) because impact and proximity come from different
 * sensors and mean different things.
 *
 * Not yet wired up — reserved for future IMU-based collision detection.
 */
class CollisionObserver {
public:
    /**
     * Called when robot detects actual collision (e.g. via accelerometer)
     */
    virtual void onCollision() = 0;

    virtual ~CollisionObserver() = default;
};

#endif // COLLISION_OBSERVER_H

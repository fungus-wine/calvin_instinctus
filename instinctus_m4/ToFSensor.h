#ifndef TOF_SENSOR_H
#define TOF_SENSOR_H

#include <Arduino.h>
#include "ToFInterface.h"

// Forward declaration
class ObstacleObserver;

/**
 * ToFSensor.h - ToF Distance Sensor System
 *
 * Processes ToF sensor data and notifies observers of obstacle proximity events.
 * Mirrors the BalanceIMU pattern: hardware abstraction via ToFInterface,
 * observer pattern for event notifications.
 *
 * Key Features:
 * - Hardware abstraction via ToFInterface (works with any ToF chip)
 * - Observer pattern for obstacle detection events
 * - Non-blocking update cycle safe for M4 real-time loop
 * - Configurable proximity threshold
 *
 * Usage:
 *   VL53L4CXInterface tofHardware(&Wire, -1, 0x29);
 *   ToFSensor sensor(&tofHardware);
 *   sensor.addObserver(&myObstacleObserver);
 *   sensor.initialize();
 *   while (true) {
 *       sensor.update();  // Non-blocking
 *   }
 */
class ToFSensor {
private:
    ToFInterface* _tof;
    ObstacleObserver* _observer;

    float _currentDistance;   // Last valid distance in mm
    bool _initialized;

public:
    /**
     * Constructor
     * @param tofHardware - pointer to ToF hardware implementation
     */
    ToFSensor(ToFInterface* tofHardware);

    /**
     * Set the obstacle observer to receive proximity events
     * @param observer - pointer to observer (cannot be null)
     */
    void setObserver(ObstacleObserver* observer);

    /**
     * Initialize the ToF sensor and start ranging
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * Non-blocking update: reads sensor if new data is available.
     * Notifies observers on obstacle detection.
     */
    void update();

    /**
     * Get last valid distance reading in mm
     * @return distance in mm, or -1.0 if no valid reading yet
     */
    float getDistance() const;
};

#endif // TOF_SENSOR_H

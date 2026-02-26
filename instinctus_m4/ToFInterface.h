#ifndef TOF_INTERFACE_H
#define TOF_INTERFACE_H

/**
 * ToFInterface.h - Hardware Abstraction Layer for ToF Sensors
 *
 * Defines a clean interface that any ToF hardware can implement.
 * Focused on the essential functions needed for collision avoidance:
 * initialization, starting measurements, and non-blocking distance reads.
 *
 * Design Goals:
 * - Hardware agnostic - works with any ToF chip (VL53L4CX, VL53L1X, etc.)
 * - Non-blocking - safe for M4 real-time loop
 * - Easy to swap - new ToF chips require minimal code changes
 *
 * Usage:
 * 1. Create a concrete implementation (e.g., VL53L4CXInterface)
 * 2. Implement initialize(), startRanging(), and readDistance() methods
 * 3. Pass to ToFSensor constructor for dependency injection
 */
class ToFInterface {
public:
    /**
     * Initialize the sensor hardware and configure settings.
     * @return true if initialization successful, false otherwise
     */
    virtual bool initialize() = 0;

    /**
     * Start continuous ranging measurements.
     * @return true if ranging started successfully
     */
    virtual bool startRanging() = 0;

    /**
     * Non-blocking distance read.
     * @param distance - distance in mm (only valid when return is true)
     * @return true if new data was available and read, false if no new data yet
     */
    virtual bool readDistance(float& distance) = 0;

    virtual ~ToFInterface() = default;
};

#endif // TOF_INTERFACE_H

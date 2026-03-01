#ifndef VL53L4CX_INTERFACE_H
#define VL53L4CX_INTERFACE_H

#include <vl53l4cx_class.h>
#include <Wire.h>
#include "ToFInterface.h"

/**
 * VL53L4CXInterface.h - VL53L4CX ToF Sensor Implementation
 *
 * Implements ToFInterface for the STM32duino VL53L4CX multi-zone ToF sensor.
 * Provides non-blocking distance reads safe for the M4 real-time loop.
 *
 * Hardware Details:
 * - Communication: I2C (Wire library)
 * - Library: STM32duino VL53L4CX
 * - Default I2C address: 0x29
 * - Both sensors share 0x29 so they must be on separate I2C buses
 *
 * Usage:
 *   VL53L4CXInterface tof(&Wire, -1, 0x29);
 *   if (tof.initialize()) {
 *       tof.startRanging();
 *       float distance;
 *       if (tof.readDistance(distance)) {
 *           // Use distance in mm
 *       }
 *   }
 */
class VL53L4CXInterface : public ToFInterface {
private:
    VL53L4CX _tof;
    uint8_t _i2cAddress;
    uint32_t _timingBudgetUs;

public:
    /**
     * Constructor
     * @param i2cBus - pointer to I2C bus (e.g., &Wire, &Wire1)
     * @param xshutPin - XSHUT pin for power control (-1 if not connected)
     * @param address - I2C address (default 0x29)
     */
    VL53L4CXInterface(TwoWire* i2cBus, int xshutPin, uint8_t address, uint32_t timingBudgetUs = 33000);

    bool initialize() override;
    bool startRanging() override;
    bool readDistance(float& distance) override;
};

#endif // VL53L4CX_INTERFACE_H

/**
 * VL53L4CXInterface.cpp - VL53L4CX ToF Sensor Implementation
 *
 * Concrete implementation of ToFInterface for the VL53L4CX sensor.
 * Uses the STM32duino VL53L4CX library for hardware communication.
 *
 * Non-blocking design:
 * - readDistance() polls VL53L4CX_GetMeasurementDataReady() once per call
 * - Returns false immediately if no new data is available
 * - Never blocks the M4 real-time loop
 *
 * Distance mode is set to SHORT for fast, close-range collision detection.
 */

#include "VL53L4CXInterface.h"
#include <InstinctusKit.h>

VL53L4CXInterface::VL53L4CXInterface(TwoWire* i2cBus, int xshutPin, uint8_t address, uint32_t timingBudgetUs)
    : _tof(i2cBus, xshutPin), _i2cAddress(address), _timingBudgetUs(timingBudgetUs) {
}

bool VL53L4CXInterface::initialize() {
    _tof.begin();

    if (_tof.InitSensor(_i2cAddress) != VL53L4CX_ERROR_NONE) {
        return false;
    }

    // Short distance mode for fast collision detection
    _tof.VL53L4CX_SetDistanceMode(VL53L4CX_DISTANCEMODE_SHORT);
    _tof.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(_timingBudgetUs);

    return true;
}

bool VL53L4CXInterface::startRanging() {
    return _tof.VL53L4CX_StartMeasurement() == VL53L4CX_ERROR_NONE;
}

bool VL53L4CXInterface::readDistance(float& distance) {
    // Non-blocking: check if data is ready, return false if not
    uint8_t dataReady = 0;
    if (_tof.VL53L4CX_GetMeasurementDataReady(&dataReady) != VL53L4CX_ERROR_NONE) {
        return false;
    }
    if (!dataReady) {
        return false;
    }

    VL53L4CX_MultiRangingData_t rangingData;
    if (_tof.VL53L4CX_GetMultiRangingData(&rangingData) != VL53L4CX_ERROR_NONE) {
        _tof.VL53L4CX_ClearInterruptAndStartMeasurement();
        return false;
    }

    // Use the closest valid target
    bool found = false;
    float closest = Config::NO_TARGET_DISTANCE;
    for (int i = 0; i < rangingData.NumberOfObjectsFound; i++) {
        if (rangingData.RangeData[i].RangeStatus == VL53L4CX_RANGESTATUS_RANGE_VALID ||
            rangingData.RangeData[i].RangeStatus == VL53L4CX_RANGESTATUS_RANGE_VALID_MIN_RANGE_CLIPPED) {
            float d = (float)rangingData.RangeData[i].RangeMilliMeter;
            if (d < closest) {
                closest = d;
                found = true;
            }
        }
    }

    _tof.VL53L4CX_ClearInterruptAndStartMeasurement();

    if (found) {
        distance = closest;
        return true;
    }

    return false;
}

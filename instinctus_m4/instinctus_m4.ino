/**
 * Load order: flash M7 first (it initializes shared queues), then M4.
 */

#include <InstinctusKit.h>
#include <RPC.h>
#include "ICM20948Interface.h"
#include "BalanceIMU.h"
#include "BalanceEventObserver.h"
#include "VL53L4CXInterface.h"
#include "ToFSensor.h"
#include "ObstacleEventObserver.h"

// Timing
unsigned long lastIMUData = 0;
unsigned long lastToFData = 0;

// IMU: ICM20948 on Wire
ICM20948Interface imuHardware(&Wire, Config::IMU_I2C_ADDRESS);
BalanceIMU balanceIMU(&imuHardware);
BalanceEventObserver balanceEventObserver;

// ToF: Both VL53L4CX on Wire (I2C0), differentiated by XSHUT pins.
// On boot, both are shut down, then brought up one at a time to assign
// unique addresses: rear gets 0x30, front keeps default 0x29.
VL53L4CXInterface rearToFHardware(&Wire, Config::TOF_REAR.xshutPin, Config::TOF_REAR.i2cAddress, Config::TOF_REAR.timingBudgetUs);
ToFSensor rearToF(&rearToFHardware);
ObstacleEventObserver rearObstacleObserver("rear", Config::TOF_REAR.warnDistanceMm);

VL53L4CXInterface frontToFHardware(&Wire, Config::TOF_FRONT.xshutPin, Config::TOF_FRONT.i2cAddress, Config::TOF_FRONT.timingBudgetUs);
ToFSensor frontToF(&frontToFHardware);
ObstacleEventObserver frontObstacleObserver("front", Config::TOF_FRONT.warnDistanceMm);

float ax, ay, az;
float gx, gy, gz;
float tiltAngle;
float frontDistance;
float rearDistance;

// Giga built-in LEDs are active-low
static void blinkLED(int pin, int durationMs = 50) {
    digitalWrite(pin, LOW);
    delay(durationMs);
    digitalWrite(pin, HIGH);
}

void setup() {
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);

    RPC.begin();
    // Spin until M7 signals that queues are initialized and serial bridge is ready.
    // M7 pushes EVENT_SYSTEM_STARTUP after completing its own setup(), so by the
    // time we receive it, Serial (bridged via RPC) is guaranteed to be usable.
    EventType startType;
    while (!m4EventQueue.pop(startType, nullptr) || startType != EVENT_SYSTEM_STARTUP);

    balanceIMU.setObserver(&balanceEventObserver);
    balanceIMU.initialize();

    // Shut down both ToF sensors before initializing either one.
    // This ensures a clean state and allows sequential address assignment.
    pinMode(Config::TOF_REAR.xshutPin, OUTPUT);
    pinMode(Config::TOF_FRONT.xshutPin, OUTPUT);
    digitalWrite(Config::TOF_REAR.xshutPin, LOW);
    digitalWrite(Config::TOF_FRONT.xshutPin, LOW);

    // Initialize ToF sensors one at a time — each InitSensor() brings up
    // its sensor via XSHUT and assigns its address while the other stays off.
    // Rear first (reprogrammed to 0x30), then front (keeps default 0x29).
    rearToF.setObserver(&rearObstacleObserver);
    if (!rearToF.initialize()) {
        EventBroadcaster::sendToM7(EVENT_SYSTEM_STARTUP, "Rear ToF init failed");
    }

    frontToF.setObserver(&frontObstacleObserver);
    if (!frontToF.initialize()) {
        EventBroadcaster::sendToM7(EVENT_SYSTEM_STARTUP, "Front ToF init failed");
    }

    lastIMUData = millis();
    lastToFData = millis();
    EventBroadcaster::sendToM7(EVENT_SYSTEM_STARTUP, "M4 Initialized");
    //visual cue that M4 is initialized
    blinkLED(LEDB);delay(50);blinkLED(LEDB);delay(50);blinkLED(LEDB);delay(1000);
}

void loop() {
    balanceIMU.update();
    frontToF.update();
    rearToF.update();
    unsigned long now = millis();

    if (now - lastIMUData >= Config::IMU_DATA_INTERVAL_MS) {
        tiltAngle = balanceIMU.getTiltAngle();
        balanceIMU.getAcceleration(ax, ay, az);
        balanceIMU.getAngularVelocity(gx, gy, gz);

        char msg[EVENT_MESSAGE_SIZE];
        snprintf(msg, sizeof(msg), "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
           ax, ay, az, gx, gy, gz, tiltAngle);

        bool ok = EventBroadcaster::sendToM7(EVENT_BALANCE_IMU_DATA, msg);
        if (!ok) {
            blinkLED(LEDR);
        }

        lastIMUData = now;
    }

    if (now - lastToFData >= Config::TOF_DATA_INTERVAL_MS) {
        frontDistance = frontToF.getDistance();
        rearDistance = rearToF.getDistance();

        char msg[EVENT_MESSAGE_SIZE];
        snprintf(msg, sizeof(msg), "%.0f,%.0f", frontDistance, rearDistance);

        bool ok = EventBroadcaster::sendToM7(EVENT_TOF_DATA, msg);
        if (!ok) {
            blinkLED(LEDR);
        }

        lastToFData = now;
    }

    // Drain m4EventQueue — process commands from M7
    EventType eventType;
    char eventData[EVENT_MESSAGE_SIZE];

    while (m4EventQueue.pop(eventType, eventData)) {
        // TODO: handle M7 commands
    }

    delay(Config::M4_LOOP_DELAY_MS);
}

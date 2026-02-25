/**
 * test_minimal_m4.ino - Minimal Intercore Queue Test (M4 Side)
 *
 * Load order: flash M7 first (it initializes shared queues), then M4.
 * Monitor Serial on the M7 side to observe bidirectional traffic.
 */

#include <InstinctusKit.h>
#include <RPC.h>
#include "ICM20948Interface.h"
#include "BalanceIMU.h"
#include "BalanceEventObserver.h"

// Timing
unsigned long lastIMUData = 0;
const unsigned long IMU_DATA_INTERVAL = 500;

// Create IMU hardware interface
ICM20948Interface imuHardware;
BalanceIMU balanceIMU(&imuHardware);
BalanceEventObserver balanceEventObserver;

float ax;
float ay;
float az; 
float gx;
float gy;
float gz;
float tiltAngle;

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

    balanceIMU.addObserver(&balanceEventObserver);
    balanceIMU.initialize();

    lastIMUData = millis();
    EventBroadcaster::sendToM7(EVENT_SYSTEM_STARTUP, "M4 Initialized");
    //visual cue that M4 is initialized
    blinkLED(LEDB);delay(50);blinkLED(LEDB);delay(50);blinkLED(LEDB);delay(1000);
}

void loop() {
    balanceIMU.update();
    unsigned long now = millis();
    
    // Send periodic sensor data event to M7
    if (now - lastIMUData >= IMU_DATA_INTERVAL) {

        // get IMU Data
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

    // Drain m4EventQueue — process commands from M7
    EventType eventType;
    char eventData[EVENT_MESSAGE_SIZE];

    while (m4EventQueue.pop(eventType, eventData)) {
    // blinkLED(LEDR);delay(50);blinkLED(LEDR);delay(50);blinkLED(LEDR);
        // switch (eventType) {
        //     case EVENT_SYSTEM_STARTUP:  blinkLED(LEDR); break;
        //     case EVENT_EMERGENCY_STOP:  blinkLED(LEDG); break;
        //     default:
        //         blinkLED(LEDB);
        //         break;
        // }
    }


    // EventType eventType;
    // char eventData[EVENT_MESSAGE_SIZE];

    // while (m4EventQueue.pop(eventType, eventData)) {
    //     blinkLED(LEDB);  // Blue  = received from M7

    //     // Echo back so M7 can confirm round-trip
    //     char reply[EVENT_MESSAGE_SIZE];
    //     snprintf(reply, sizeof(reply), "ack:%s", eventData);
    //     EventBroadcaster::sendToM7(EVENT_SYSTEM_HEALTH, reply);
    // }
    // delay(System::MAIN_LOOP_DELAY);
    delay(10);
}
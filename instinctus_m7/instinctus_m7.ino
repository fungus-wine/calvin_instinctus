
/**
 * test_minimal_m7.ino - Minimal Intercore Queue Test (M7 Side)
 *
 * Tests bidirectional intercore communication via InstinctusKit EventQueue:
 *   - Initializes shared queues (M7 must run first)
 *   - Sends a ping command to M4 every 2 seconds
 *   - Prints every event received from M4 to Serial
 *
 * Load order: flash THIS sketch first, then flash test_minimal_m4.
 * Monitor Serial here at 115200 baud.
 *
 * Expected output:
 *   [M7] Queues initialized
 *   [M7] TX ping:0 to M4 ... OK
 *   [M7] RX SYSTEM_STARTUP  : m4_ready
 *   [M7] RX BALANCE_STATUS  : count:0
 *   [M7] RX SYSTEM_HEALTH   : ack:ping:0
 *   [M7] RX BALANCE_STATUS  : count:1
 *   ...
 *   [M7] Stats  rx=4  m4Q=0  m7Q=0
 */

#include <InstinctusKit.h>
#include <RPC.h>

// Timing
unsigned long lastPingMs  = 0;
unsigned long lastStatsMs = 0;
const unsigned long PING_INTERVAL  = 2000;
const unsigned long STATS_INTERVAL = 5000;

uint32_t pingCount = 0;
uint32_t rxCount   = 0;

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

    // M7 owns queue initialization — must happen before M4 tries to use them
    RPC.begin(); //boot M4
    m4EventQueue.initialize(HSEM_ID_M4_QUEUE);
    m7EventQueue.initialize(HSEM_ID_M7_QUEUE);

    Serial.begin(115200);
    delay(150);  // Brief pause for USB CDC to enumerate; Don't use while(!Serial) - casues M4 Problems
    
    // initialize dsiplay here

    lastPingMs  = millis();
    lastStatsMs = millis();

    EventBroadcaster::broadcastEvent(EVENT_SYSTEM_STARTUP, "M7 Initialized");
    blinkLED(LEDR);delay(50);blinkLED(LEDG);delay(50);blinkLED(LEDB);delay(1000);
}

void loop() {
    // blinkLED(LEDR);delay(50);blinkLED(LEDG);delay(50);blinkLED(LEDB);delay(1000);
    unsigned long now = millis();

    // 1. Send periodic ping command to M4
    if (now - lastPingMs >= PING_INTERVAL) {
        char msg[32];
        snprintf(msg, sizeof(msg), "ping:%lu", pingCount);

        bool ok = EventBroadcaster::sendToM4(EVENT_EMERGENCY_STOP, msg);
        Serial.print("M7 TX ");
        Serial.print(msg);
        Serial.print(" to M4 ... ");
        Serial.println(ok ? "OK" : "QUEUE FULL");

        pingCount++;
        lastPingMs = now;
    }

    // 2. Drain m7EventQueue — print everything M4 sent
    EventType eventType;
    char eventData[EVENT_MESSAGE_SIZE];

    while (m7EventQueue.pop(eventType, eventData)) {

        Serial.print("[M7] RX ");
        switch (eventType) {
            case EVENT_SYSTEM_STARTUP:   Serial.print("SYSTEM_STARTUP "); break;
            case EVENT_BALANCE_IMU_DATA: Serial.print("BALANCE_DATA   "); break;
            // case EVENT_TOF_FRONT_DATA:   Serial.print("TOF_FRONT_DATA "); break;
            // case EVENT_TOF_REAR_DATA:    Serial.print("TOF_REAR_DATA  "); break;
            // case EVENT_EMERGENCY_STOP:   Serial.print("EMERGENCY_STOP "); break;
            default:
                Serial.print("type=");
                Serial.print((int)eventType);
                Serial.print("         ");
                break;
        }
        Serial.print(": ");
        Serial.println(eventData);
    }

    // 3. Periodic stats
    if (now - lastStatsMs >= STATS_INTERVAL) {
        Serial.print("M7 BX M4Q: ");
        Serial.print(EventBroadcaster::getM4QueueCount());
        Serial.print(" M7Q: ");
        Serial.println(EventBroadcaster::getM7QueueCount());
        lastStatsMs = now;
    }
    delay(10);
}

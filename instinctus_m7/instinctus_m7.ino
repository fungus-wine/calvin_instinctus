
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
unsigned long lastQueueCount = 0;
const unsigned long QUEUE_COUNT_INTERVAL = 5000;

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
    m4EventQueue.initialize(HSEM_ID_M4_QUEUE);
    m7EventQueue.initialize(HSEM_ID_M7_QUEUE);
    RPC.begin(); //boot M4

    Serial.begin(115200);
    delay(150);  // Brief pause for USB CDC to enumerate; Don't use while(!Serial) - casues M4 Problems
    
    // initialize dsiplay here

    lastQueueCount = millis();

    EventBroadcaster::broadcastEvent(EVENT_SYSTEM_STARTUP, "M7 Initialized");
    blinkLED(LEDR);delay(50);blinkLED(LEDG);delay(50);blinkLED(LEDB);delay(1000);
}

void loop() {
    unsigned long now = millis();

    // Drain m7EventQueue
    EventType eventType;
    char eventData[EVENT_MESSAGE_SIZE];

    while (m7EventQueue.pop(eventType, eventData)) {
        switch (eventType) {
            case EVENT_SYSTEM_STARTUP:   
                Serial.print("SYSTEM_STARTUP");
                break;
            case EVENT_BALANCE_IMU_DATA: 
                Serial.print("BALANCE_DATA");
                break;
            // case EVENT_TOF_FRONT_DATA:   Serial.print("TOF_FRONT_DATA "); break;
            // case EVENT_TOF_REAR_DATA:    Serial.print("TOF_REAR_DATA  "); break;
            // case EVENT_EMERGENCY_STOP:   Serial.print("EMERGENCY_STOP "); break;
            default:
                Serial.print("type=");
                Serial.print((int)eventType);;
                break;
        }
        Serial.print(": ");
        Serial.println(eventData);
    }

    // 3. Periodic stats
    if (now - lastQueueCount >= QUEUE_COUNT_INTERVAL) {
        Serial.print("M4Q: ");
        Serial.print(EventBroadcaster::getM4QueueCount());
        Serial.print(" M7Q: ");
        Serial.println(EventBroadcaster::getM7QueueCount());
        lastQueueCount = now;
    }
    delay(10);
}

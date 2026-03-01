#include <InstinctusKit.h>
#include <RPC.h>

// Timing
unsigned long lastQueueCount = 0;

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

    Serial.begin(Config::SERIAL_BAUD_RATE);
    delay(Config::USB_ENUM_DELAY_MS);  // Brief pause for USB CDC to enumerate; Don't use while(!Serial) - causes M4 problems

    lastQueueCount = millis();

    EventBroadcaster::broadcastEvent(EVENT_SYSTEM_STARTUP, "M7 Initialized");
    blinkLED(LEDR);delay(50);blinkLED(LEDG);delay(50);blinkLED(LEDB);delay(1000);
}

void loop() {
    unsigned long now = millis();

    // handle events
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
            case EVENT_TOF_DATA: 
                Serial.print("TOF_DATA");
                break;
            case EVENT_PROXIMITY_WARNING: 
                Serial.print("PROXIMITY_WARNING");
                break;
            case EVENT_EMERGENCY_STOP: 
                Serial.print("EMERGENY_STOP");
                break;
            default:
                Serial.print("event type=");
                Serial.print((int)eventType);;
                break;
        }
        Serial.print(": ");
        Serial.println(eventData);
    }

    // queue status reprort
    if (now - lastQueueCount >= Config::QUEUE_STATUS_INTERVAL_MS) {
        Serial.print("M4Q: ");
        Serial.print(EventBroadcaster::getM4QueueCount());
        Serial.print(" M7Q: ");
        Serial.println(EventBroadcaster::getM7QueueCount());
        lastQueueCount = now;
    }
    delay(10);
}

/**
 * balancer_m7.ino - M7 Core Main Program
 *
 * Cortex-M7 Core Responsibilities:
 * - Display management (GIGA R1 WiFi display)
 * - Process status events from M4 core
 * - Future: Jetson communication, navigation IMU
 *
 * Current Mode: Hardware Validation Display
 * - Receives IMU data from M4
 * - Receives collision warnings from M4
 * - Displays real-time sensor status
 */

#include <BalancerKit.h>
#include "TerminalDisplay.h"

// Display instance
TerminalDisplay display;

// Timing variables
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 100;  // 10Hz display refresh

// Status tracking
unsigned long eventsReceived = 0;
unsigned long lastEventTime = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(100);

  Serial.println("===========================================");
  Serial.println("  Balancer M7 Core - Display & Monitoring");
  Serial.println("===========================================");
  Serial.println();

  // Initialize display
  Serial.print("Initializing display... ");
  display.initialize();
  display.println("=== M7 CORE STARTING ===", TerminalDisplay::GREEN_COLOR);
  Serial.println("OK");

  // Initialize dual event queues (shared between M4 and M7 cores)
  Serial.print("Initializing event queues... ");
  m4EventQueue.initialize();    // M4 processes this queue
  m7EventQueue.initialize();    // M7 processes this queue
  display.println("Event queues initialized", TerminalDisplay::GREEN_COLOR);
  Serial.println("OK");

  // Send startup broadcast event
  Serial.print("Broadcasting startup event... ");
  EventBroadcaster::broadcastEvent(EVENT_SYSTEM_STARTUP, "M7 ready");
  display.println("Startup event sent", TerminalDisplay::GREEN_COLOR);
  Serial.println("OK");

  Serial.println();
  Serial.println("M7 Core Initialized");
  Serial.println("Waiting for M4 events...");
  Serial.println();

  display.println("", TerminalDisplay::TEXT_COLOR);
  display.println("=== WAITING FOR M4 DATA ===", TerminalDisplay::YELLOW_COLOR);
  display.println("", TerminalDisplay::TEXT_COLOR);

  lastEventTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // Process M7 events (from M4, Jetson, other M7 components, or broadcasts)
  EventType eventType;
  char eventBuffer[64];

  while (m7EventQueue.pop(eventType, eventBuffer)) {
    eventsReceived++;
    lastEventTime = currentMillis;

    // Process and display the event
    processStatusEvent(eventType, eventBuffer);

    // Log to serial
    Serial.print("M7 Event #");
    Serial.print(eventsReceived);
    Serial.print(": Type=");
    Serial.print((int)eventType);
    Serial.print(" Data=");
    Serial.println(eventBuffer ? eventBuffer : "(none)");
  }

  // Check for M4 timeout (no events received in 5 seconds)
  if (currentMillis - lastEventTime > 5000 && eventsReceived > 0) {
    if ((currentMillis / 1000) % 10 == 0) {  // Print once every 10 seconds
      display.println("WARNING: No M4 events", TerminalDisplay::RED_COLOR);
    }
  }

  delay(10);
}

// Process status events from M4
void processStatusEvent(EventType eventType, const char* eventData) {
  char displayBuffer[80];

  switch(eventType) {
    case EVENT_SYSTEM_HEALTH:
      // M4 sends: "Tilt:2.35° F:450 R:1200"
      // or: "M4 initialized" or "M4 hardware validation mode"
      if (eventData && (strstr(eventData, "Tilt:") || strstr(eventData, "T:"))) {
        // Parse sensor data
        snprintf(displayBuffer, sizeof(displayBuffer), "M4: %s", eventData);
        display.println(displayBuffer, TerminalDisplay::GREEN_COLOR);
      } else {
        // Status message
        snprintf(displayBuffer, sizeof(displayBuffer), "M4: %s",
                 eventData ? eventData : "Health OK");
        display.println(displayBuffer, TerminalDisplay::BLUE_COLOR);
      }
      break;

    case EVENT_BALANCE_STATUS:
      // Balance update from M4
      snprintf(displayBuffer, sizeof(displayBuffer), "Balance: %s",
               eventData ? eventData : "OK");
      display.println(displayBuffer, TerminalDisplay::TEXT_COLOR);
      break;

    case EVENT_COLLISION_WARNING:
      // Obstacle detected: "Forward obstacle: 85 mm" or "Rear obstacle: 92 mm"
      snprintf(displayBuffer, sizeof(displayBuffer), "!!! %s !!!",
               eventData ? eventData : "COLLISION WARNING");
      display.println(displayBuffer, TerminalDisplay::RED_COLOR);

      // Also log prominently to serial
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Serial.print("COLLISION WARNING: ");
      Serial.println(eventData ? eventData : "(no details)");
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      break;

    case EVENT_EMERGENCY_STOP:
      snprintf(displayBuffer, sizeof(displayBuffer), "*** EMERGENCY STOP ***");
      display.println(displayBuffer, TerminalDisplay::RED_COLOR);

      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Serial.println("  EMERGENCY STOP EVENT RECEIVED");
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      break;

    case EVENT_SYSTEM_STARTUP:
      snprintf(displayBuffer, sizeof(displayBuffer), "System: %s",
               eventData ? eventData : "STARTUP");
      display.println(displayBuffer, TerminalDisplay::GREEN_COLOR);
      break;

    case EVENT_SAFETY_ALERT:
      snprintf(displayBuffer, sizeof(displayBuffer), "SAFETY: %s",
               eventData ? eventData : "Alert!");
      display.println(displayBuffer, TerminalDisplay::RED_COLOR);

      Serial.print("SAFETY ALERT: ");
      Serial.println(eventData ? eventData : "(no details)");
      break;

    case EVENT_MOTOR_STATUS:
      snprintf(displayBuffer, sizeof(displayBuffer), "Motors: %s",
               eventData ? eventData : "Status update");
      display.println(displayBuffer, TerminalDisplay::YELLOW_COLOR);
      break;

    default:
      snprintf(displayBuffer, sizeof(displayBuffer), "Unknown Event %d: %s",
               (int)eventType, eventData ? eventData : "(no data)");
      display.println(displayBuffer, TerminalDisplay::RED_COLOR);

      Serial.print("WARNING: Unknown event type: ");
      Serial.println((int)eventType);
      break;
  }
}

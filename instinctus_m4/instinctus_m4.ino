/**
 * balancer_m4.ino - M4 Core Main Program (Hardware Validation Phase)
 *
 * Current Hardware:
 * - ICM20948 9-DOF IMU (I2C)
 * - VL53L4CX Time-of-Flight sensor - Forward (+X direction)
 * - VL53L4CX Time-of-Flight sensor - Rear (-X direction)
 *
 * Coordinate System (X-Forward Convention):
 * - X-axis: Forward/Backward (primary balance axis)
 * - Y-axis: Left/Right
 * - Z-axis: Up/Down (gravity reference)
 *
 * IMU Mounting: Mount ICM20948 with X-axis pointing forward, at wheel axle height
 *
 * Functionality:
 * - Report real-time IMU tilt data
 * - Monitor TOF sensors for obstacles < 10cm
 * - Send collision warnings to M7 core
 * - Emergency stop on dangerous tilt angles
 *
 * Motor control disabled until hardware installation
 */

#include <BalancerKit.h>
#include <vl53l4cx_class.h>
#include "IMUInterface.h"
#include "ICM20948Interface.h"
#include "BalanceIMU.h"
#include "BalanceObserver.h"
#include "BalanceEventObserver.h"

// Uncomment when motors are installed:
// #include <ACANFD_GIGA_R1.h>
// #include "MotorInterface.h"
// #include "ODriveS1Interface.h"
// #include "DriveCoordinator.h"
// #include "BalanceMotorController.h"

// ============================================================================
// Hardware Objects & I2C Bus Assignments
// ============================================================================

// I2C Sensors
// IMPORTANT: Mount ICM20948 with X-axis pointing forward, centered at wheel axle height
ICM20948Interface imuHardware(&Wire, 0x69);  // Balance IMU on default I2C bus (Wire)
// Forward TOF will use Wire (initialized later)
// Rear TOF will use Wire1 (initialized later)

// Algorithm Layer (Hardware Agnostic)
BalanceEventObserver balanceEventObserver;
BalanceIMU balanceIMU(&imuHardware);

// Motor control (DISABLED - uncomment when motors installed)
// ODriveS1Interface leftMotor(0x01);
// ODriveS1Interface rightMotor(0x02);
// DriveCoordinator driveSystem(&leftMotor, &rightMotor);
// BalanceMotorController motorController(&driveSystem);

// Time-of-Flight sensors
Adafruit_VL53L4CX tofForward = Adafruit_VL53L4CX();
Adafruit_VL53L4CX tofRear = Adafruit_VL53L4CX();

// Each TOF on separate I2C bus, both use default address 0x29
// Forward: Wire (default I2C bus)
// Rear: Wire1 (secondary I2C bus)

// ============================================================================
// Timing Variables
// ============================================================================

unsigned long lastBalanceUpdate = 0;
unsigned long lastTofUpdate = 0;
unsigned long lastStatusReport = 0;

const unsigned long BALANCE_UPDATE_INTERVAL = 10;   // 100Hz balance control
const unsigned long TOF_UPDATE_INTERVAL = 50;       // 20Hz TOF readings
const unsigned long STATUS_REPORT_INTERVAL = 100;   // 10Hz status reports for plotter

// ============================================================================
// Sensor State Variables
// ============================================================================

bool tofForwardActive = false;
bool tofRearActive = false;
uint16_t distanceForward = 0;   // mm
uint16_t distanceRear = 0;      // mm

const uint16_t COLLISION_THRESHOLD = 100;  // 10cm in millimeters

bool forwardObstacleDetected = false;
bool rearObstacleDetected = false;

// ============================================================================
// Setup
// ============================================================================

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(100);

  Serial.println("===========================================");
  Serial.println("  Balancer M4 - Hardware Validation");
  Serial.println("===========================================");
  Serial.println();

  // Initialize dual event queues
  Serial.print("Initializing event queues... ");
  m4EventQueue.initialize();
  m7EventQueue.initialize();
  Serial.println("OK");

  // Initialize I2C buses
  // Both buses needed because TOF sensors have the same I2C address (0x29)
  Serial.print("Initializing I2C buses... ");
  Wire.begin();   // Default I2C: ICM20948 (0x69) + Forward TOF (0x29)
  Wire1.begin();  // Secondary I2C: Rear TOF (0x29)
  Serial.println("OK"); 

  // Initialize IMU
  Serial.print("Initializing Balance IMU on Wire... ");
  if (!balanceIMU.initialize()) {
    Serial.println("FAILED!");
    Serial.println("ERROR: Check IMU wiring and I2C connection");
    while (true) delay(1000);
  }
  Serial.println("OK");

  // Add balance event observer (sends status to M7)
  Serial.print("Registering balance event observer... ");
  if (!balanceIMU.addObserver(&balanceEventObserver)) {
    Serial.println("FAILED!");
    while (true) delay(1000);
  }
  Serial.println("OK");

  // Add motor controller observer (DISABLED - uncomment when motors installed)
  // This provides IMMEDIATE motor response without event queue latency
  // Serial.print("Registering motor controller observer... ");
  // if (!balanceIMU.addObserver(&motorController)) {
  //   Serial.println("FAILED!");
  //   while (true) delay(1000);
  // }
  // Serial.println("OK");

  // Initialize Forward TOF sensor (on default I2C bus - Wire)
  Serial.print("Initializing Forward TOF sensor on Wire)... ");
  if (!tofForward.begin(0x29, &Wire)) {
    Serial.println("FAILED!");
    Serial.println("WARNING: Forward TOF not available");
    tofForwardActive = false;
  } else {
    // Configure ranging mode
    if (!tofForward.startRanging()) {
      Serial.println("FAILED to start ranging!");
      tofForwardActive = false;
    } else {
      Serial.println("OK");
      tofForwardActive = true;
    }
  }

  // Initialize Rear TOF sensor (on secondary I2C bus - Wire1)
  Serial.print("Initializing Rear TOF sensor on Wire1... ");
  if (!tofRear.begin(0x29, &Wire1)) {
    Serial.println("FAILED!");
    Serial.println("WARNING: Rear TOF not available");
    tofRearActive = false;
  } else {
    // Configure ranging mode
    if (!tofRear.startRanging()) {
      Serial.println("FAILED to start ranging!");
      tofRearActive = false;
    } else {
      Serial.println("OK");
      tofRearActive = true;
    }
  }

  Serial.println();
  Serial.println("M4 Core Initialized");
  Serial.println("IMU: ACTIVE");
  Serial.print("Forward TOF: ");
  Serial.println(tofForwardActive ? "ACTIVE" : "INACTIVE");
  Serial.print("Rear TOF: ");
  Serial.println(tofRearActive ? "ACTIVE" : "INACTIVE");
  Serial.println();
  Serial.println("Monitoring sensors...");
  Serial.println();

  // Send startup event to M7
  EventBroadcaster::sendToM7(EVENT_SYSTEM_HEALTH, "M4 hardware validation mode");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  unsigned long currentMillis = millis();

  // Update balance control at 100Hz
  if (currentMillis - lastBalanceUpdate >= BALANCE_UPDATE_INTERVAL) {
    balanceIMU.update();
    lastBalanceUpdate = currentMillis;
  }

  // Update TOF sensors at 20Hz
  if (currentMillis - lastTofUpdate >= TOF_UPDATE_INTERVAL) {
    updateTofSensors();
    lastTofUpdate = currentMillis;
  }

  // Process M4 events (commands from M7, broadcasts)
  EventType eventType;
  char eventBuffer[64];
  while (m4EventQueue.pop(eventType, eventBuffer)) {
    processM4Event(eventType, eventBuffer);
  }

  // Send periodic status reports
  if (currentMillis - lastStatusReport >= STATUS_REPORT_INTERVAL) {
    sendStatusReport();
    lastStatusReport = currentMillis;
  }

  // Short delay to prevent hogging CPU
  delay(System::MAIN_LOOP_DELAY);
}

// ============================================================================
// TOF Sensor Updates
// ============================================================================

void updateTofSensors() {
  // Read forward TOF sensor
  if (tofForwardActive && tofForward.dataReady()) {
    VL53L4CX_Result_t results;
    int status = tofForward.getResult(&results);

    if (status == VL53L4CX_ERROR_NONE) {
      distanceForward = results.distance_mm;

      // Check for obstacle
      bool obstacleNow = (distanceForward < COLLISION_THRESHOLD);

      // Trigger event on state change
      if (obstacleNow && !forwardObstacleDetected) {
        Serial.print("FORWARD OBSTACLE DETECTED: ");
        Serial.print(distanceForward);
        Serial.println(" mm");

        char msg[64];
        snprintf(msg, sizeof(msg), "Forward obstacle: %d mm", distanceForward);
        EventBroadcaster::sendToM7(EVENT_COLLISION_WARNING, msg);

        forwardObstacleDetected = true;
      } else if (!obstacleNow && forwardObstacleDetected) {
        Serial.println("Forward obstacle cleared");
        forwardObstacleDetected = false;
      }
    }

    tofForward.clearInterrupt();
  }

  // Read rear TOF sensor
  if (tofRearActive && tofRear.dataReady()) {
    VL53L4CX_Result_t results;
    int status = tofRear.getResult(&results);

    if (status == VL53L4CX_ERROR_NONE) {
      distanceRear = results.distance_mm;

      // Check for obstacle
      bool obstacleNow = (distanceRear < COLLISION_THRESHOLD);

      // Trigger event on state change
      if (obstacleNow && !rearObstacleDetected) {
        Serial.print("REAR OBSTACLE DETECTED: ");
        Serial.print(distanceRear);
        Serial.println(" mm");

        char msg[64];
        snprintf(msg, sizeof(msg), "Rear obstacle: %d mm", distanceRear);
        EventBroadcaster::sendToM7(EVENT_COLLISION_WARNING, msg);

        rearObstacleDetected = true;
      } else if (!obstacleNow && rearObstacleDetected) {
        Serial.println("Rear obstacle cleared");
        rearObstacleDetected = false;
      }
    }

    tofRear.clearInterrupt();
  }
}

// ============================================================================
// Event Processing
// ============================================================================

void processM4Event(EventType eventType, const char* eventData) {
  switch(eventType) {
    case EVENT_EMERGENCY_STOP:
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      Serial.println("  EMERGENCY STOP RECEIVED");
      Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      EventBroadcaster::sendToM7(EVENT_SAFETY_ALERT, "Emergency stop acknowledged");
      break;

    case EVENT_SYSTEM_STARTUP:
      Serial.print("System startup event: ");
      Serial.println(eventData ? eventData : "");
      EventBroadcaster::sendToM7(EVENT_SYSTEM_HEALTH, "M4 ready");
      break;

    default:
      Serial.print("Unhandled event: ");
      Serial.print((int)eventType);
      if (eventData) {
        Serial.print(" Data: ");
        Serial.println(eventData);
      } else {
        Serial.println();
      }
      break;
  }
}

// ============================================================================
// Status Reporting
// ============================================================================

void sendStatusReport() {
  // Get IMU data
  float tiltAngle = balanceIMU.getTiltAngle();
  float ax, ay, az, gx, gy, gz;
  balanceIMU.getAcceleration(ax, ay, az);
  balanceIMU.getAngularVelocity(gx, gy, gz);

  // Plotter-compatible format (one line per update)
  Serial.print("Tilt:");
  Serial.print(tiltAngle, 2);
  Serial.print(" AccelX:");
  Serial.print(ax, 2);
  Serial.print(" AccelY:");
  Serial.print(ay, 2);
  Serial.print(" AccelZ:");
  Serial.print(az, 2);
  Serial.print(" GyroX:");
  Serial.print(gx, 3);
  Serial.print(" GyroY:");
  Serial.print(gy, 3);
  Serial.print(" GyroZ:");
  Serial.print(gz, 3);

  // TOF data
  if (tofForwardActive) {
    Serial.print(" TOF_Fwd:");
    Serial.print(distanceForward);
  }

  if (tofRearActive) {
    Serial.print(" TOF_Rear:");
    Serial.print(distanceRear);
  }

  Serial.println();  // End line

  // Send condensed status to M7 (unchanged)
  char statusMsg[64];
  snprintf(statusMsg, sizeof(statusMsg), "T:%.1f° F:%d R:%d",
           tiltAngle, distanceForward, distanceRear);
  EventBroadcaster::sendToM7(EVENT_SYSTEM_HEALTH, statusMsg);
}

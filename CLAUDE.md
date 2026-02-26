# Calvin Instinctus - Project Instructions

## System Overview

**Calvin Instinctus** is the low-level reflex and motor control system for Calvin, a self-balancing robot. Running on an Arduino Giga R1 WiFi (dual-core ARM), it handles real-time balance control, motor actuation, and safety monitoring.

**Calvin's Three-System Architecture:**
- **instinctus** (THIS SYSTEM) - Reflexive motor control and balance
- **cogitator** - High-level AI and planning (Jetson Orin Nano) - `/Users/damoncali/code/calvin_cogitator/CLAUDE.md`
- **explorator** - Human monitoring interface (Electron app) - `/Users/damoncali/code/calvin_explorator/CLAUDE.md`

**Integration:**
- Sends status updates to cogitator via serial (M7 core)
- Receives commands from cogitator via serial (M7 core)
- Status data flows through cogitator to explorator for visualization

## Hardware Platform

**Arduino Giga R1 WiFi** - Dual-core STM32H747XI
- **M4 Core** (Cortex-M4 @ 240 MHz) - Real-time control at 100Hz
- **M7 Core** (Cortex-M7 @ 480 MHz) - Display and communication at 10Hz
- **RAM**: 1 MB (shared), **Flash**: 2 MB
- **Display**: Arduino Giga Display Shield 480×320 touchscreen
- **Connectivity**: WiFi, USB, CAN, I2C (x2), SPI, UART (x4)

**Sensors:**
- ICM20948 9-axis IMU (I2C Wire, 0x69) - Balance sensing
- 2x VL53L4CX ToF sensors (I2C Wire, rear 0x30 / front 0x29) - Collision detection
  - Both on same bus (Wire), differentiated via XSHUT pins (rear=D31, front=D32)
  - On boot: both XSHUT LOW, then rear brought up and reprogrammed to 0x30, then front brought up at default 0x29
  - XSHUT ensures clean power cycle on every MCU reset
- INA228 power monitor (I2C 0x40) - Battery monitoring

**Actuators:**
- 2x ODrive S1 motor controllers (CAN @ 250kbps, IDs 0x01/0x02)

## Architecture: Dual-Core Responsibilities

### M4 Core (Real-Time, 100Hz) - CRITICAL TIMING

**Tasks:**
1. **Balance Control** - Read IMU, run complementary filter, calculate tilt
2. **Motor Control** - Generate velocity commands, send CAN messages to ODrives
3. **Collision Detection** - Poll ToF sensors at 20 Hz
4. **Safety Monitoring** - Tilt limits, battery voltage, emergency stops

**Never block on M4** - No delays >1ms, no Serial.print, no blocking I/O

### M7 Core (10Hz) - Communication Hub

**Tasks:**
1. **Display Management** - Render events on touchscreen
2. **M4 ↔ Jetson Bridge** - Forward status to Jetson, route commands to M4
3. **Event Processing** - Pop from m7EventQueue, display and forward

## InstinctusKit Library

**Location:** `/Users/damoncali/code/arduino/librarius/InstinctusKit/src/`

### EventQueue System (M4 ↔ M7 Communication)

**Files:** `EventQueue.h`, `EventQueue.cpp`, `InstinctusKit.h`

**Queue Structure:**
```cpp
EventQueue m4EventQueue;  // M7 → M4 commands
EventQueue m7EventQueue;  // M4 → M7 status
```

**Event Item:**
```cpp
struct EventItem {
    EventType type;      // 1 byte enum
    char text[64];       // 64-byte payload
    bool read;           // Read flag
};
```

**Event Types:**

M4 → M7 (Status):
- TBD

M7 → M4 (Commands):
- TBD

Broadcast (Both):
- TBD

**EventBroadcaster API:**
```cpp
#include <InstinctusKit.h>

EventBroadcaster::sendToM4(EVENT_SET_TARGET_POSITION, "150,150");
EventBroadcaster::sendToM7(EVENT_BALANCE_STATUS, "2.35");
EventBroadcaster::broadcastEvent(EVENT_EMERGENCY_STOP, "collision");
```

## Project Structure

```
/Users/damoncali/code/arduino/calvin_instinctus/
├── instinctus_m4/              # M4 Core (Real-time control)
│   ├── instinctus_m4.ino       # Main M4 program
│   ├── BalanceIMU.{h,cpp}      # Complementary filter
│   ├── ICM20948Interface.{h,cpp}  # IMU driver
│   ├── ODriveS1Interface.{h,cpp}  # CAN motor driver
│   ├── DriveCoordinator.{h,cpp}   # Dual motor control
│   ├── BalanceMotorController.{h,cpp}  # Balance → Motor
│   └── BalanceEventObserver.{h,cpp}    # Balance → Events
│
├── instinctus_m7/              # M7 Core (Display/Comms)
│   ├── instinctus_m7.ino       # Main M7 program
│   ├── TerminalDisplay.{h,cpp} # Touchscreen display
│   └── JetsonInterface.{h,cpp} # Serial to Jetson
│
└── test_*/                     # Test sketches

/Users/damoncali/code/arduino/librarius/InstinctusKit/
└── src/
    ├── EventQueue.{h,cpp}      # Dual-queue system
    ├── InstinctusKit.h
    ├── Config.h                # Timing constants
    └── HardwareConfig.h        # Hardware settings
```

## Communication Protocols

### M4 ↔ M7 (Intercore)

**Protocol:** InstinctusKit EventQueue
**Method:** Shared memory with atomic operations

**Example:**
```cpp
// M4 sends tilt
EventBroadcaster::sendToM7(EVENT_BALANCE_STATUS, "2.35");

// M7 receives
EventItem event;
if (m7EventQueue.pop(event)) {
    display.println(event.text);
}
```

### M7 ↔ Jetson (Serial)

**Protocol:** Newline-delimited JSON (defined by cogitator)
**Physical:** Serial1/2/3 (TBD) at 115200 baud

**Message Format (see cogitator CLAUDE.md):**

M7 → Jetson:
```json
{"type":"balance_status","angle":2.35,"velocity":0.12}
{"type":"motor_status","left_vel":100,"right_vel":98}
{"type":"collision_warning","sensor":"front","distance":50}
```

Jetson → M7:
```json
{"type":"set_velocity","left":150,"right":150}
{"type":"emergency_stop","reason":"user_command"}
```

## Build and Upload

This project uses the **Grot** tool (~/code/gems/grot) for building and uploading Arduino sketches.

### M4 Core
```bash
cd instinctus_m4
grot load
```

### M7 Core
```bash
cd instinctus_m7
grot load
```

.grotconfig files specify board settings, target core, port, and memory allocation.

## Integration Points

### With Cogitator (Jetson)
**Documentation:** `/Users/damoncali/code/calvin_cogitator/CLAUDE.md`

**Interface:** Serial communication (NOT IMPLEMENTED)
**Data Flow:**
- M7 sends: balance_status, motor_status, collision_warning, battery_status
- M7 receives: set_velocity, emergency_stop, set_limits

**Next Steps:**
1. Implement JetsonInterface on M7
2. Coordinate with Jetson serial handler implementation
3. Test with ping/pong protocol

### With Explorator (Electron)
**Documentation:** `/Users/damoncali/code/calvin_explorator/CLAUDE.md`

**Interface:** Indirect via Jetson (no direct connection)
**Data Flow:** M4 → M7 → Jetson → Explorator

**Telemetry Provided:**
- Balance status (tilt angle, velocity)
- Motor status (position, velocity, current)
- Battery health (voltage, current, power)
- ToF distance measurements
- I2C bus health
- IMU FFT data for vibration analysis

## Safety Features

**Tilt Limits:**
- Warning and e stop limits.

**Watchdog Timer:** (TODO) M4 monitors M7 heartbeat, stops motors on timeout

**Battery Protection:** (TODO) Critical voltage shutdown at 9.5V (3S LiPo)

**Safe State:** Motors stop, balance continues, warning displayed, alert sent


## Code Style

- Class names: `PascalCase` (e.g., `BalanceIMU`)
- Methods: `camelCase` (e.g., `getTiltAngle()`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `CRITICAL_TILT_ANGLE`)
- Private members: `_camelCase`
- Comments for timing constraints: `// Must call at 100Hz`

## Resources

- [Arduino Giga R1 Docs](https://docs.arduino.cc/hardware/giga-r1-wifi/)
- [ODrive CAN Protocol](https://docs.odriverobotics.com/v/latest/can-protocol.html)
- [ICM20948 Datasheet](https://invensense.tdk.com/products/motion-tracking/9-axis/icm-20948/)

## Notes

- **Timing is critical** - M4 must maintain 100Hz for stable balance
- **Never block on M4** - No Serial.print, no delays >1ms
- **Serial is M7 only** - All M4 output goes via event queue
- **CAN conflicts** - Only M4 controls CAN bus

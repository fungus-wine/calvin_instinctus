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

**Teensy 4.1** - Microcontroller
- **Processor** ARM Cortex-M7 at 600 MHz
- **Memory** 7936K Flash, 1024K RAM (512K tightly coupled), 4K EEPROM (emulated)
- **Connectivity**: 8 serial, 3 SPI, 3 I2C ports, 3 CAN Bus (1 with CAN FD)

**Sensors:**
- ICM20948 9-axis IMU (I2C Wire, 0x69) - Balance sensing and collision detection
- 2x VL53L4CX ToF sensors (I2C Wire, rear 0x30 / front 0x29) - Obstacle detection
  - Both on same bus (Wire), differentiated via XSHUT pins (rear=D31, front=D32)
  - On boot: both XSHUT LOW, then rear brought up and reprogrammed to 0x30, then front brought up at default 0x29
  - XSHUT ensures clean power cycle on every MCU reset
- Battery monitor (specific model TBD)

**Actuators:**
- 2x ODrive S1 motor controllers (CAN @ 250kbps, IDs 0x01/0x02)
- 2x Odrive Dual Shaft Motor - D5312s 330KV

## Architecture: Dual-Core Responsibilities

### High Priority Tasks - CRITICAL TIMING

**Tasks:**
1. **Balance Control** - Read IMU, run complementary filter, calculate tilt
2. **Motor Control** - Generate velocity commands, send CAN messages to ODrives
3. **Collision Detection** - Poll ToF sensors at 20 Hz
4. **Safety Monitoring** - Tilt limits, battery voltage, emergency stops

**Never block high priority tasks** - No delays >1ms, no Serial.print, no blocking I/O

### Low Priortiy Tasks - Communication Hub

**Tasks:**
1. **Jetson Bridge** - Forward events to Jetson, route commands to M4


## Communication Protocols

### M7 ↔ Jetson (Serial)

**Protocol:** Newline-delimited JSON (defined by cogitator), details TBD
**Physical:** Serial1/2/3 (TBD) at 115200 baud


## Build and Upload
- This project uses the **Grot** tool (~/code/gems/grot) for building and uploading Arduino sketches.
- `Users/damoncali/code/gems/grot`
- .grotconfig files specify board settings, target core, port, and memory allocation.
- Grot is not yet capable of working with the Teensy 4.1

## Integration Points

### With Cogitator (Jetson Orin Nano, Python)
**Documentation:** `/Users/damoncali/code/calvin_cogitator/CLAUDE.md`

**Interface:** Serial communication (NOT IMPLEMENTED)
**Data Flow:**
- M7 sends: TBD
- M7 receives: TBD

### With Explorator (Electron/Vue.js)
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

**Proximity and Collision Detection:**
- Warning when too close to objects (detected by ToF sensors)
- Corrective action when collision detected (TBD).  

**Battery Protection:** (TODO) Critical voltage warning and shutdown at appropriate voltages (3S LiPo)

**Safe State:** Motors stop, balance continues, warning displayed, alert sent

**E Stop** Total motor shutdown.

## Code Style

- Class names: `PascalCase` (e.g., `BalanceIMU`)
- Methods: `camelCase` (e.g., `getTiltAngle()`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `CRITICAL_TILT_ANGLE`)
- Private members: `_camelCase`

## Resources

- [Arduino Giga R1 Docs](https://docs.arduino.cc/hardware/giga-r1-wifi/)
- [ODrive CAN Protocol](https://docs.odriverobotics.com/v/latest/can-protocol.html)
- [ICM20948 Datasheet](https://invensense.tdk.com/products/motion-tracking/9-axis/icm-20948/)

## Notes

- **Timing is critical** - M4 must maintain sufficient loop timing for stable balance
- **Never block on critical tasks** - No Serial.print, no delays >1ms (excepting debugging)

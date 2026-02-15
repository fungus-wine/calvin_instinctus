# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Upload Commands

This project uses the Grot tool to build and upload Arduino sketches to an Arduino GIGA R1 WiFi board with dual cores (M4 and M7).

### M4 Core (Primary Logic)
```bash
cd balancer_m4
grot load
```

### M7 Core (Display/UI)
```bash
cd balancer_m7
grot load
```

The configuration files `balancer_m4.yml` and `balancer_m7.yml` specify board settings including target core, port, and memory allocation.

## Library Management

This project uses custom libraries located in `../my-libraries/BalancerKit/` that get symlinked into `../libraries` for the Arduino IDE to discover them.

## Architecture Overview

This is a dual-core Arduino robot project with a layered, event-driven architecture:

### Core Design Patterns
- **Observer Pattern**: Sensors notify observers (like BalanceEventObserver) for real-time responses
- **Dual Event Queue System**: Separate queues for M4 and M7 with EventBroadcaster routing
- **Direct Event Processing**: Events processed directly via processM4Event() and processStatusEvent() functions
- **Hardware Abstraction**: Clean interfaces for sensor hardware (IMUInterface) and motors (MotorInterface)

### Inter-Core Communication Design
- **Dual Event Queues**:
  - `m4EventQueue`: Events for M4 to process (movement, balance control)
  - `m7EventQueue`: Events for M7 to process (sensor data, system health)
- **EventBroadcaster**: Helper class with sendToM4(), sendToM7(), broadcastEvent() methods
- **Thread-Safe**: Atomic mutex operations allow simultaneous access from both cores
- **Event Types**: Core-specific events plus broadcast events (emergency stop, system status)
- **Direct Processing**: Events processed directly via processM4Event() and processStatusEvent() functions

### Key Components
- **M4 Core**: Real-time balance control, motor management, collision detection (planned), safety systems (planned)
- **M7 Core**: Display management (implemented), Jetson communication (planned), navigation IMU (planned), command translation
- **Shared Libraries**: Dual event queues, configuration in `../my-libraries/BalancerKit/src/`
- **Sensor Layer**: BalanceIMU with complementary filter, ICM20948Interface for I2C communication
- **Motor Layer**: DriveCoordinator for dual-motor control, ODriveS1Interface for CAN communication

### Data Flow (Planned)
1. **M4 Balance Control**: ICM20948Interface → BalanceIMU → BalanceEventObserver → EventBroadcaster::sendToM7()
2. **M7 Navigation**: Built-in IMU → Jetson SLAM data (50Hz) - Not yet implemented
3. **Command Flow**: Jetson → M7 → EventBroadcaster::sendToM4() → m4EventQueue → processM4Event()
4. **Status Flow**: M4 → EventBroadcaster::sendToM7() → m7EventQueue → processStatusEvent()
5. **Safety Events**: Collision sensors → EventBroadcaster::broadcastEvent() → Both cores

### File Organization
- Headers (.h) contain interfaces and declarations
- Implementation (.cpp) files contain logic
- Shared configuration and utilities in `../my-libraries/BalancerKit/src/`
- Each core has its own main .ino file and configuration
- Test sketches in `test_*` directories verify individual components

## Current Implementation Status

### ✅ Fully Implemented & Tested
- **EventQueue.h/cpp**: Dual-core event queue system with atomic mutex operations
- **EventBroadcaster**: Clean routing with sendToM4(), sendToM7(), broadcastEvent() methods
- **BalanceIMU.h/cpp**: Complementary filter algorithm (98% gyro, 2% accel)
- **BalanceObserver.h**: Observer interface for balance events
- **BalanceEventObserver.h/cpp**: Bridge from balance events to inter-core event system
- **IMUInterface.h**: Abstract hardware interface for any IMU sensor
- **ICM20948Interface.h/cpp**: Concrete I2C driver for ICM20948 9-DOF sensor
- **MotorInterface.h**: Abstract interface for motor controllers
- **ODriveS1Interface.h/cpp**: CAN bus driver for ODrive S1 (partial - init complete, methods pending)
- **DriveCoordinator.h/cpp**: Dual-motor coordination and synchronization
- **TerminalDisplay.h/cpp**: GIGA R1 display with scrolling terminal output
- **balancer_m4.ino**: M4 main program with complete architecture integration
- **balancer_m7.ino**: M7 main program with event processing and test data generation
- **Config.h**: System timing constants (loop delays, sensor intervals)
- **HardwareConfig.h**: ICM20948 hardware constants (ranges, I2C address)

### ⏳ Planned / Not Implemented
- **Balance Control PID**: Convert tilt angle to motor velocity commands
- **Collision Detection**: Ultrasonic sensor integration and obstacle avoidance
- **Safety Systems**: Hardware emergency stop circuits and tilt limits
- **Position Limits**: Bounded motion control to prevent runaway
- **Jetson Communication**: USB serial link between M7 and Jetson Orin Nano
- **Navigation IMU**: Built-in GIGA R1 IMU for SLAM data
- **Autonomous Navigation**: Integration with Jetson for path planning

## Test Files

The following test sketches verify individual components:

- **test_dual_queues.ino**: Verifies dual event queue system, EventBroadcaster routing, atomic operations
- **test_balance_imu.ino**: Tests BalanceIMU complementary filter, IMUInterface abstraction, observer callbacks
- **test_imu_events.ino**: Tests complete IMU→Observer→EventQueue integration flow
- **test_motor_control.ino**: Tests motor control stack, CAN bus, ODrive communication, dual-motor sync

## Known Issues

1. **ODriveS1Interface partial** - CAN initialization implemented (first 50 lines), remaining methods need completion
2. **M7 test mode** - Currently sends random test data instead of real Jetson commands

## Architecture Notes

The architecture emphasizes modularity, decoupling, and clean separation between hardware abstraction, event processing, and behavior logic. All major components are now integrated in the main M4 program, which combines IMU balance control, motor coordination, and event-driven communication between cores.

### Recommended Next Steps
1. Complete ODriveS1Interface implementation (remaining CAN methods)
2. Implement balance control PID loop (tilt → motor velocity)
3. Add safety systems and tilt limits
4. Integrate Jetson communication on M7 core
5. Test full system integration with hardware

# Self-Balancing Robot Project

A dual-core Arduino GIGA R1 WiFi robot working with Jetson Orin Nano for autonomous navigation and computer vision.

## System Overview

### Hardware Architecture
- **Arduino GIGA R1 WiFi**: Dual ARM Cortex cores (M4 + M7)
- **Jetson Orin Nano**: AI processing and computer vision
- **ODrive S1**: Brushless motor controllers with encoders
- **Dual IMU Setup**: ICM20948 (balance) + Built-in GIGA IMU (navigation)
- **Ultrasonic Sensors**: Collision detection
- **Stereo Camera**: Vision processing on Jetson

### Three-Tier Control System

1. **Jetson Orin Nano**: High-level AI, vision, SLAM, path planning
2. **Arduino M7**: Communication hub, navigation IMU, display
3. **Arduino M4**: Real-time balance control, motor control, safety systems

## Inter-Core Communication Architecture

### Dual Event Queue System
- **m4EventQueue**: Events for M4 to process (movement, balance parameters)
- **m7EventQueue**: Events for M7 to process (sensor data, system health)
- **EventBroadcaster**: Helper for targeted or broadcast events with sendToM4(), sendToM7(), broadcastEvent()

### Data Flow
```
Command Flow: Jetson → M7 → EventBroadcaster::sendToM4() → m4EventQueue → M4 → ODrives → Motors
Status Flow:  M4 → EventBroadcaster::sendToM7() → m7EventQueue → M7 → Display/Jetson
Safety Flow:  Collision Sensors → EventBroadcaster::broadcastEvent() → Both Cores
```

### Communication Benefits
- **Thread-Safe**: Atomic operations prevent race conditions
- **Independent Timing**: Cores operate at different frequencies without coordination
- **Event Broadcasting**: Critical events (emergency stop) reach both cores
- **Clean Separation**: Each core processes only relevant events

## Project Structure

```
balancer/
├── README.md              # This file - project overview
├── CLAUDE.md              # AI assistant guidance
├── balancer_m4/           # M4 real-time control
│   ├── README.md          # M4-specific documentation
│   └── balancer_m4.ino    # Balance control main program
├── balancer_m7/           # M7 communication hub  
│   ├── README.md          # M7-specific documentation
│   └── balancer_m7.ino    # Communication main program
├── shared/                # Common libraries and utilities
└── documents/             # Design notes and specifications
```

## Development Plan

### Phase 1: Foundation (Complete)
- **✓ Dual Event Queue System**: Implemented m4EventQueue and m7EventQueue with EventBroadcaster
- **✓ Architecture Cleanup**: Removed StateMachine and Debug.h for simplified development
- **✓ Event Processing**: Direct event handling with processM4Event() and processStatusEvent()
- **Pending**: IMU Balance Control and ODrive Motor Integration
- **Result**: Clean event-driven architecture ready for balance control implementation

### Phase 2: Safety Systems (Week 3) 
- **Collision Detection**: Ultrasonic sensors on M4 with immediate hardware stops
- **Safety Broadcasting**: Emergency events reach both cores via EventBroadcaster
- **Position Limits**: Bounded motion control with automatic stopping
- **Result**: Safe balancing robot with hardware-level safety systems

### Phase 3: Communication Hub (Week 4)
- **Jetson Interface**: USB serial communication on M7
- **Navigation IMU**: Built-in GIGA IMU processing for SLAM data
- **Command Translation**: High-level Jetson commands → low-level M4 position targets
- **Result**: Remotely controlled robot with dual IMU navigation

### Phase 4: Full Autonomy (Week 5+)
- **Computer Vision Integration**: Stereo camera and object detection
- **SLAM and Path Planning**: Autonomous navigation with obstacle avoidance
- **Multi-Observer System**: Multiple observers per sensor for specialized processing
- **Result**: Fully autonomous self-balancing robot

## Key Design Principles

- **Safety First**: Hardware-level collision detection and emergency stops bypass event queues
- **Real-Time Reliability**: Balance control uses direct observer pattern (no queue delays)
- **Position-Based Control**: Target positions instead of speeds prevent runaway scenarios  
- **Modular Testing**: Each component testable independently with simple enable/disable flags
- **Dual IMU Strategy**: Separate sensors for balance (M4) vs navigation (M7)
- **Observer + Event Hybrid**: Immediate local response via observers, async coordination via events
- **Smart Event Filtering**: Observers generate events selectively to prevent queue overflow

## Getting Started

1. **Setup Hardware**: Connect IMU, motors, and sensors
2. **Build M4**: `cd balancer_m4 && grot load`
3. **Build M7**: `cd balancer_m7 && grot load`
4. **Test Components**: Use individual test sketches for each subsystem

See individual README files in `balancer_m4/` and `balancer_m7/` for core-specific details.
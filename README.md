# The Calvin Robot Project: Calvin Instinctus

Calvin is a two-wheeled self-balancing robot with on board AI processing and computer vision. The code for the project is broken into four repos:

- **Calvin Instinctus**: This Repo. Code for the M4 and M7 Cores of the Arduino GIGA R1 Wifi. This provides for Calvin's low level functionality and safety.
- **InstinctusKit**: Arudino Code that is shared between cores 
- **Calvin Cogitator**: Code that runs on the Jetson. This is Calvin's high level thought.
- **Calvin Explorator**: A Electron/Vue.js app that allows for human monitorig from a computer.

The 3D models are still a work in progress and will at some point be shared in another repo if I ever get it into reasonable shape. 

## System Overview

### Hardware Architecture
- **Arduino GIGA R1 WiFi**: Dual ARM Cortex cores (M4 + M7)
- **Arduino GIGA Display**: Dual ARM Cortex cores (M4 + M7)
- **Jetson Orin Nano**: AI processing, SLAM, etc. ZeroMQ with multiple services
- **OAK-D Pro W**: Stereo Camera with Computer vision AI models on board 
- **ODrive S1**: Brushless motor controllers with encoders
- **Dual IMU Setup**: ICM20948 (balance & collision detection) + Built-in OAK-D IMU (navigation)
- **ToF Sensors**: VL53L4CX ToF sensors for obstacle detection
- **Seeed Studios ReSpeaker 4 Mic Array**: Voice recognition and direction
- **Visaton FR58 Speaker Driver with Amp**: Makes noise

### Three-Tier Control System

1. **Jetson Orin Nano**: High-level AI, vision, audio, SLAM, path planning, etc
2. **Arduino M7**: Communication hub, navigation IMU, display
3. **Arduino M4**: Real-time balance control, motor control, safety systems

## Inter-Core Communication Architecture

### Dual Event Queue System
Event Queues live in shared memory so they can be accessed by either core of the Arduino.
- **m4EventQueue**: Events for M4 to process (movement, balance parameters)
- **m7EventQueue**: Events for M7 to process (sensor data, system health)
- **EventBroadcaster**: Helper for targeted or broadcast events with sendToM4(), sendToM7(), broadcastEvent()

### Data Flow
```
Command Flow: Jetson → M7 → EventBroadcaster::sendToM4() → m4EventQueue → M4 → ODrives → Motors
Status Flow:  M4 → EventBroadcaster::sendToM7() → m7EventQueue → M7 → Display/Jetson
Safety Flow:  Collision Sensors → EventBroadcaster::broadcastEvent() → Both Cores
```

## Development Notes
- The project uses standard arduino tools. I made and use a wrapper for the arduino-clli tool called grot, and a front end for grot called servitor. There's nothing special about them, though - I just find them convenient.
- There's lots of AI-written code in here (I use Claude Code frequently). If it's important, it's been heavily influenced and monitoried by me if not hand written. If it's not important, there's a decent chance you'll find some jank. Over time, the goal is to replace AI jank with human reviewed/written code across the entire project.
- This is a hobby, and I'm firmly into "uncqualified and unconcerned" territory. It's a giant work in progress that's only just beginning. None of this is finished, working code. 

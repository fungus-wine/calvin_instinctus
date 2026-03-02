# The Calvin Robot Project: Calvin Instinctus

Calvin is a two-wheeled self-balancing robot with on board AI processing and computer vision. The code for the project is broken into four repos:

- **Calvin Instinctus**: This Repo. Code for the Teensy 4.1. This provides for Calvin's low level functionality and safety.
- **Calvin Cogitator**: Code that runs on the Jetson. This is Calvin's high level thought.
- **Calvin Explorator**: A Electron/Vue.js app that allows for human monitorig from a computer.

The 3D models are still a work in progress and will at some point be shared in another repo if I ever get it into reasonable shape. 

## System Overview

### Hardware Architecture
- **Teensy 4.1**: ARM Cortex-M7 at 600 MHz
- **Jetson Orin Nano**: AI processing, SLAM, etc. ZeroMQ with multiple services
- **OAK-D Pro W**: Stereo Camera with Computer vision AI models on board 
- **ODrive S1**: Brushless motor controllers with encoders
- **Dual IMU Setup**: ICM20948 (balance & collision detection) + Built-in OAK-D IMU (navigation)
- **ToF Sensors**: VL53L4CX ToF sensors for obstacle detection
- **Seeed Studios ReSpeaker 4 Mic Array**: Voice recognition and direction
- **Visaton FR58 Speaker Driver with Amp**: Makes noise

### Two-Tier Control System

1. **Jetson Orin Nano**: High-level AI, vision, audio, SLAM, path planning, etc
2. **Teensy 4.1**: Real-time balance control, motor control, safety systems


## Development Notes
- The project uses standard arduino tools. I made and use a wrapper for the arduino-clli tool called grot, and a front end for grot called servitor. There's nothing special about them, though - I just find them convenient.
- There's lots of AI-written code in here (I use Claude Code frequently). If it's important, it's been heavily influenced and monitoried by me if not hand written. If it's not important, there's a decent chance you'll find some jank. Over time, the goal is to replace AI jank with human reviewed/written code across the entire project.
- This is a hobby, and I'm firmly into "uncqualified and unconcerned" territory. It's a giant work in progress that's only just beginning. None of this is finished, working code. 

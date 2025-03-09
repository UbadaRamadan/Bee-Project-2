# Stepper Motor Control with Arduino and Python

A project to control stepper motors via keyboard input, featuring adjustable speeds and smooth motion. Ideal for CNC machines, camera sliders, or robotics projects.

## Features
- **Keyboard Control**: Use arrow keys for direction and `S`/`D` to adjust speed.
- **3 Preset Speeds**: Slow, Medium, and Fast (configurable in code).
- **Smooth Motion**: Eliminates initial jerk when starting movement.
- **Real-Time Adjustments**: Change speed without stopping motors.
- **Emergency Stop**: Release arrow keys or press `ESC` to halt.

---

## Hardware Requirements
- Arduino Mega (or compatible board).
- Stepper motors (e.g., NEMA 17).
- Motor drivers (e.g., A4988, TMC2208).
- Jumper wires and power supply.

### Wiring Guide
| Motor Driver | Arduino Pin |
|--------------|-------------|
| X-axis DIR   | 2           |
| X-axis STEP  | 3           |
| Y-axis DIR   | 22          |
| Y-axis STEP  | 23          |

---

## Software Requirements
- Python 3.x
- Arduino CLI (for uploading code)
- `curses` and `pyserial` libraries
- Arduino IDE (optional)

### Install Dependencies
```bash
pip install pyserial curses

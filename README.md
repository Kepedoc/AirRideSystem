
# AirRideSystem - BMW E38 740iL

Modern non-blocking air suspension control system for the **BMW 7 Series E38 (740iL)** with RF remote control and emergency pause/resume functionality.

Built with Arduino, OLED display, and 433 MHz RF receiver.

---

## Table of Contents
- [Description](#description)
- [Features](#features)
- [Hardware Components](#hardware-components)
- [Wiring](#wiring)
- [Electronic Logic](#electronic-logic)
- [Code Logic](#code-logic)
- [3D Printed Parts](#3d-printed-parts)
- [Finished Product](#finished-product)
- [Installation & Upload](#installation--upload)
- [RF Remote Codes](#rf-remote-codes)
- [Future Improvements](#future-improvements)

---

## Description

This project replaces the original (or adds to) the air suspension control on a BMW E38 740iL with a fully custom, reliable, and safe Arduino-based system.

It allows you to call **three different height/pressure presets** via a wireless RF remote, while constantly monitoring front, rear, and tank pressures. The system includes smart compressor control, over-pressure protection, tank margin logic, and an **emergency pause/resume** button.

Everything runs **non-blocking**, so the system remains responsive at all times.

---

## Features

- 3 programmable pressure presets (currently: 0/0 bar, 6/6 bar, 11/11 bar)
- Real-time pressure monitoring (front, rear, storage tank)
- Non-blocking preset execution with timeout protection
- Emergency pause / resume via RF remote
- Smart compressor activation (only when needed)
- Tank pressure margin safety (prevents compressor running against low tank)
- OLED display feedback
- Tolerance-based control (±0.5 bar)
- 60-second safety timeout per preset
- Clean valve and compressor control logic

---

## Hardware Components

- **Microcontroller**: Arduino Uno R3
- **Display**: 0.91" or 1.3" SSD1306 OLED (128x32 or 128x64)
- **RF Receiver**: 433 MHz
- **Pressure Sensors**: 3× analog pressure transducers
- **Compressor**: Dual air compressor pack
- **Solenoids**: 2× pneumatic valves 5/3
- **Power**: 5v and 24v power converter

---

## Wiring

**Inputs (Analog):**
- `A0` → Front axle pressure sensor
- `A1` → Rear axle pressure sensor
- `A2` → Storage tank pressure sensor

**Outputs (Digital):**
- `D8`  → Compressor relay
- `D9`  → Front Up (Inflate) solenoid
- `D10` → Rear Up (Inflate) solenoid
- `D11` → Front Down (Deflate) solenoid
- `D12` → Rear Down (Deflate) solenoid
- `D2`  → 433 MHz RF Receiver data pin (interrupt)

**I2C OLED:**
- SDA → A4
- SCL → A5

> Full wiring diagram and pinout image coming soon.

---

## Electronic Logic

- All valves are **active HIGH**
- Compressor is **active HIGH**
- Valves use complementary logic (inflate and deflate cannot be active at the same time)
- Compressor only runs when at least one axle needs more pressure **and** tank has sufficient margin

---

## Code Logic

The code is designed to be **non-blocking** and safe:

- Presets run in the background via `doPresetStep()` called every ~200 ms
- Emergency toggle instantly pauses/resumes the current preset
- Independent front and rear control
- Automatic completion detection when both axles reach target ± tolerance
- Full cleanup of all outputs when preset ends or is stopped

Key constants you can tune:
- `TOLERANCE` (currently 0.5 bar)
- `TANK_MARGIN` (currently 2.0 bar)
- `PRESET_TIMEOUT` (60 seconds)
- `VOLTS_PER_BAR` (calibrate to your exact sensors!)

---

## 3D Printed Parts

(Section under construction – add your parts here)
- Enclosure for Arduino + OLED
- Mounting bracket for pressure sensors
- ...

---

## Finished Product

(Insert photos here – before/after, installed in car, OLED screenshots, etc.)
![alt text](https://de.pinterest.com/pin/707135579033631707)

---

## Installation & Upload

1. Install the following libraries via Library Manager:
   - `Adafruit SSD1306`
   - `Adafruit GFX`
   - `RCSwitch`

2. Adjust `VOLTS_PER_BAR` constant to match **your** pressure sensors
3. (Optional) Change the RF codes in the top of the sketch
4. Upload the code to your Arduino
5. Test in a safe environment before driving

---

## RF Remote Codes

| Button       | Code       | Function                          |
|--------------|------------|-----------------------------------|
| Button 1     | 14637857   | Emergency Pause / Resume          |
| Preset 1     | 11789684   | Preset 1 (0/0 bar – fully down)   |
| Preset 2     | 2673720    | Preset 2 (6/6 bar)                |
| Preset 3     | 2673714    | Preset 3 (11/11 bar – fully up)   |

You can easily change these codes to match your remote.

---

## Future Improvements

- Add height sensors (instead of only pressure)
- Manual up/down buttons
- Bluetooth / WiFi control via phone
- Data logging (SD card or telemetry)
- Low tank / low voltage warnings
- Auto-leveling mode
- Integration with vehicle CAN bus (E38-specific)

---

**Project status**: Working prototype – daily driven on BMW E38 740iL

Feel free to open issues or submit pull requests!

---

Made with ❤️ for the E38 community.


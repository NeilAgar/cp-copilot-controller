# Open-Source Assistive "Copilot" Controller

> A durable, low-cost compliant mechanism controller designed for users with spasticity. It uses advanced signal processing to filter out involuntary tremors and includes a "split-control" feature for cooperative gaming, also letting you play on your switch with a computer.

---

![Fully assembled controller next to Nintendo Switch](controller.png)

## Table of Contents

* [The Problem & Solution](#the-problem--solution)
* [Getting Started](#getting-started)
* [Bill of Materials (BOM)](#bill-of-materials-bom)
* [3D Printing/Fabrication](#3d-printingfabrication)
* [Assembly](#assembly)


* [Firmware and Software](#firmware-and-software)
* [Flashing the ESP32 Firmware](#flashing-the-esp32-firmware)
* [Setting up the PC "Copilot" App](#setting-up-the-pc-copilot-app)


* [Usage](#usage)
* [How it Works (Technical Details)](#how-it-works-technical-details)

---

## The Problem & Solution

Traditional mechanical accessibility switches are often expensive, fragile, and fail to account for spastic movements. For users with conditions like Cerebral Palsy, high-impact involuntary movements can break standard hinges or register erratic false inputs.

**The Solution:** This controller replaces mechanical parts with a 3D-printed compliant mechanism (a solid-state spring that flexes) and capacitive sensing. Instead of a simple on/off switch, the onboard ESP32-S3 uses a Kalman filter and adaptive algorithms to distinguish between intentional presses and spastic tremors in real-time.

It also features a **"Split-Control Bridge,"** allowing a partner to handle navigation tasks (like movement) via Wi-Fi from a PC, while the user controls primary actions on the controller.

---

## Getting Started

### Bill of Materials (BOM)

You can purchase all necessary components via this **[Amazon Wishlist](https://www.amazon.com/hz/wishlist/ls/JKSAYW0141NF?ref_=wl_share)**.

| Item | Description | Qty | Note |
| --- | --- | --- | --- |
| **Microcontroller** | ESP32-S3 Dev Board OR Adafruit Metro ESP32-S3 | 1 | The Adafruit board is easier to wire but costs slightly more. Only choose one! |
| **Wires** | 22-24 AWG Stranded Wire to connect components & the foil |
| **Potentiometer** | 10k Potentiometer (Optional) | 1 | Allows for on-the-fly sensitivity adjustment. |
| **Sensor Material** | Aluminum Foil | Small Square | Kitchen foil works perfectly. |

### 3D Printing/Fabrication

The design uses a compliant mechanism, so it must be printed in **PETG** or **PLA+** for durability. Do not use standard brittle PLA.

**Option 1: Print it yourself**
Choose the files matching your microcontroller.

* **For Generic ESP32-S3:**
* [Download STLs](print_files/ESP32-prints.zip)
* [Download 3MF (OrcaSlicer)](print_files/Print-ESP32.3mf)


* **For Adafruit Metro ESP32-S3:**
* [Download STLs](print_files/Adafruit-prints.zip)
* [Download 3MF (OrcaSlicer)](print_files/Print-Adafruit.3mf)



**Option 2: Order printed parts (If you don't own a printer)**
Use these links to order the parts directly through CraftCloud.

* [Order parts for Generic ESP32-S3 Version](https://craftcloud3d.com/configuration/b1fc5dbd-c97f-46f4-a522-1355bb4c70ae)
* [Order parts for Adafruit Metro Version](https://craftcloud3d.com/configuration/f809734d-5e56-4cc8-a2c9-b1faf550a3c4)

### Assembly

Detailed assembly instructions and video guides can be found here:
**[View Assembly Guide & Pictures (Google Drive)](https://drive.google.com/drive/folders/16ZFhgrj6qlABAafDJcucosywIMY7Crnk)**

---

## Firmware and Software

### Flashing the ESP32 Firmware

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Follow instructions online to add `esp32` board support to your IDE.
3. Download the firmware file from this repo: [`copilot_firmware.ino`](copilot_firmware.ino).
4. Open the `.ino` file in Arduino IDE. Select your board type (e.g., "Adafruit Metro ESP32-S3" or "ESP32S3 Dev Module") and the correct COM port.
5. Click **Upload** (the arrow pointing to the right).

> **Troubleshooting:** If the upload fails to connect, hold down the **BOOT** button on your ESP32 board, press and release the **RESET** button, then release the **BOOT** button. Hit upload again.

### Setting up the PC "Copilot" App

This Python script allows a partner on a PC to send navigation inputs to the controller over Wi-Fi.

1. Download and install [Python](https://www.python.org/downloads/) (ensure you check "Add Python to PATH" during installation).
2. Open your command prompt or terminal and install the required library:
```bash
pip install pygame
```


3. Download the controller script: [`controller.py`](controller.py).

---

## Usage

1. **Connect Hardware:** Plug the finished controller into your Nintendo Switch dock or PC via USB-C. It will be recognized as a wired Pro Controller.
2. **Connect Network:** On your PC laptop, connect to the Wi-Fi network named `MarioCopilot` (Password: `12345678`).
3. **Run Bridge:** On your PC, run the Python script. A window will appear showing controller status.
```bash
python controller.py
```


4. **Play:** The user presses the main button on the controller for actions (e.g., Jump), while the partner uses the PC keyboard (WASD/Arrow Keys) for movement.

---

## How it Works (Technical Details)

This device addresses the "noisy signal" problem inherent in spasticity. Involuntary tremors can cause standard capacitive sensors to trigger falsely.

Instead of a simple threshold, this firmware runs a **1-Dimensional Kalman Filter** on the raw sensor data. This mathematical algorithm estimates the true intent of the user by smoothing out high-frequency jitter (tremors) while responding quickly to significant changes (intentional presses).

Furthermore, the system uses an adaptive baseline that constantly re-calibrates to changing environmental conditions like humidity or temperature drift, ensuring consistent performance without manual recalibration.
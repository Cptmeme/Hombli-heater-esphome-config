# Hombli Smart Convector Heater (2000W) - ESPHome Custom Driver

This repository contains a custom **ESPHome configuration** to fully control the **Hombli Smart Convector Heater (Glass Panel 2000W)** locally, bypassing the Tuya cloud and the standard ESPHome Tuya component.

**Device Link:** [Hombli Smart Convector Heater 2000W (White/Black)](https://hombli.com/nl/collections/verwarming/products/hombli-slimme-convectorkachel-2000w-wit-glas)

## 🛑 The Problem
This specific heater uses a "Hybrid" TuyaMCU protocol that is incompatible with the standard ESPHome `tuya:` component.
1.  **Init Failure:** Attempting to use the standard component results in an endless `Initialization failed at init_state 0` loop.
2.  **Handshake Issues:** The device requires a specific "Version 0" handshake to wake up, but communicates via "Version 3" packets afterwards.
3.  **Connection Reset:** If the MCU receives standard Tuya heartbeat packets repeatedly, it resets the connection, stopping data updates (temperature/power) from being reported.

## 🚀 The Solution: "Raw UART Driver"
This configuration uses a **Pure YAML Custom Driver** approach.
* **No External Dependencies:** No `.h` files or custom components required.
* **Direct UART Parsing:** We read raw hex bytes directly from the UART bus using a lambda function.
* **Manual Handshake:** A script runs *once* at boot to wake the device up (`tasmota_fix`).
* **Passive Polling:** A lightweight poll loop (`0x08` Query) keeps the connection alive without triggering a reset.

## ⚙️ Features
* **Full Local Control:** Works entirely offline via Home Assistant / ESPHome API.
* **Thermostat Control:** Set target temperature (5°C - 35°C).
* **Mode Selection:** High, Low, and Eco (Anti-freeze) modes.
* **Feedback:** Accurate Room Temperature and Power State reporting.
* **Extra Features:**
    * **Child Lock:** Toggle the physical lock on the device.
    * **Screen Display:** Turn the LED display On/Off.
    * **Silent Mode:** Turn the "Beep" sound On/Off.

## 🛠️ Hardware & Installation
This configuration is designed for the **WT0132C6-S5** module (or any esp-based replacement board).

### The Wireless Module: WT0132C6-S5
The heater's chip was replaced by a **WT0132C6-S5** module.
* **Core:** Espressif **ESP32-C6** SoC (RISC-V 32-bit, 160MHz)
* **Wireless:** Wi-Fi 6 (802.11ax), Bluetooth 5.3, Zigbee 3.0, Thread 1.3
* **Memory:** 4MB Flash, 512KB SRAM
* **Pins:** The UART communication occurs on pins **GPIO16 (TXD)** and **GPIO17 (RXD)**.

### Wiring
* **TX (ESP)** -> **RX (Heater)**
* **RX (ESP)** -> **TX (Heater)**
* **GND** -> **GND**
* **3.3V** -> **3.3V**

*Note: The provided config uses `GPIO16` (TX) and `GPIO17` (RX), matching the WT0132C6-S5 pinout.*

### How to Use
1.  Copy the contents of `tuyamcu.yaml` into your ESPHome configuration.
2.  Update the `wifi` and `ota` passwords in the secrets section.
3.  Adjust the `uart` pins if your board differs from the ESP32-C6 chip used.
4.  Flash the device.

## 📊 Datapoint Mapping (DPID)
For those interested in the reverse-engineering logic, here are the raw Tuya Datapoints used:

| DP ID | Type | Function | Notes |
| :--- | :--- | :--- | :--- |
| **1** | Bool | **Power** | `1` = On, `0` = Off |
| **2** | Int | **Target Temp** | Value in °C |
| **3** | Int | **Current Temp** | Value in °C (Read Only) |
| **4** | Enum | **Mode** | `0`=High, `1`=Low, `2`=Eco |
| **7** | Bool | **Child Lock** | `1` = Locked |
| **101** | Bool | **Screen** | **Inverted logic:** `0` = On, `1` = Off |
| **103** | Bool | **Sound/Beep** | **Inverted logic:** `0` = On, `1` = Off |

## ⚠️ Disclaimer
This involves modifying mains-voltage appliances.
* **Do not attempt this if you are not comfortable with soldering or electronics.**
* **Always unplug the heater** before opening it.

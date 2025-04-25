# Ring The Cows Project - Arduino Firmware

## Overview
The "Ring The Cows" project is an IoT system that uses ESP8266 microcontrollers to create a network of connected cowbells. The system allows for remote triggering of bells over WiFi using UDP communication.

## Hardware Components
- ESP8266 WiFi microcontroller
- Solenoid for bell striking mechanism
- Tilt switch for motion detection
- LEDs for status indication (red and blue)
- Button for manual control
- Battery power with level monitoring

## Features
- **WiFi Connectivity**: Connects to a specified WiFi network
- **UDP Communication**: Sends and receives messages over UDP
- **Bell Control**: Can trigger the bell remotely or via motion
- **Motion Detection**: Uses a tilt switch to detect movement and trigger the bell
- **Status Reporting**: Sends regular status updates including:
  - Battery level
  - WiFi signal strength (RSSI)
  - Device ID
  - Firmware version
- **Power Management**: Includes deep sleep mode to conserve battery
- **Manual Control**: Button for manual bell triggering and sleep mode

## Communication Protocol
The system uses a simple text-based protocol over UDP:
- **P**: Ping message (responds with bell ID)
- **B**: Bang message (triggers the bell)
- **T**: Tilt switch toggle (enables/disables motion triggering)
- **R**: Reconnect WiFi
- **S**: Status request (responds with device information)
- **H**: Heartbeat message

## Configuration
- The bell ID is stored in EEPROM
- WiFi credentials are hardcoded (SSID: "cowbell", Password: "##bellbell##")
- Default UDP port: 9999 (listening)

## Memory Optimization
The code has been optimized for the limited memory of the ESP8266:
- Uses char arrays instead of String objects to reduce memory fragmentation
- Builds UDP messages in a buffer before sending to minimize network operations
- Efficient use of conditional statements

## Version
Current firmware version: 9

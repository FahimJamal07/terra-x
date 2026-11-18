# TERRA-X – Autonomous Agricultural Rover for Sericulture

The firmware uses a modular PlatformIO/Arduino architecture for an ESP32-D0WD-V3 rover. It provides manual and autonomous movement, NEO-6M GPS NMEA parsing through TinyGPSPlus, ZTS-3002-compatible Modbus soil readings over RS485, waypoint tracking, sampling, and a web dashboard.

## Build

```text
pio run
```

The project uses the PlatformIO `esp32dev` board definition with the Arduino framework. The pin map and current hardware settings are defined only in `src/config.h`.

## Architecture

- `src/main.cpp`: Starts Serial, initializes the system, and runs the application update loop.
- `src/config.h`: Single source of truth for GPIO assignments and current hardware constants. The NEO-6M GPS uses the existing UART2 RX GPIO34/TX GPIO33 configuration; the soil RS485 transceiver uses RX GPIO13, TX GPIO14, and DE/RE GPIO4.
- `src/types.h`: Shared enums, sensor data structures, and system status.
- `src/core/`: Application lifecycle and shared system state.
- `src/hardware/`: Hardware-specific driver boundaries for the L298N, TB6612FNG, NEO-6M GPS UART, and soil RS485 link.
- `src/sensors/`: Sensor-facing modules that will later consume hardware drivers and publish typed readings.
- `src/control/`: Manual and actuator control orchestration. Controllers do not know about web transport.
- `src/safety/`: Emergency-stop and safety policy boundary.
- `src/autonomy/`: Future waypoint and navigation logic. It has no GPIO access.
- `src/mission/`: Future mission orchestration.
- `src/sampling/`: Future sampling workflow controller.
- `src/communications/web/`: Future HTTP/API transport. It must communicate through controller and state interfaces, never GPIO.
- `src/ui/dashboard/`: Future dashboard presentation boundary.

All modules expose `begin()` and `update()` lifecycle methods where appropriate. Sensor communication is kept behind hardware and sensor interfaces so transport changes do not leak into control or web code.

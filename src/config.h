#pragma once

#include <stdint.h>

namespace config {
namespace rover {
constexpr uint8_t ena = 25;
constexpr uint8_t in1 = 26;
constexpr uint8_t in2 = 27;
constexpr uint8_t in3 = 18;
constexpr uint8_t in4 = 19;
constexpr uint8_t enb = 23;
constexpr uint8_t speed_minimum = 0;
constexpr uint8_t speed_maximum = 255;
constexpr uint8_t speed_default = 180;
constexpr uint32_t command_timeout_ms = 600;
}  // namespace rover

namespace actuator {
constexpr uint8_t ain1 = 16;
constexpr uint8_t ain2 = 17;
constexpr uint8_t pwma = 32;
constexpr uint8_t stby = 12;
constexpr uint32_t command_timeout_ms = 600;
}  // namespace actuator

namespace gps {
constexpr uint8_t uart_rx = 34;
constexpr uint8_t uart_tx = 33;
constexpr uint32_t baud = 9600;
}  // namespace gps

namespace soil {
constexpr bool enabled = true;
constexpr uint8_t uart_rx = 13;
constexpr uint8_t uart_tx = 14;
constexpr uint8_t de_re = 4;
constexpr uint32_t baud = 4800;
constexpr uint8_t modbus_address = 1;
}  // namespace soil

namespace sensors {
constexpr uint32_t gps_stale_timeout_ms = 3000;
constexpr uint32_t soil_stale_timeout_ms = 3000;
}  // namespace sensors

namespace navigation {
constexpr double arrival_radius_m = 2.0;
constexpr uint8_t minimum_speed = 171;
constexpr uint8_t maximum_speed = 200;
constexpr uint32_t gps_stale_timeout_ms = 3000;
}  // namespace navigation

namespace sampling {
constexpr uint32_t lowering_time_ms = 300;
constexpr uint32_t settling_time_ms = 500;
constexpr uint32_t raising_time_ms = 300;
}  // namespace sampling

namespace wifi {
constexpr char ssid[] = "SOIL_VEHICLE";
constexpr char password[] = "12345678";
}  // namespace wifi
}  // namespace config

#include "gps.h"

#include "../../config.h"

void GPSHardware::begin() {
    serial_.begin(config::gps::baud, SERIAL_8N1, config::gps::uart_rx, config::gps::uart_tx);
    serial_.setTimeout(0);
    data_ = GPSData{};
}

void GPSHardware::update() {
}

int GPSHardware::available() {
    return serial_.available();
}

int GPSHardware::read() {
    return serial_.read();
}

size_t GPSHardware::read(uint8_t* buffer, size_t length) {
    return serial_.readBytes(buffer, length);
}

HardwareSerial& GPSHardware::serial() {
    return serial_;
}

GPSData GPSHardware::data() const {
    return data_;
}

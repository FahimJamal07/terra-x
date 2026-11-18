#include "gps_sensor.h"

#include <Arduino.h>
#include <math.h>
#include "../../config.h"

void GPSSensor::begin(GPSHardware& hardware) {
    hardware_ = &hardware;
    data_ = GPSData{};
    lastValidUpdate_ = 0;
}

void GPSSensor::update() {
    if (hardware_ == nullptr) {
        return;
    }

    uint16_t processed = 0;
    while (hardware_->available() > 0 && processed < 128) {
        processByte(static_cast<char>(hardware_->read()));
        ++processed;
    }
}

GPSData GPSSensor::getData() const {
    return data_;
}

GPSData GPSSensor::data() const {
    return getData();
}

bool GPSSensor::isHealthy() const {
    return hasFreshData();
}

void GPSSensor::processByte(char value) {
    gps_.encode(static_cast<uint8_t>(value));
    if (!gps_.location.isValid()) {
        return;
    }
    data_.latitude = gps_.location.lat();
    data_.longitude = gps_.location.lng();
    data_.fixValid = true;
    data_.timestamp = millis();
    lastValidUpdate_ = data_.timestamp;
    if (gps_.speed.isValid()) {
        data_.speedKmph = gps_.speed.kmph();
    }
    if (gps_.altitude.isValid()) {
        data_.altitudeMeters = gps_.altitude.meters();
    }
    if (gps_.satellites.isValid()) {
        data_.satellites = static_cast<uint8_t>(gps_.satellites.value());
    }
}

bool GPSSensor::hasFreshData() const {
    return lastValidUpdate_ != 0 &&
           millis() - lastValidUpdate_ <= config::sensors::gps_stale_timeout_ms;
}

#include "safety.h"

#include <Arduino.h>

#include "../config.h"

void Safety::begin(L298N& roverDriver, TB6612& actuatorDriver) {
    roverDriver_ = &roverDriver;
    actuatorDriver_ = &actuatorDriver;
    roverDriver_->stop();
    actuatorDriver_->stop();
    emergencyStopActive_ = false;
    roverTimedOut_ = true;
    actuatorTimedOut_ = true;
    lastRoverRefresh_ = millis();
    lastActuatorRefresh_ = lastRoverRefresh_;
}

void Safety::refreshRover() {
    if (!emergencyStopActive_) {
        lastRoverRefresh_ = millis();
        roverTimedOut_ = false;
    }
}

void Safety::refreshActuator() {
    if (!emergencyStopActive_) {
        lastActuatorRefresh_ = millis();
        actuatorTimedOut_ = false;
    }
}

void Safety::emergencyStop() {
    emergencyStopActive_ = true;
    if (roverDriver_ != nullptr) {
        roverDriver_->stop();
    }
    if (actuatorDriver_ != nullptr) {
        actuatorDriver_->stop();
    }
}

void Safety::update() {
    const unsigned long now = millis();
    if (now - lastRoverRefresh_ >= config::rover::command_timeout_ms) {
        roverTimedOut_ = true;
        if (roverDriver_ != nullptr) {
            roverDriver_->stop();
        }
    }
    if (now - lastActuatorRefresh_ >= config::actuator::command_timeout_ms) {
        actuatorTimedOut_ = true;
        if (actuatorDriver_ != nullptr) {
            actuatorDriver_->stop();
        }
    }
    if (emergencyStopActive_) {
        if (roverDriver_ != nullptr) {
            roverDriver_->stop();
        }
        if (actuatorDriver_ != nullptr) {
            actuatorDriver_->stop();
        }
    }
}

void Safety::clearEmergencyStop() {
    emergencyStopActive_ = false;
    const unsigned long now = millis();
    lastRoverRefresh_ = now;
    lastActuatorRefresh_ = now;
    roverTimedOut_ = true;
    actuatorTimedOut_ = true;
}

bool Safety::isEmergencyStopped() const {
    return emergencyStopActive_;
}

bool Safety::emergencyStopActive() const {
    return emergencyStopActive_;
}

bool Safety::roverTimedOut() const {
    return roverTimedOut_;
}

bool Safety::actuatorTimedOut() const {
    return actuatorTimedOut_;
}

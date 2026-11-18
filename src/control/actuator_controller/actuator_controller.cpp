#include "actuator_controller.h"

#include "../../config.h"

void ActuatorController::begin(TB6612& driver, Safety& safety) {
    driver_ = &driver;
    safety_ = &safety;
    speed_ = config::rover::speed_default;
    state_ = ActuatorState::Stopped;
    driver_->stop();
}

void ActuatorController::update() {
    if (safety_ != nullptr && (safety_->isEmergencyStopped() || safety_->actuatorTimedOut())) {
        stop();
    }
}

void ActuatorController::up() {
    if (driver_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
        stop();
        return;
    }
    state_ = ActuatorState::Up;
    driver_->up(speed_);
    safety_->refreshActuator();
}

void ActuatorController::down() {
    if (driver_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
        stop();
        return;
    }
    state_ = ActuatorState::Down;
    driver_->down(speed_);
    safety_->refreshActuator();
}

void ActuatorController::stop() {
    state_ = ActuatorState::Stopped;
    if (driver_ != nullptr) {
        driver_->stop();
    }
}

void ActuatorController::setSpeed(uint8_t speed) {
    speed_ = constrainedSpeed(speed);
    if (state_ != ActuatorState::Stopped &&
        safety_ != nullptr && !safety_->isEmergencyStopped() && driver_ != nullptr) {
        driver_->setSpeed(speed_);
    }
}

ActuatorState ActuatorController::getState() const {
    return state_;
}

uint8_t ActuatorController::constrainedSpeed(uint8_t speed) const {
    if (speed < config::rover::speed_minimum) {
        return config::rover::speed_minimum;
    }
    if (speed > config::rover::speed_maximum) {
        return config::rover::speed_maximum;
    }
    return speed;
}

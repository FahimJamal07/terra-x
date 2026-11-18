#include "rover_controller.h"

#include "../../config.h"

void RoverController::begin(L298N& driver, Safety& safety) {
    driver_ = &driver;
    safety_ = &safety;
    speed_ = config::rover::speed_default;
    direction_ = DriveDirection::Stop;
    driver_->stop();
}

void RoverController::update() {
    if (safety_ != nullptr && (safety_->isEmergencyStopped() || safety_->roverTimedOut())) {
        stop();
    }
}

void RoverController::stop() {
    direction_ = DriveDirection::Stop;
    if (driver_ != nullptr) {
        driver_->stop();
    }
}

void RoverController::forward(uint8_t speed) {
    if (driver_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
        stop();
        return;
    }
    speed_ = constrainedSpeed(speed);
    direction_ = DriveDirection::Forward;
    driver_->forward(speed_);
    safety_->refreshRover();
}

void RoverController::reverse(uint8_t speed) {
    if (driver_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
        stop();
        return;
    }
    speed_ = constrainedSpeed(speed);
    direction_ = DriveDirection::Reverse;
    driver_->reverse(speed_);
    safety_->refreshRover();
}

void RoverController::left(uint8_t speed) {
    if (driver_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
        stop();
        return;
    }
    speed_ = constrainedSpeed(speed);
    direction_ = DriveDirection::Left;
    driver_->left(speed_);
    safety_->refreshRover();
}

void RoverController::right(uint8_t speed) {
    if (driver_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
        stop();
        return;
    }
    speed_ = constrainedSpeed(speed);
    direction_ = DriveDirection::Right;
    driver_->right(speed_);
    safety_->refreshRover();
}

void RoverController::setSpeed(uint8_t speed) {
    speed_ = constrainedSpeed(speed);
    if (direction_ != DriveDirection::Stop &&
        safety_ != nullptr && !safety_->isEmergencyStopped() && driver_ != nullptr) {
        driver_->setSpeed(speed_);
    }
}

uint8_t RoverController::getSpeed() const {
    return speed_;
}

DriveDirection RoverController::getDirection() const {
    return direction_;
}

uint8_t RoverController::constrainedSpeed(uint8_t speed) const {
    if (speed < config::rover::speed_minimum) {
        return config::rover::speed_minimum;
    }
    if (speed > config::rover::speed_maximum) {
        return config::rover::speed_maximum;
    }
    return speed;
}

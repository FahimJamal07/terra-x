#include "tb6612.h"

#include <Arduino.h>

#include "../../config.h"

void TB6612::begin() {
    pinMode(config::actuator::in3, OUTPUT);
    pinMode(config::actuator::in4, OUTPUT);
    pinMode(config::actuator::enb, OUTPUT);
    stop();
}

void TB6612::update() {
}

void TB6612::up(uint8_t speed) {
    digitalWrite(config::actuator::in3, LOW);
    digitalWrite(config::actuator::in4, HIGH);
    setSpeed(speed);
}

void TB6612::down(uint8_t speed) {
    digitalWrite(config::actuator::in3, HIGH);
    digitalWrite(config::actuator::in4, LOW);
    setSpeed(speed);
}

void TB6612::stop() {
    analogWrite(config::actuator::enb, 0);
    digitalWrite(config::actuator::in3, LOW);
    digitalWrite(config::actuator::in4, LOW);
    speed_ = 0;
}

void TB6612::setSpeed(uint8_t speed) {
    speed_ = constrainedSpeed(speed);
    analogWrite(config::actuator::enb, speed_);
}

uint8_t TB6612::constrainedSpeed(uint8_t speed) const {
    if (speed == 0) {
        return 0;
    }
    return constrain(speed, config::rover::speed_minimum, config::rover::speed_maximum);
}

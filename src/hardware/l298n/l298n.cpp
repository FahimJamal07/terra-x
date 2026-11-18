#include "l298n.h"

#include <Arduino.h>

#include "../../config.h"

void L298N::begin() {
    pinMode(config::rover::ena, OUTPUT);
    pinMode(config::rover::in1, OUTPUT);
    pinMode(config::rover::in2, OUTPUT);
    pinMode(config::rover::in3, OUTPUT);
    pinMode(config::rover::in4, OUTPUT);
    pinMode(config::rover::enb, OUTPUT);
    stop();
}

void L298N::update() {
}

void L298N::forward(uint8_t speed) {
    setDirection(HIGH, LOW, HIGH, LOW);
    setSpeed(speed);
}

void L298N::reverse(uint8_t speed) {
    setDirection(LOW, HIGH, LOW, HIGH);
    setSpeed(speed);
}

void L298N::left(uint8_t speed) {
    setDirection(LOW, HIGH, HIGH, LOW);
    setSpeed(speed);
}

void L298N::right(uint8_t speed) {
    setDirection(HIGH, LOW, LOW, HIGH);
    setSpeed(speed);
}

void L298N::stop() {
    analogWrite(config::rover::ena, 0);
    analogWrite(config::rover::enb, 0);
    digitalWrite(config::rover::in1, LOW);
    digitalWrite(config::rover::in2, LOW);
    digitalWrite(config::rover::in3, LOW);
    digitalWrite(config::rover::in4, LOW);
    speed_ = 0;
}

void L298N::setSpeed(uint8_t speed) {
    speed_ = constrainedSpeed(speed);
    analogWrite(config::rover::ena, speed_);
    analogWrite(config::rover::enb, speed_);
}

uint8_t L298N::constrainedSpeed(uint8_t speed) const {
    if (speed == 0) {
        return 0;
    }
    return constrain(speed, config::rover::speed_minimum, config::rover::speed_maximum);
}

void L298N::setDirection(bool leftFirst, bool leftSecond, bool rightFirst, bool rightSecond) {
    digitalWrite(config::rover::in1, leftFirst);
    digitalWrite(config::rover::in2, leftSecond);
    digitalWrite(config::rover::in3, rightFirst);
    digitalWrite(config::rover::in4, rightSecond);
}

#include "rs485.h"

#include <Arduino.h>

#include "../../config.h"

void SoilRs485::begin() {
    pinMode(config::soil::de_re, OUTPUT);
    digitalWrite(config::soil::de_re, LOW);
    serial_.begin(config::soil::baud, SERIAL_8N1, config::soil::uart_rx, config::soil::uart_tx);
    serial_.setTimeout(0);
    data_ = SoilData{};
    Serial.println("SOIL: UART CONFIG");
    Serial.print("SOIL: RX GPIO = ");
    Serial.println(config::soil::uart_rx);
    Serial.print("SOIL: TX GPIO = ");
    Serial.println(config::soil::uart_tx);
    Serial.print("SOIL: BAUD = ");
    Serial.println(config::soil::baud);
    Serial.println("SOIL: UART FORMAT = 8N1");
}

void SoilRs485::update() {
}

int SoilRs485::available() {
    return serial_.available();
}

int SoilRs485::read() {
    return serial_.read();
}

size_t SoilRs485::read(uint8_t* buffer, size_t length) {
    return serial_.readBytes(buffer, length);
}

size_t SoilRs485::write(const uint8_t* buffer, size_t length) {
    digitalWrite(config::soil::de_re, HIGH);
    Serial.println("SOIL: MAX485 TX MODE");
    delayMicroseconds(50);
    const size_t written = serial_.write(buffer, length);
    serial_.flush();
    delayMicroseconds(50);
    digitalWrite(config::soil::de_re, LOW);
    Serial.println("SOIL: MAX485 RX MODE");
    return written;
}

void SoilRs485::clearInput() {
    while (serial_.available() > 0) {
        serial_.read();
    }
}

HardwareSerial& SoilRs485::serial() {
    return serial_;
}

SoilData SoilRs485::data() const {
    return data_;
}

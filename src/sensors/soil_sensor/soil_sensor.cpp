#include "soil_sensor.h"

#include <Arduino.h>
#include <string.h>

#include "../../config.h"

namespace {

constexpr uint32_t request_interval_ms = 1000;
constexpr uint32_t response_timeout_ms = 500;
constexpr size_t response_capacity = 256;
constexpr size_t modbus_response_length = 13;

void printHex(const uint8_t* bytes, size_t length) {
    for (size_t index = 0; index < length; ++index) {
        if (index > 0) {
            Serial.print(' ');
        }
        if (bytes[index] < 0x10) {
            Serial.print('0');
        }
        Serial.print(bytes[index], HEX);
    }
    Serial.println();
}

uint16_t modbusCrc(const uint8_t* bytes, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t index = 0; index < length; ++index) {
        crc ^= bytes[index];
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 1U) != 0 ? (crc >> 1) ^ 0xA001 : crc >> 1;
        }
    }
    return crc;
}

size_t findModbusResponse(const uint8_t* bytes, size_t length) {
    if (length < modbus_response_length) {
        return response_capacity;
    }
    for (size_t index = 0; index + modbus_response_length <= length; ++index) {
        if (bytes[index] == 0x01 && bytes[index + 1] == 0x03 && bytes[index + 2] == 0x08) {
            return index;
        }
    }
    return response_capacity;
}

uint16_t readRegister(const uint8_t* response, size_t index) {
    return static_cast<uint16_t>(response[index] << 8U) | response[index + 1];
}

}  // namespace

void SoilSensor::begin(SoilRs485& transport) {
    transport_ = &transport;
    data_ = SoilData{};
    status_ = "NO_RESPONSE";
    lastRequestAt_ = 0;
    responseStartedAt_ = 0;
    responseLength_ = 0;
    requested_ = false;
    requestInProgress_ = false;
}

void SoilSensor::update() {
    if (transport_ == nullptr) {
        return;
    }

    if (!requestInProgress_) {
        if (requested_ && millis() - lastRequestAt_ < request_interval_ms) {
            return;
        }
        startRequest();
    }

    while (transport_->available() > 0 && responseLength_ < responseCapacity) {
        const size_t index = responseLength_++;
        const uint32_t byteTime = millis();
        const int byteValue = transport_->read();
        response_[index] = static_cast<uint8_t>(byteValue);
        Serial.print("SOIL: RX BYTE [");
        Serial.print(index);
        Serial.print("] TIME=");
        Serial.print(byteTime);
        Serial.print(" VALUE=");
        if (response_[index] < 0x10) {
            Serial.print('0');
        }
        Serial.println(response_[index], HEX);
    }

    const bool responseComplete = findModbusResponse(response_, responseLength_) != responseCapacity;
    const bool responseTimedOut = millis() - responseStartedAt_ >= response_timeout_ms;
    if (responseComplete || responseTimedOut || responseLength_ == responseCapacity) {
        processResponse();
    }
}

void SoilSensor::deactivate() {
    if (!requestInProgress_) {
        return;
    }
    requestInProgress_ = false;
    responseLength_ = 0;
    transport_->clearInput();
}

void SoilSensor::startRequest() {
    static const uint8_t query[] = {
        config::soil::modbus_address, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};

    transport_->clearInput();
    Serial.print("SOIL: TX -> ");
    printHex(query, sizeof(query));
    Serial.print("SOIL: TX TIME BEFORE = ");
    Serial.println(millis());
    transport_->write(query, sizeof(query));
    Serial.print("SOIL: TX TIME AFTER = ");
    Serial.println(millis());

    responseLength_ = 0;
    responseStartedAt_ = millis();
    requestInProgress_ = true;
    requested_ = true;
    lastRequestAt_ = responseStartedAt_;
}

void SoilSensor::processResponse() {
    static const uint8_t query[] = {
        config::soil::modbus_address, 0x03, 0x00, 0x00, 0x00, 0x04, 0x44, 0x09};
    const size_t candidateOffset = findModbusResponse(response_, responseLength_);
    requestInProgress_ = false;

    Serial.print("SOIL: TOTAL RX BYTES = ");
    Serial.println(responseLength_);
    Serial.print("SOIL: COMPLETE RX HEX -> ");
    printHex(response_, responseLength_);

    data_.valid = false;
    if (responseLength_ == 0) {
        Serial.println("SOIL: NO RESPONSE");
        status_ = "NO_RESPONSE";
        return;
    }

    const bool isExactEcho = responseLength_ == sizeof(query) &&
                             memcmp(response_, query, sizeof(query)) == 0;
    if (isExactEcho) {
        Serial.println("SOIL: TX ECHO DETECTED");
        status_ = "TX_ECHO";
        return;
    }

    if (candidateOffset == responseCapacity) {
        Serial.println("SOIL: INVALID_RESPONSE");
        Serial.println("SOIL: INVALID MODBUS HEADER");
        status_ = "PARSE_ERROR";
        return;
    }

    const uint8_t* candidate = response_ + candidateOffset;
    Serial.println("SOIL: RESPONSE RECEIVED");
    const uint16_t receivedCrc = static_cast<uint16_t>(candidate[11]) |
                                 (static_cast<uint16_t>(candidate[12]) << 8);
    const bool crcValid = modbusCrc(candidate, 11) == receivedCrc;
    Serial.println(crcValid ? "SOIL: CRC = OK" : "SOIL: CRC = INVALID");
    if (!crcValid) {
        status_ = "CRC_ERROR";
        return;
    }

    const int16_t temperatureRaw = static_cast<int16_t>(readRegister(candidate, 5));
    data_.moisture = static_cast<float>(readRegister(candidate, 3)) / 10.0F;
    data_.temperature = static_cast<float>(temperatureRaw) / 10.0F;
    data_.conductivity = static_cast<float>(readRegister(candidate, 7));
    data_.pH = static_cast<float>(readRegister(candidate, 9)) / 10.0F;
    data_.timestamp = millis();
    data_.valid = true;
    status_ = "OK";
}

SoilData SoilSensor::getData() const {
    return data_;
}

SoilData SoilSensor::data() const {
    return getData();
}

bool SoilSensor::isHealthy() const {
    return data_.valid && millis() - data_.timestamp <= config::sensors::soil_stale_timeout_ms;
}

const char* SoilSensor::status() const {
    return status_;
}

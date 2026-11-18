#include "sampling_controller.h"

#include <Arduino.h>
#include <string.h>

#include "../config.h"

RoverController* roverController_ = nullptr;
ActuatorController* actuatorController_ = nullptr;
SoilSensor* soilSensor_ = nullptr;
Safety* safety_ = nullptr;
SampleRecord latestSample_;
SamplingState state_ = SamplingState::IDLE;
uint32_t stateStartedAt_ = 0;
bool hasLatestSample_ = false;
bool measurementValid_ = false;

void SamplingController::begin(RoverController& roverController, ActuatorController& actuatorController,
							   SoilSensor& soilSensor, Safety& safety) {
	roverController_ = &roverController;
	actuatorController_ = &actuatorController;
	soilSensor_ = &soilSensor;
	safety_ = &safety;
	state_ = SamplingState::IDLE;
	latestSample_ = SampleRecord{};
	hasLatestSample_ = false;
	measurementValid_ = false;
}

void SamplingController::update() {
	if (!isActive()) {
		return;
	}
	if (roverController_ == nullptr || actuatorController_ == nullptr ||
		soilSensor_ == nullptr || safety_ == nullptr || safety_->isEmergencyStopped()) {
		fail();
		return;
	}

	roverController_->stop();
	const uint32_t elapsed = millis() - stateStartedAt_;
	switch (state_) {
		case SamplingState::LOWERING:
			actuatorController_->down();
			if (elapsed >= config::sampling::lowering_time_ms) {
				state_ = SamplingState::SETTLING;
				stateStartedAt_ = millis();
			}
			break;
		case SamplingState::SETTLING:
			actuatorController_->down();
			if (elapsed >= config::sampling::settling_time_ms) {
				state_ = SamplingState::READING;
				stateStartedAt_ = millis();
			}
			break;
		case SamplingState::READING:
			stopActuator();
			recordSoilMeasurement();
			state_ = SamplingState::RAISING;
			stateStartedAt_ = millis();
			break;
		case SamplingState::RAISING:
			actuatorController_->up();
			if (elapsed >= config::sampling::raising_time_ms) {
				stopActuator();
				state_ = measurementValid_ ? SamplingState::COMPLETE : SamplingState::FAILED;
			}
			break;
		default:
			break;
	}
}

bool SamplingController::startSample(const char* waypointId, double latitude, double longitude) {
	if (!config::soil::enabled || isActive() || waypointId == nullptr ||
		strlen(waypointId) >= sizeof(latestSample_.waypointId) ||
		latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
		return false;
	}
	latestSample_ = SampleRecord{};
	strncpy(latestSample_.waypointId, waypointId, sizeof(latestSample_.waypointId) - 1);
	latestSample_.latitude = latitude;
	latestSample_.longitude = longitude;
	state_ = SamplingState::LOWERING;
	stateStartedAt_ = millis();
	measurementValid_ = false;
	hasLatestSample_ = false;
	return true;
}

const SampleRecord& SamplingController::getLatestSample() const {
	return latestSample_;
}

bool SamplingController::hasLatestSample() const {
	return hasLatestSample_;
}

SamplingState SamplingController::getState() const {
	return state_;
}

bool SamplingController::isActive() const {
	return state_ == SamplingState::LOWERING || state_ == SamplingState::SETTLING ||
		   state_ == SamplingState::READING || state_ == SamplingState::RAISING;
}

void SamplingController::fail() {
	measurementValid_ = false;
	if (roverController_ != nullptr) {
		roverController_->stop();
	}
	stopActuator();
	state_ = SamplingState::FAILED;
	latestSample_.timestamp = millis();
	latestSample_.valid = false;
	hasLatestSample_ = true;
}

void SamplingController::recordSoilMeasurement() {
	const SoilData soil = soilSensor_->getData();
	latestSample_.moisture = soil.moisture;
	latestSample_.temperature = soil.temperature;
	latestSample_.conductivity = soil.conductivity;
	latestSample_.pH = soil.pH;
	latestSample_.timestamp = soil.timestamp != 0 ? soil.timestamp : millis();
	measurementValid_ = soil.valid && soilSensor_->isHealthy();
	latestSample_.valid = measurementValid_;
	hasLatestSample_ = true;
}

void SamplingController::stopActuator() {
	if (actuatorController_ != nullptr) {
		actuatorController_->stop();
	}
}

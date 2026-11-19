#include "system.h"

#include <Arduino.h>

#include "../config.h"

void System::begin() {
    l298n_.begin();
    tb6612_.begin();
    gpsHardware_.begin();
    if (config::soil::enabled) {
        soilRs485_.begin();
    }

    safety_.begin(l298n_, tb6612_);
    roverController_.begin(l298n_, safety_);
    actuatorController_.begin(tb6612_, safety_);
    gpsSensor_.begin(gpsHardware_);
    soilSensor_.begin(soilRs485_);
    navigator_.begin(gpsSensor_, roverController_);
    waypointManager_.begin();
    mission_.begin();
    samplingController_.begin(roverController_, actuatorController_, soilSensor_, safety_);
    api_.begin(webServer_, *this);
    dashboard_.begin(webServer_);
    webServer_.begin();

    SystemStatus status;
    status.roverSpeed = 200;
    state_.setStatus(status);
}

void System::update() {
    l298n_.update();
    tb6612_.update();
    gpsHardware_.update();
    if (config::soil::enabled) {
        soilRs485_.update();
    }
    safety_.update();
    roverController_.update();
    actuatorController_.update();
    gpsSensor_.update();
    if (config::soil::enabled) {
        if (actuatorController_.getState() == ActuatorState::Down) {
            soilSensor_.activate();
            soilSensor_.update();
        } else {
            soilSensor_.deactivate();
        }
    }
    samplingController_.update();
    if (!samplingController_.isActive()) {
        waypointManager_.update();
        if (navigator_.hasArrived() && waypointManager_.advanceWaypoint()) {
            Waypoint nextWaypoint;
            if (waypointManager_.getCurrentWaypoint(nextWaypoint)) {
                navigator_.setTarget(nextWaypoint);
            }
        }
        Waypoint currentWaypoint;
        const bool hasCurrentWaypoint = waypointManager_.getCurrentWaypoint(currentWaypoint);
        if (!navigator_.hasTarget() && hasCurrentWaypoint) {
            navigator_.setTarget(currentWaypoint);
        } else if (navigator_.hasTarget() && !hasCurrentWaypoint &&
                   !waypointManager_.isMissionComplete()) {
            navigator_.clearTarget();
        }
        navigator_.update();
    }
    SystemStatus status = state_.status();
    status.gpsStatus = gpsSensor_.isHealthy();
    status.soilStatus = soilSensor_.isHealthy();
    status.driveDirection = roverController_.getDirection();
    status.actuatorState = actuatorController_.getState();
    status.roverSpeed = roverController_.getSpeed();
    status.emergencyStop = safety_.isEmergencyStopped();
    status.navigationState = navigator_.getState();
    status.operatingMode = status.emergencyStop || status.navigationState == NavigationState::ERROR
                               ? OperatingMode::Emergency
                               : navigator_.hasTarget() ? OperatingMode::Autonomous : OperatingMode::Manual;
    state_.setStatus(status);
    mission_.update();
    webServer_.update();
    api_.update();
    dashboard_.update();
}

void System::demoUpdate() {
    update();
}

const State& System::state() const {
    return state_;
}

RoverController& System::roverController() {
    return roverController_;
}

ActuatorController& System::actuatorController() {
    return actuatorController_;
}

Safety& System::safety() {
    return safety_;
}

const GPSSensor& System::gpsSensor() const {
    return gpsSensor_;
}

const SoilSensor& System::soilSensor() const {
    return soilSensor_;
}

WaypointManager& System::waypointManager() {
    return waypointManager_;
}

const WaypointManager& System::waypointManager() const {
    return waypointManager_;
}

NavigationController& System::navigationController() {
    return navigator_;
}

const NavigationController& System::navigationController() const {
	return navigator_;
}

SamplingController& System::samplingController() {
    return samplingController_;
}

const SamplingController& System::samplingController() const {
    return samplingController_;
}

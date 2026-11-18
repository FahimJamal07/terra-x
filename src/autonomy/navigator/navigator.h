#pragma once

#include "../waypoint/waypoint.h"
#include "../../control/rover_controller/rover_controller.h"
#include "../../sensors/gps_sensor/gps_sensor.h"
#include "../../types.h"

class NavigationController {
public:
    void begin(const GPSSensor& gpsSensor, RoverController& roverController);
    void update();

    void setTarget(const Waypoint& waypoint);
    void clearTarget();
    bool hasTarget() const;
    bool hasArrived() const;
    double getDistance() const;
    double getBearing() const;
    NavigationState getState() const;

private:
    static double distanceBetween(const GPSData& current, const Waypoint& target);
    static double bearingBetween(const GPSData& current, const Waypoint& target);
    bool gpsIsUsable(const GPSData& data) const;
    void stop();

    const GPSSensor* gpsSensor_ = nullptr;
    RoverController* roverController_ = nullptr;
    Waypoint target_;
    bool targetSet_ = false;
    double distance_ = 0.0;
    double bearing_ = 0.0;
    NavigationState state_ = NavigationState::IDLE;
};

using Navigator = NavigationController;

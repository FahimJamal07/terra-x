#pragma once

#include "../autonomy/mission/mission.h"
#include "../autonomy/navigator/navigator.h"
#include "../autonomy/waypoint/waypoint.h"
#include "../communications/web/api.h"
#include "../communications/web/web_server.h"
#include "../control/actuator_controller/actuator_controller.h"
#include "../control/rover_controller/rover_controller.h"
#include "../hardware/gps/gps.h"
#include "../hardware/l298n/l298n.h"
#include "../hardware/rs485/rs485.h"
#include "../hardware/tb6612/tb6612.h"
#include "../safety/safety.h"
#include "../sampling/sampling_controller.h"
#include "../sensors/gps_sensor/gps_sensor.h"
#include "../sensors/soil_sensor/soil_sensor.h"
#include "../ui/dashboard/dashboard.h"
#include "state.h"

class System {
public:
    void begin();
    void update();
    void demoUpdate();

    const State& state() const;
    RoverController& roverController();
    ActuatorController& actuatorController();
    Safety& safety();
    const GPSSensor& gpsSensor() const;
    const SoilSensor& soilSensor() const;
    WaypointManager& waypointManager();
    const WaypointManager& waypointManager() const;
    NavigationController& navigationController();
    const NavigationController& navigationController() const;
    SamplingController& samplingController();
    const SamplingController& samplingController() const;

private:
    State state_;
    L298N l298n_;
    TB6612 tb6612_;
    GPSHardware gpsHardware_;
    SoilRs485 soilRs485_;
    GPSSensor gpsSensor_;
    SoilSensor soilSensor_;
    RoverController roverController_;
    ActuatorController actuatorController_;
    Safety safety_;
    WaypointManager waypointManager_;
    Navigator navigator_;
    Mission mission_;
    SamplingController samplingController_;
    WebService webServer_;
    Api api_;
    Dashboard dashboard_;
};

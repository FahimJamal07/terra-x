#pragma once

#include <stdint.h>

#include "../control/actuator_controller/actuator_controller.h"
#include "../control/rover_controller/rover_controller.h"
#include "../autonomy/waypoint/waypoint.h"
#include "../sensors/soil_sensor/soil_sensor.h"
#include "../types.h"

struct SampleRecord {
    char waypointId[Waypoint::sampleIdCapacity] = {};
    double latitude = 0.0;
    double longitude = 0.0;
    uint32_t timestamp = 0;
    float moisture = 0.0F;
    float temperature = 0.0F;
    float conductivity = 0.0F;
    float pH = 0.0F;
    bool valid = false;
};

class SamplingController {
public:
    void begin(RoverController& roverController, ActuatorController& actuatorController,
               SoilSensor& soilSensor, Safety& safety);
    void update();
    bool startSample(const char* waypointId, double latitude, double longitude);
    const SampleRecord& getLatestSample() const;
    bool hasLatestSample() const;
    SamplingState getState() const;
    bool isActive() const;

private:
    void fail();
    void recordSoilMeasurement();
    void stopActuator();
};
extern RoverController* roverController_;
extern ActuatorController* actuatorController_;
extern SoilSensor* soilSensor_;
extern Safety* safety_;
extern SampleRecord latestSample_;
extern SamplingState state_;
extern uint32_t stateStartedAt_;
extern bool hasLatestSample_;
extern bool measurementValid_;

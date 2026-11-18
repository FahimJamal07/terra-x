#pragma once

#include <stddef.h>
#include <stdint.h>

struct Waypoint {
    static constexpr size_t sampleIdCapacity = 32;
    double latitude = 0.0;
    double longitude = 0.0;
    double arrivalRadius = 2.0;
    char sampleId[sampleIdCapacity] = {};
};

class WaypointManager {
public:
    void begin();
    void update();
    bool addWaypoint(const Waypoint& waypoint);
    bool removeWaypoint(size_t index);
    void clearWaypoints();
    bool getWaypoint(size_t index, Waypoint& waypoint) const;
    size_t getWaypointCount() const;
    bool setCurrentWaypoint(size_t index);
    bool advanceWaypoint();
    bool getCurrentWaypoint(Waypoint& waypoint) const;
    bool isMissionComplete() const;
    size_t getCurrentWaypointIndex() const;

    static constexpr size_t maximumWaypoints = 16;

private:
    bool validWaypoint(const Waypoint& waypoint) const;

    Waypoint waypoints_[maximumWaypoints];
    size_t waypointCount_ = 0;
    size_t currentWaypoint_ = 0;
};

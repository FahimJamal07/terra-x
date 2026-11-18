#include "waypoint.h"

#include <math.h>

#include "../../config.h"

void WaypointManager::begin() {
	clearWaypoints();
}

void WaypointManager::update() {
}

bool WaypointManager::addWaypoint(const Waypoint& waypoint) {
	if (waypointCount_ >= maximumWaypoints || !validWaypoint(waypoint)) {
		return false;
	}
	waypoints_[waypointCount_] = waypoint;
	if (waypoints_[waypointCount_].arrivalRadius <= 0.0) {
		waypoints_[waypointCount_].arrivalRadius = config::navigation::arrival_radius_m;
	}
	waypoints_[waypointCount_].sampleId[sizeof(waypoints_[waypointCount_].sampleId) - 1] = '\0';
	++waypointCount_;
	return true;
}

bool WaypointManager::removeWaypoint(size_t index) {
	if (index >= waypointCount_) {
		return false;
	}
	for (size_t current = index; current + 1 < waypointCount_; ++current) {
		waypoints_[current] = waypoints_[current + 1];
	}
	--waypointCount_;
	if (waypointCount_ == 0) {
		currentWaypoint_ = 0;
	} else if (currentWaypoint_ > index) {
		--currentWaypoint_;
	} else if (currentWaypoint_ >= waypointCount_) {
		currentWaypoint_ = waypointCount_ - 1;
	}
	return true;
}

void WaypointManager::clearWaypoints() {
	waypointCount_ = 0;
	currentWaypoint_ = 0;
}

bool WaypointManager::getWaypoint(size_t index, Waypoint& waypoint) const {
	if (index >= waypointCount_) {
		return false;
	}
	waypoint = waypoints_[index];
	return true;
}

size_t WaypointManager::getWaypointCount() const {
	return waypointCount_;
}

bool WaypointManager::setCurrentWaypoint(size_t index) {
	if (index >= waypointCount_) {
		return false;
	}
	currentWaypoint_ = index;
	return true;
}

bool WaypointManager::advanceWaypoint() {
	if (currentWaypoint_ + 1 >= waypointCount_) {
		currentWaypoint_ = waypointCount_;
		return false;
	}
	++currentWaypoint_;
	return true;
}

bool WaypointManager::getCurrentWaypoint(Waypoint& waypoint) const {
	return currentWaypoint_ < waypointCount_ && getWaypoint(currentWaypoint_, waypoint);
}

bool WaypointManager::isMissionComplete() const {
	return waypointCount_ > 0 && currentWaypoint_ >= waypointCount_;
}

size_t WaypointManager::getCurrentWaypointIndex() const {
	return currentWaypoint_;
}

bool WaypointManager::validWaypoint(const Waypoint& waypoint) const {
	return isfinite(waypoint.latitude) && isfinite(waypoint.longitude) &&
		   waypoint.latitude >= -90.0 && waypoint.latitude <= 90.0 &&
		   waypoint.longitude >= -180.0 && waypoint.longitude <= 180.0 &&
		   waypoint.arrivalRadius >= 0.0 && waypoint.arrivalRadius <= 10000.0;
}

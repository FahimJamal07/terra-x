#include "navigator.h"

#include <Arduino.h>
#include <math.h>

#include "../../config.h"

namespace {
constexpr double earthRadiusMeters = 6371000.0;
constexpr double degreesToRadians = 0.017453292519943295;
constexpr double radiansToDegrees = 57.29577951308232;

double normalizeDegrees(double degrees) {
	while (degrees < 0.0) {
		degrees += 360.0;
	}
	while (degrees >= 360.0) {
		degrees -= 360.0;
	}
	return degrees;
}
}  // namespace

void NavigationController::begin(const GPSSensor& gpsSensor, RoverController& roverController) {
	gpsSensor_ = &gpsSensor;
	roverController_ = &roverController;
	clearTarget();
}

void NavigationController::update() {
	if (!targetSet_) {
		state_ = NavigationState::IDLE;
		return;
	}
	if (gpsSensor_ == nullptr || roverController_ == nullptr) {
		state_ = NavigationState::ERROR;
		stop();
		return;
	}

	const GPSData gps = gpsSensor_->getData();
	if (!gpsIsUsable(gps)) {
		state_ = NavigationState::GPS_LOST;
		stop();
		return;
	}
	distance_ = distanceBetween(gps, target_);
	bearing_ = bearingBetween(gps, target_);

	const double arrivalRadius = target_.arrivalRadius > 0.0 ? target_.arrivalRadius : config::navigation::arrival_radius_m;
	if (distance_ <= arrivalRadius) {
		state_ = NavigationState::ARRIVED;
		stop();
		return;
	}

	state_ = NavigationState::MOVING;
	roverController_->forward(config::navigation::minimum_speed);
}

void NavigationController::setTarget(const Waypoint& waypoint) {
	if (!isfinite(waypoint.latitude) || !isfinite(waypoint.longitude) ||
		waypoint.latitude < -90.0 || waypoint.latitude > 90.0 ||
		waypoint.longitude < -180.0 || waypoint.longitude > 180.0) {
		targetSet_ = false;
		state_ = NavigationState::ERROR;
		stop();
		return;
	}
	target_ = waypoint;
	if (target_.arrivalRadius <= 0.0) {
		target_.arrivalRadius = config::navigation::arrival_radius_m;
	}
	targetSet_ = true;
	state_ = NavigationState::IDLE;
}

void NavigationController::clearTarget() {
	targetSet_ = false;
	distance_ = 0.0;
	bearing_ = 0.0;
	state_ = NavigationState::IDLE;
	stop();
}

bool NavigationController::hasTarget() const {
	return targetSet_;
}

bool NavigationController::hasArrived() const {
	return state_ == NavigationState::ARRIVED;
}

double NavigationController::getDistance() const {
	return distance_;
}

double NavigationController::getBearing() const {
	return bearing_;
}

NavigationState NavigationController::getState() const {
	return state_;
}

double NavigationController::distanceBetween(const GPSData& current, const Waypoint& target) {
	const double latitude1 = current.latitude * degreesToRadians;
	const double latitude2 = target.latitude * degreesToRadians;
	const double deltaLatitude = (target.latitude - current.latitude) * degreesToRadians;
	const double deltaLongitude = (target.longitude - current.longitude) * degreesToRadians;
	const double sineLatitude = sin(deltaLatitude / 2.0);
	const double sineLongitude = sin(deltaLongitude / 2.0);
	const double haversine = sineLatitude * sineLatitude +
							 cos(latitude1) * cos(latitude2) * sineLongitude * sineLongitude;
	return earthRadiusMeters * 2.0 * atan2(sqrt(haversine), sqrt(1.0 - haversine));
}

double NavigationController::bearingBetween(const GPSData& current, const Waypoint& target) {
	const double latitude1 = current.latitude * degreesToRadians;
	const double latitude2 = target.latitude * degreesToRadians;
	const double deltaLongitude = (target.longitude - current.longitude) * degreesToRadians;
	const double y = sin(deltaLongitude) * cos(latitude2);
	const double x = cos(latitude1) * sin(latitude2) -
					 sin(latitude1) * cos(latitude2) * cos(deltaLongitude);
	return normalizeDegrees(atan2(y, x) * radiansToDegrees);
}

bool NavigationController::gpsIsUsable(const GPSData& data) const {
	return data.fixValid && data.timestamp != 0 &&
		   millis() - data.timestamp <= config::navigation::gps_stale_timeout_ms;
}

void NavigationController::stop() {
	if (roverController_ != nullptr) {
		roverController_->stop();
	}
}

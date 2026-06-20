#include "GeoLocation.h"

#include <utility>

GeoLocation::GeoLocation() = default;

Vector3d GeoLocation::geo2cart(Vector2d location) {
//    double latitude = location[0];
//    double longitude = location[1];
//    double theta = (90 - latitude) * PI / 180;
//    double phi = longitude * PI / 180;
//    Vector3d ret;
//    ret << sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta);
//    return ret;
    return rfdfgeo::geo2cart(std::move(location));
}

Vector2d GeoLocation::cart2geo(Vector3d x) {
//    x.normalize();
//    double phi = PI/2;
//    if (x[0] > 0) phi = atan(x[1]/x[0]);
//    if (x[0] < 0) phi = atan(x[1]/x[0]) + PI;
//    double theta = acos(x[2]);
//    double latitude = 90 - (theta * 180 / PI);
//    double longitude = phi * 180 / PI;
//    Vector2d ret;
//    ret << latitude, longitude;
//    return ret;
    return rfdfgeo::cart2geo(std::move(x));
}

double GeoLocation::distanceTo(const GeoLocation& goLocation) const {
    double p = 0.017453292519943295;    // Math.PI / 180
    double a = 0.5 - cos((goLocation.latitude - this->latitude) * p)/2 +
            cos(this->latitude * p) * cos(goLocation.latitude * p) *
            (1 - cos((goLocation.longitude - this->longitude) * p))/2;
    return 12742 * asin(sqrt(a)); // 2 * R; R = 6371 km    return 0;
}

GeoLocation GeoLocation::meanWith(const GeoLocation &geoLocation) {
    GeoLocation ret(this->cart2geo((this->coordinatesCartesian + geoLocation.coordinatesCartesian)));
    return ret;
}

std::string GeoLocation::toString() const {
    char ss[1000];
    snprintf(ss, 1000, "lat: %f, lon: %f", this->latitude, this->longitude);
    return std::string(ss);
};

std::string GeoLocation::toJSON() const {
    return "{"
           ",\"lat\":" + to_string(this->latitude) +
           ",\"lon\":" + to_string(this->longitude) +
           "}";
};

std::ostream& operator<<(std::ostream &strm, const GeoLocation &geoLocation) {
    return strm << "(" << geoLocation.latitude << ", " << geoLocation.longitude << ")";
}

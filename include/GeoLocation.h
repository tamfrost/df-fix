#ifndef UNTITLED_GEOLOCATION_H
#define UNTITLED_GEOLOCATION_H

#include "geolib.h"

class GeoLocation {
public:
    double latitude{}, longitude{};
    Vector2d coordinatesGeographic;
    Vector3d coordinatesCartesian;

    GeoLocation();

    explicit GeoLocation(Vector2d latlon) {
        latitude = latlon(0);
        longitude = latlon(1);
        coordinatesGeographic << latlon;
        coordinatesCartesian = GeoLocation::geo2cart(latlon);
    };

    GeoLocation(double lat, double lon) {
        latitude = lat;
        longitude = lon;
        coordinatesGeographic << lat, lon;
        coordinatesCartesian = GeoLocation::geo2cart(coordinatesGeographic);
    };

    static Vector3d geo2cart(Vector2d location);
    static Vector2d cart2geo(Vector3d x);
    double distanceTo(const GeoLocation& geoLocation) const;
    GeoLocation meanWith(const GeoLocation& geoLocation);

    std::string toString() const;
    std::string toJSON() const;
    friend std::ostream& operator<<(std::ostream &strm, const GeoLocation &geoLocation);

};


#endif //UNTITLED_GEOLOCATION_H

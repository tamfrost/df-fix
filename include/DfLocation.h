#ifndef WEBASSEMBLY_DFLOCATION_H
#define WEBASSEMBLY_DFLOCATION_H

#include "DirectionCollection.h"
#include "GeoLocation.h"

/**
 *
 */
class DfLocation: public DirectionCollection, public GeoLocation {
private:
    typedef DirectionCollection Super;
public:
    std::string name = "not set";
    DfLocation(double lat, double lon, std::string name): GeoLocation(lat, lon) {
        this->name = std::move(name);
    }
    Direction calculateMeanDirection(std::string id) override;
    std::string toString();
    std::string directionsToJSON();
    friend std::ostream& operator<<(std::ostream &strm, const DfLocation &dfdfLocation);
    friend bool operator== (const DfLocation& location1, const DfLocation& location2);
    friend bool operator< (const DfLocation& location1, const DfLocation& location2);
};


#endif //WEBASSEMBLY_DFLOCATION_H

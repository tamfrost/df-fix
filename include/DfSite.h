#ifndef WEBASSEMBLY_DFSITE_H
#define WEBASSEMBLY_DFSITE_H

#include "DirectionCollection.h"
#include "GeoLocation.h"

/**
 *
 */
class DfSite: public DirectionCollection, public GeoLocation {
private:
    typedef DirectionCollection Super;
public:
    std::string name = "not set";
    DfSite(double lat, double lon, std::string name): GeoLocation(lat, lon) {
        this->name = std::move(name);
    }
    Direction calculateMeanDirection(std::string id) override;
    std::string toString();
    std::string directionsToJSON();
    friend std::ostream& operator<<(std::ostream &strm, const DfSite &dfdfSite);
    friend bool operator== (const DfSite& site1, const DfSite& site2);
    friend bool operator< (const DfSite& site1, const DfSite& site2);
};


#endif //WEBASSEMBLY_DFSITE_H

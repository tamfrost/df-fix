#ifndef WEBASSEMBLY_DIRECTION_H
#define WEBASSEMBLY_DIRECTION_H

#include <utility>

#include "geolib.h"
#include "GeoLocation.h"

class DirectionCross;
class DfLocation;

/**
 *
 */
class Direction: public GeoLocation {
private:
public:
    std::string name;
    int id = -1;
    bool included = false;
    vector<Direction*> primarySharers;
    double value{}, sigma{},  originalSigma{};
    DfLocation* dfLocation{};

    Direction() = default;
    Direction(DfLocation* location, double value, double sigma, std::string id);
    Direction(double longitude, double latitude, double value, double sigma, std::string id);
    Direction(double value, double sigma, std::string id);

    void setDfLocation(DfLocation* dfLocation);
    DirectionCross cross(Direction* direction);
    void getLines(int nPoints, double distance, double *main, double *upper, double *lower);

    friend bool operator== (const Direction& direction1, const Direction& direction2);
    friend bool operator< (const Direction& direction1, const Direction& direction2);
    friend std::ostream& operator<<(std::ostream &strm, const Direction &direction);
    std::string toString();
    std::string toJSON();

    void setId(int id);
};

#endif //WEBASSEMBLY_DIRECTION_H

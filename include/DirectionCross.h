#ifndef WEBASSEMBLY_DIRECTIONCROSS_H
#define WEBASSEMBLY_DIRECTIONCROSS_H

#include "Direction.h"
#include "GeoLocation.h"

/**
 *
 */
class DirectionCross : public GeoLocation {
private:
public:
    bool included = false;
    bool includedStatistics = true;
    int primaryCrossForDirection1 = 2;
    int primaryCrossForDirection2 = 2;
    Direction *direction1{}, *direction2{};
    double distanceToLocation = std::numeric_limits<double>::max();
//    double longitude1{}, latitude1{}, longitude2{}, latitude2{};
    Vector3d crossVector;
    Vector2d crossLocation, crossLocation1, crossLocation2;
    
    DirectionCross() = default;
    DirectionCross(Direction* b1, Direction* b2);
    static Vector3d directionVector_(Vector2d locationLocation, double direction);
    static Vector3d geographic2cartesian_(Vector2d location);
    static Vector2d cartesian2geographic_(Vector3d x);
    static Vector3d directionPlaneNormal_(Vector3d directionVector, Vector2d location);
    bool containsDirection(Direction* direction, Direction* otherDirection) const;
    std::string toString();
    std::string toJSON();
    friend std::ostream& operator<<(std::ostream &strm, const DirectionCross &directionCross);
};


#endif //WEBASSEMBLY_DIRECTIONCROSS_H

#include "DirectionCross.h"
#include "DfLocation.h"

/**
 * calculate the cross between two directions (assuming a spherical earth)
 * @param b1
 * @param b2
 */
DirectionCross::DirectionCross(Direction* d1, Direction* d2) {

    direction1 = d1;
    direction2 = d2;

    //the coordinates of the df-locations
    Vector2d locationCoordinates1;
    Vector2d locationCoordinates2;
    locationCoordinates1 << d1->latitude/*replace1*/, d1->longitude/*replace1*/;
    locationCoordinates2 << d2->latitude/*replace1*/, d2->longitude/*replace1*/;

    //vectors pointing along the two directions
    Vector3d directionVector1 = directionVector_(locationCoordinates1, d1->value);
    Vector3d directionVector2 = directionVector_(locationCoordinates2, d2->value);

    //normal vectors to the planes in which the direction vectors lies that also goes through the earth center.
    //- the crosses lies on the earth surface in the intersection of these two planes
    //- there are always two crosses separated by 180 degrees.
    //- the primary cross for a direction is the first one encountered when moving along the earths surface in
    //  that direction.
    //- we are interested only in situations when both directions share the same primary cross
    Vector3d directionPlaneNormal1 = directionPlaneNormal_(directionVector1, locationCoordinates1);
    Vector3d directionPlaneNormal2 = directionPlaneNormal_(directionVector2, locationCoordinates2);

    //the vector pointing from the earth center towards the cross location
    crossVector = directionPlaneNormal2.cross(directionPlaneNormal1);
    crossVector.normalize();

    //make the vector point to the cross on the northern hemisphere
    if (crossVector[2] < 0) crossVector = -crossVector;

    //northern cross is primary to direction 1
    if (crossVector.dot(directionVector1) > 0) primaryCrossForDirection1 = 1;
    //northern cross is primary to direction 2
    if (crossVector.dot(directionVector2) > 0) primaryCrossForDirection2 = 1;

    //convert to geographic coordinates
    crossLocation1 = cartesian2geographic_(crossVector); //northern cross
    crossLocation2 = cartesian2geographic_(-crossVector); //southern cross

    //cross and both directions are flagged as included if they share the same primary cross
    //Note that all directions are initially excluded by default when created. If the direction
    //is a member of any cross where both directions agree on their primary cross it will be
    //flagged as included
    if (primaryCrossForDirection1 == primaryCrossForDirection2) {
        this->included = true;
        direction1->included = true;
        direction2->included = true;
        d1->primarySharers.push_back(d2);
        d2->primarySharers.push_back(d1);
        // std::cout << "PS: " << d1->primarySharers.size() << std::endl;
    }

    //set crossLocation to the coordinates of the primary cross
    crossLocation = primaryCrossForDirection1 == 1 ? crossLocation1 : crossLocation2;

    this->latitude = crossLocation[0];
    this->longitude = crossLocation[1];

    distanceToLocation = (this->distanceTo(*(direction1)) + this->distanceTo(*(direction2)))/2;
}

/**
 * Calculates a vector pointing in the direction of the source
 * @param locationCoordinates (lat,lon)
 * @param direction in degrees (north 0, south 180)
 * @return (x,y,z) vector in global cartesian coordinates
 */
Vector3d DirectionCross::directionVector_(Vector2d locationCoordinates, double direction) {
//    double latitude = locationCoordinates[0];
    double longitude = locationCoordinates[1];
    Vector3d localUpVector = geographic2cartesian_(locationCoordinates);  // pointing up
    Vector3d localEWVector;
    localEWVector << -sin(longitude * PI / 180), cos(longitude * PI / 180), 0.0; // pointing east
    Vector3d localNSVector = localUpVector.cross(localEWVector);  //pointing north
    Vector3d directionVector = sin(direction * PI / 180) * localEWVector + cos(direction * PI / 180) * localNSVector;
    return directionVector;
}

/**
 * Converts geographic coordinates to cartesian on a unit sphere. Depending on origin
 * this is a unit vector pointing either towards the location with the coordinates (origin at earth center),
 * or a unit vector pointing straight up in the sky (origin at the location)
 * @param location (lat,lon)
 * @return (x,y,z) vector in global cartesian coordinates
 */
Vector3d DirectionCross::geographic2cartesian_(Vector2d location) {
    double latitude = location[0];
    double longitude = location[1];
    double theta = (90 - latitude) * PI / 180;
    double phi = longitude * PI / 180;
    Vector3d ret;
    ret << sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta);
    return ret;
}

Vector2d DirectionCross::cartesian2geographic_(Vector3d x) {
    x.normalize();
    double phi = PI/2;
    if (x[0] != 0) phi = atan2(x[1], x[0]);
    double theta = acos(x[2]);
    double latitude = 90 - (theta * 180 / PI);
    double longitude = phi * 180 / PI;
    Vector2d ret;
    ret << latitude, longitude;
    return ret;
}

Vector3d DirectionCross::directionPlaneNormal_(Vector3d directionVector, Vector2d location) {
    Vector3d localUpVector = geographic2cartesian_(location);  // pointing up
    Vector3d directionPlaneNormal = geographic2cartesian_(location).cross(directionVector);
    return directionPlaneNormal;
}

std::string DirectionCross::toString() {
    char ss[1000];
    snprintf(ss, 1000, "PRIMARY CROSS: %i %i ... %s x %s ... (%f, %f) ... %s",
            this->primaryCrossForDirection1,
            this->primaryCrossForDirection2,
            this->direction1->name.c_str(),
            this->direction2->name.c_str(),
            this->crossLocation[0],
            this->crossLocation[1],
            this->included ? "INCLUDED" : ""
             );
    return std::string(ss);
};

std::string DirectionCross::toJSON() {
    return "{"
           "\"lat\":" + to_string(this->crossLocation[0]) +
           ",\"lon\":" + to_string(this->crossLocation[1]) +
           ",\"coordinates\":[[" +  to_string(this->crossLocation1[0]) + "," + to_string(this->crossLocation1[1]) + "]" + "," + "[" +  to_string(this->crossLocation2[0]) + "," + to_string(this->crossLocation2[1]) + "]]"
//           ",\"cross1Coordinates\":[" + to_string(crossLocation1(0)) + "," + to_string(crossLocation1(1)) + "]" +
//           ",\"cross2Coordinates\":[" + to_string(crossLocation2(0)) + "," + to_string(crossLocation2(1)) + "]" +
           ",\"primaryIndex\":[" + to_string(primaryCrossForDirection1) + "," + to_string(primaryCrossForDirection2) + "]" +
           ",\"included\":" + (this->included ? "true" : "false") +
            ",\"direction1\":{\"primaryCross\":" + to_string(primaryCrossForDirection1) + ",\"direction\":" + this->direction1->toJSON() + "}" +
            ",\"direction2\":{\"primaryCross\":" + to_string(primaryCrossForDirection2) + ",\"direction\":" + this->direction2->toJSON() + "}" +
           "}";
};

std::ostream& operator<<(std::ostream &strm, const DirectionCross &directionCross) {
//    char *ss;
//    sprintf(ss, " %s x %s ... ", directionCross.direction1.dfLocation.name.c_str(), directionCross.direction2.dfLocation.name.c_str());
    return strm <<
                "PRIMARY CROSS: " <<
                directionCross.direction1->name << "x" << directionCross.direction2->name << " ... " <<
                // directionCross.direction1->dfLocation->name << "x" << directionCross.direction2->dfLocation->name << " ... " <<
                directionCross.crossLocation.transpose() << " ... " <<
                directionCross.direction1->included << "," <<
                directionCross.direction2->included
            ;
}

bool DirectionCross::containsDirection(Direction *direction, Direction *otherDirection) const {
    if (direction->name == direction1->name) {
        otherDirection = direction2;
        return true;
    }
    if (direction->name == direction2->name) {
        otherDirection = direction1;
        return true;
    }
    return false;
}


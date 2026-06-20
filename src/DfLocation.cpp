#include "DfLocation.h"

Direction DfLocation::calculateMeanDirection(std::string id) {
    try {

        double maxDirectionDensityValue = Super::calculateDirectionDensity();

        //excluding outliers
        for(auto direction: *this) {
            if (abs(rfdfgeo::subtractDirections(maxDirectionDensityValue, direction->value)) > 45) {
                direction->included = false;
            }
        }
        //calculate again without the outliers
        Direction md = Super::calculateMeanDirection(id);

        md.setDfLocation(this);
        md.name = this->name + "_mean";
        md.latitude = this->latitude;
        md.longitude = this->longitude;

        return md;
    }
    catch (NoIncludedDirectionsException e) {
        throw e;
    }
}

std::string DfLocation::toString() {
    char ss[1000];
    snprintf(ss, 1000, "%s %i", this->name.c_str(), (int)this->size());
    return std::string(ss);
};

std::string DfLocation::directionsToJSON() {
this->meanDirection.name = this->name + "_mean";
this->meanDirection.latitude = this->latitude;
this->meanDirection.longitude = this->longitude;
return "{"
           "\"name\":\"" + this->name + "\"" +
           ",\"lat\":" + to_string(this->latitude) +
           ",\"lon\":" + to_string(this->longitude) +
           ",\"mean\":" + this->meanDirection.toJSON() +
           ",\"directions\":" + Super::directionsToJSON() +
           ",\"density\":" + Super::densityToJSON() +
           ",\"maxDensityDirection\":" + to_string(this->maxDirectionDensity.value) +
           "}";
};

std::ostream& operator<<(std::ostream &strm, const DfLocation &dfdfLocation) {
    return strm << dfdfLocation.name << ": " << dfdfLocation.size();
}

bool operator== (const DfLocation& location1, const DfLocation& location2) {
    return (location1.latitude == location2.latitude);
}

bool operator< (const DfLocation& location1, const DfLocation& location2) {
    return (location1.latitude < location2.latitude);
}



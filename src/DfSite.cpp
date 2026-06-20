#include "DfSite.h"

Direction DfSite::calculateMeanDirection(std::string id) {
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

        // md.setDfSite(this);
        md.name = this->name + "_mean";
        md.latitude = this->latitude;
        md.longitude = this->longitude;

        return md;
    }
    catch (NoIncludedDirectionsException e) {
        throw e;
    }
}

std::string DfSite::toString() {
    char ss[1000];
    snprintf(ss, 1000, "%s %i", this->name.c_str(), (int)this->size());
    return std::string(ss);
};

std::string DfSite::directionsToJSON() {
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

std::ostream& operator<<(std::ostream &strm, const DfSite &dfdfSite) {
    return strm << dfdfSite.name << ": " << dfdfSite.size();
}

bool operator== (const DfSite& site1, const DfSite& site2) {
    return (site1.latitude == site2.latitude);
}

bool operator< (const DfSite& site1, const DfSite& site2) {
    return (site1.latitude < site2.latitude);
}



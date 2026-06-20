#include "Bearing.h"
#include "DirectionCrossCollection.h"
#include "DfLocation.h"

Bearing::Bearing(DfLocation* location, double value, double sigma, std::string id): name(std::move(id)), value(value), sigma(sigma), originalSigma(sigma), dfLocation(location) {
    this->latitude = location->latitude;
    this->longitude = location->longitude;
};

Bearing::Bearing(double longitude, double latitude, double value, double sigma, std::string id): name(std::move(id)), value(value), sigma(sigma), originalSigma(sigma) {
    this->latitude = latitude;
    this->longitude = longitude;
};

Bearing::Bearing(double value, double sigma, std::string id): name(std::move(id)), value(value), sigma(sigma), originalSigma(sigma) {
};

void Bearing::setDfLocation(DfLocation* dfLocation) {
    Bearing::dfLocation = dfLocation;
}

// BearingCross Bearing::cross(Bearing* bearing){
//     BearingCross cross(this, bearing);
//     return cross;
// }

void Bearing::getLines(int nPoints, double distance, double* main, double* upper, double* lower) {

    const Geodesic& geod = Geodesic::WGS84();

    const GeodesicLine& mainLine = geod.Line(this->latitude/*replace1*/, this->longitude/*replace1*/, this->value, Geodesic::ALL);
    const GeodesicLine& upperLine = geod.Line(this->latitude/*replace1*/, this->longitude/*replace1*/, this->value + this->sigma, Geodesic::ALL);
    const GeodesicLine& lowerLine = geod.Line(this->latitude/*replace1*/, this->longitude/*replace1*/, this->value - this->sigma, Geodesic::ALL);

    for (int iPoint=0; iPoint < nPoints; iPoint++) {
        mainLine.Position(distance * iPoint, main[iPoint], main[nPoints + iPoint]);
        upperLine.Position(distance * iPoint, upper[iPoint], upper[nPoints + iPoint]);
        lowerLine.Position(distance * iPoint, lower[iPoint], lower[nPoints + iPoint]);
    }
}

std::string Bearing::toString() {
    char ss[1000];
    if (this->dfLocation) snprintf(ss, 1000, "%s (%s) %3.1f +- %.1f %s", this->name.c_str(), this->dfLocation->name.c_str(), this->value, this->sigma, this->included ? "INCLUDED" : "EXCLUDED");
    else snprintf(ss, 1000, "%s (%3.3f,%3.3f) %3.1f +- %.1f %i %s", this->name.c_str(), this->latitude, this->longitude, this->value, this->sigma, (int)(this->primarySharers).size(), this->included ? "INCLUDED" : "EXCLUDED");
    return std::string(ss);
}

// std::string Bearing::to_json() {
//     json jb;
//     jb["name"] = this->name;
//     jb["value"] = this->value;
//     jb["sigma"] = this->sigma;
//     jb["latitude"] = this->latitude;
//     jb["longitude"] = this->longitude;
//     jb["included"] = this->included;
//     jb["primarySharers"] = json::array();
//     for (auto ps : this->primarySharers) {
//         jb["primarySharers"].push_back(ps->id);
//     }
//     return jb.dump(); // Serialize the JSON object to a string
// }

std::string Bearing::toJSON() {
    std::string psJson = "[";
    for (auto ps : this->primarySharers) {
        // psJson += "\"" + ps->name + "\"";
        psJson += std::to_string(ps->id);
        if (ps != this->primarySharers.back()) psJson += ",";
    }
    psJson += "]";
    return "{"
            "\"name\":\"" + this->name + "\"" +
            ",\"value\":" + to_string(this->value) +
            ",\"sigma\":" + to_string(this->sigma) +
            ",\"included\":" + (this->included ? "true" : "false") +
            ",\"lat\":" + to_string(this->latitude/*replace1*/) +
            ",\"lon\":" + to_string(this->longitude/*replace1*/) +
            ",\"primarySharers\":" + psJson +
//            ",\"dfLocation\":" + this->dfLocation->toJSON() +
           "}";
}

std::ostream& operator<<(std::ostream &strm, const Bearing &bearing) {
    char ss[1000];
    snprintf(ss, 1000, "%s %s %3.1f +- %.1f %s", bearing.name.c_str(), bearing.dfLocation->name.c_str(), bearing.value, bearing.sigma, bearing.included ? "INCLUDED" : "EXCLUDED");
    return strm << ss;
}

bool operator== (const Bearing& bearing1, const Bearing& bearing2) {
    return (bearing1.name == bearing2.name);
}

bool operator< (const Bearing& bearing1, const Bearing& bearing2) {
    return (bearing1.name < bearing2.name);
}

void Bearing::setId(int id) {
    Bearing::id = id;
}


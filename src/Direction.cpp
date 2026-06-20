#include "Direction.h"
#include "DirectionCrossCollection.h"
#include "DfLocation.h"

Direction::Direction(DfLocation* location, double value, double sigma, std::string id): name(std::move(id)), value(value), sigma(sigma), originalSigma(sigma), dfLocation(location) {
    this->latitude = location->latitude;
    this->longitude = location->longitude;
};

Direction::Direction(double longitude, double latitude, double value, double sigma, std::string id): name(std::move(id)), value(value), sigma(sigma), originalSigma(sigma) {
    this->latitude = latitude;
    this->longitude = longitude;
};

Direction::Direction(double value, double sigma, std::string id): name(std::move(id)), value(value), sigma(sigma), originalSigma(sigma) {
};

void Direction::setDfLocation(DfLocation* dfLocation) {
    Direction::dfLocation = dfLocation;
}

DirectionCross Direction::cross(Direction* direction){
    DirectionCross cross(this, direction);
    return cross;
}

void Direction::getLines(int nPoints, double distance, double* main, double* upper, double* lower) {

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

std::string Direction::toString() {
    char ss[1000];
    if (this->dfLocation) snprintf(ss, 1000, "%s (%s) %3.1f +- %.1f %s", this->name.c_str(), this->dfLocation->name.c_str(), this->value, this->sigma, this->included ? "INCLUDED" : "EXCLUDED");
    else snprintf(ss, 1000, "%s (%3.3f,%3.3f) %3.1f +- %.1f %i %s", this->name.c_str(), this->latitude, this->longitude, this->value, this->sigma, (int)(this->primarySharers).size(), this->included ? "INCLUDED" : "EXCLUDED");
    return std::string(ss);
}

std::string Direction::toJSON() {
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

std::ostream& operator<<(std::ostream &strm, const Direction &direction) {
    char ss[1000];
    snprintf(ss, 1000, "%s %s %3.1f +- %.1f %s", direction.name.c_str(), direction.dfLocation->name.c_str(), direction.value, direction.sigma, direction.included ? "INCLUDED" : "EXCLUDED");
    return strm << ss;
}

bool operator== (const Direction& direction1, const Direction& direction2) {
    return (direction1.name == direction2.name);
}

bool operator< (const Direction& direction1, const Direction& direction2) {
    return (direction1.name < direction2.name);
}

void Direction::setId(int id) {
    Direction::id = id;
}


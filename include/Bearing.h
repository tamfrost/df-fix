#ifndef WEBASSEMBLY_BEARING_H
#define WEBASSEMBLY_BEARING_H

#include <utility>
// #include <nlohmann/json.hpp>

#include "geolib.h"
#include "GeoLocation.h"

// using json = nlohmann::json;

// class BearingCross;
class DfLocation;

/**
 *
 */
class Bearing: public GeoLocation {
private:
public:
    std::string name;
    int id = -1;
    bool included = false;
    vector<Bearing*> primarySharers;
    double value{}, sigma{},  originalSigma{};
    DfLocation* dfLocation{};

    Bearing() = default;
    Bearing(DfLocation* location, double value, double sigma, std::string id);
    Bearing(double longitude, double latitude, double value, double sigma, std::string id);
    Bearing(double value, double sigma, std::string id);

    void setDfLocation(DfLocation* dfLocation);
    // BearingCross cross(Bearing* bearing);
    void getLines(int nPoints, double distance, double *main, double *upper, double *lower);

    friend bool operator== (const Bearing& bearing1, const Bearing& bearing2);
    friend bool operator< (const Bearing& bearing1, const Bearing& bearing2);
    friend std::ostream& operator<<(std::ostream &strm, const Bearing &bearing);
    std::string toString();
    std::string toJSON();
    // std::string to_json();
    // void to_json(json& json, const Bearing& bearing);
    // void Bearing::to_json(json& j, const Bearing& b) {
    //     j = json{{"name", std::string(b.name)}};
    // }

    void setId(int id);
};

// inline void to_json(json& jb, const Bearing& b) {
//     jb = json::object();
//     jb["name"] = b.name;
//     jb["value"] = b.value;
//     jb["sigma"] = b.sigma;
//     jb["latitude"] = b.latitude;
//     jb["longitude"] = b.longitude;
//     jb["included"] = b.included;
//     jb["primarySharers"] = json::array();
//     for (auto ps : b.primarySharers) {
//         jb["primarySharers"].push_back(ps->id);
//     }
// }

#endif //WEBASSEMBLY_BEARING_H

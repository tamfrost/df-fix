#ifndef UNTITLED_LIKELIHOODTOOL_H
#define UNTITLED_LIKELIHOODTOOL_H

#include "geolib.h"

class LikelihoodTool {

public:
    LikelihoodTool(double lonStart, double latStart, VectorXd measuredDirections, MatrixXd sigmas, MatrixXd locations);
    vector<double> lonData;
    vector<double> latData;
    vector<double> lhData;
    vector<double> lonIndex;
    vector<double> latIndex;

private:
    double stepSize = 0.1;
    double lhoodMaxLat{};
    double lonNextMax{};
    double lonCenter, latCenter;
    VectorXd measuredDirections;
    MatrixXd sigmas;
    MatrixXd locations;
    void walkLonLat(double lonStart, double latStart, double latDiff);
    void walkNS(double lonStart, double latStart, double latDiff);
    void walkLon(double lonStart, double latStart, double latDiff);
    void walkEW(double lonOffset, double lonDiff, double lonStart, double latStart, double latDiff);
    double likelihood(double cLat, double cLon);
    template <typename T> int sign(T val) {
        return (T(0) < val) - (val < T(0));
    }
};


#endif //UNTITLED_LIKELIHOODTOOL_H

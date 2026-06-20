#ifndef WEBASSEMBLY_DIRECTIONCOLLECTION_H
#define WEBASSEMBLY_DIRECTIONCOLLECTION_H

#include <utility>

#include "geolib.h"
#include "Direction.h"

class DirectionCrossCollection;

/**
 *
 */
class DirectionCollection : public std::set<Direction*> {
protected:
public:
    ~DirectionCollection() = default;
    DirectionCollection() = default;
    explicit DirectionCollection(std::string name) : name(std::move(name)) {}

    std::string name;
    vector<double> sinValues, cosValues;
    double meanDirectionValue = 0, meanDirectionSigma = 0, medianDirection = 0;
    Direction meanDirection, maxDirectionDensity;
    MatrixXd finalEigenvecs;
    VectorXd directionDensity;
    DirectionErrorModel directionErrorModel = CIRCULAR;
    CrossErrorModel crossErrorModel = SIGMA;
    bool densityCalculated = false;
    vector<Vector2d> positionEstimates;

    void list();
    DirectionCrossCollection calculateCrosses();
    DirectionCrossCollection calculateCrosses(DirectionCollection* directionCollection);
    void selectDirections();
    vector<Vector2d> estimatePosition(double guessCoord1, double guessCoord2, double &angle, double &ev1, double &ev2, int &fitStatus);
//    void getLines(int nPoints, double distance, double* main, double* upper, double* lower, int* id, int* included);
    double calculateDirectionDensity();
    Direction calculateMeanDirection();
    Direction calculateMedianDirection();
    virtual Direction calculateMeanDirection(std::string id);
    map<std::string, vector<double>> calculateLikelihood(double cLon, double cLat);

    friend std::ostream& operator<<(std::ostream &strm, const DirectionCollection &directionCollection);
    void setName(const string &name);
    std::string directionsToJSON();
    std::string estimatesToJSON();
    std::string densityToJSON();
};


#endif //WEBASSEMBLY_DIRECTIONCOLLECTION_H

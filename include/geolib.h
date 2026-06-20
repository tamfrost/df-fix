#ifndef WEBASSEMBLY_GEOLIB_H
#define WEBASSEMBLY_GEOLIB_H

#include <cstdio>
#include <iostream>
#include <exception>
#include <cmath>
#include <utility>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <chrono>
#include <Eigen/Dense>
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/GeodesicLine.hpp>
#include <cstdarg>
#include "Logger.h"

using namespace GeographicLib;
using namespace Eigen;

const double PI = 3.14159265358979;
const double EARTH_RADIUS = 6371;

enum CrossErrorModel {SIGMA, CROSS_EQUAL, CROSS_NORM};
enum DirectionErrorModel {CIRCULAR, MEAN_OF_SIGMA};

class NoIncludedDirectionsException : public std::exception {
public:
    const char* what () const throw () {
        return "no included directions";
    }
};

class DivergentFit : public std::exception {
public:
    const char* what () const throw () {
        return "position estimate is diverging";
    }
};

namespace rfdfgeo {

    /**
     *
     * @param location
     * @return
     */
    Vector3d geo2cart(Vector2d location);

    /**
     *
     * @param x
     * @return
     */
    Vector2d cart2geo(Vector3d x);

    /**
     *
     * @param b1
     * @param b2
     * @return
     */
    double addDirection(double b1, double b2);

    /**
     *
     * @param b1
     * @param b2
     * @return
     */
    VectorXd addDirections(VectorXd b1, VectorXd b2);

    /**
     *
     * @param b1
     * @param b2
     * @return
     */
    VectorXd subtractDirections(VectorXd b1, VectorXd b2);

    /**
     *
     * @param b1
     * @param b2
     * @return
     */
    double subtractDirections(double b1, double b2);

    /**
     *
     * @param p1
     * @param p2
     * @return
     */
    double geodesicInverse(VectorXd p1, VectorXd p2);

    /**
     *
     * @param position
     * @param locations
     * @return
     */
    VectorXd geodesicInverse2d(Vector2d position, MatrixXd locations);

    /**
     *
     * @param lat1
     * @param lon1
     * @param lat2
     * @param lon2
     * @return
     */
    double getDirection(double lat1, double lon1, double lat2, double lon2);

    /**
     *
     * @param axis
     * @param angle
     * @return
     */
    Matrix3d rotationMatrix3d(Vector3d axis, double angle);

    /**
     *
     * @param nDirections
     * @param locationCoord
     * @param directionMeasured
     * @param sigma
     * @param crossGuess
     * @param angle
     * @param ev1
     * @param ev2
     * @param finalEigenvecs
     * @return
     */
    VectorXd solveIterationSphere(int nDirections, MatrixXd locationCoord, VectorXd directionMeasured, VectorXd sigma, VectorXd crossGuess, double* angle, double* ev1, double* ev2, MatrixXd* finalEigenvecs);

    /**
     *
     * @param nDirections
     * @param locationCoord
     * @param locationDirections
     * @param sigmas
     * @param crossGuess
     * @param crossErrorModel
     * @param angle
     * @param ev1
     * @param ev2
     * @param fitStatus
     * @param finalEigenvecs
     * @return
     */
    vector<Vector2d> positionEstimate(int nDirections, MatrixXd locationCoord, VectorXd locationDirections, VectorXd sigmas, VectorXd crossGuess, double* angle, double* ev1, double* ev2, int* fitStatus, MatrixXd* finalEigenvecs);

    /**
     *
     * @param lonStart
     * @param latStart
     * @param measuredDirections
     * @param sigmas
     * @param locations
     * @return
     */
    map<std::string, vector<double>> calculateLikelihood(double lonStart, double latStart, VectorXd measuredDirections, MatrixXd sigmas, MatrixXd locations);

    /**
     *
     * @param mean
     * @param sigma
     * @return
     */
    VectorXd kernelDensity(double mean, double sigma);

    /**
     *
     * @param kernelDensity
     * @return
     */
    double kernelPeaks(VectorXd kernelDensity);

}

#endif //WEBASSEMBLY_GEOLIB_H

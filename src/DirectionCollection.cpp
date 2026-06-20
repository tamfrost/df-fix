#include "DirectionCollection.h"
#include "DirectionCrossCollection.h"
#include "DfLocation.h"
#include "LikelihoodTool.h"
#include <complex>
//#include <unsupported/Eigen/MatrixFunctions>

std::ostream& operator<<(std::ostream &strm, const DirectionCollection &directionCollection) {
    return strm << "DIRECTION COLLECTION: " << directionCollection.size();
}

//void DirectionCollection::getLines(int nPoints, double distance, double* main, double* upper, double* lower, int* id, int* included) {
//
//    int nDirections = this->size();
//
//    const Geodesic& geod = Geodesic::WGS84();
//    int iDirection = 0;
//    for(auto direction : *this) {
//
//        id[iDirection] = direction->id;
//        included[iDirection] = direction->included ? 1 : 0;
//
//        const GeodesicLine& mainLine = geod.Line(direction->latitude/*replace1*/, direction->longitude/*replace1*/, direction->value, Geodesic::ALL);
//        const GeodesicLine& upperLine = geod.Line(direction->latitude/*replace1*/, direction->longitude/*replace1*/, direction->value + direction->sigma, Geodesic::ALL);
//        const GeodesicLine& lowerLine = geod.Line(direction->latitude/*replace1*/, direction->longitude/*replace1*/, direction->value - direction->sigma, Geodesic::ALL);
//
//        for (int iPoint=0; iPoint < nPoints; iPoint++) {
//
//            int col = iPoint;
//            int latRowStart = nPoints * 2 * iDirection;
//            int lonRowStart = latRowStart + nPoints;
//
//            int latIndex = latRowStart + col;
//            int lonIndex = lonRowStart + col;
//
//            mainLine.Position(distance * iPoint, main[latIndex], main[lonIndex]);
//            upperLine.Position(distance * iPoint, upper[latIndex], upper[lonIndex]);
//            lowerLine.Position(distance * iPoint, lower[latIndex], lower[lonIndex]);
//        }
//        iDirection++;
//    }
//}

//DirectionCollection DirectionCrossCollection::getDirections() {
//    DirectionCollection includedDirections;
//    std::set<Direction*> dum;
//    for (DirectionCross bc : *this) {
//        Direction* b1 = bc.direction1;
//        Direction* b2 = bc.direction2;
//        if (bc.direction1->included && bc.direction2->included) {
//            includedDirections.insert(b1);
//            includedDirections.insert(b2);
//        }
//    }
//    return includedDirections;
//}

void DirectionCollection::list() {
    Logger->log("LIST OF DIRECTION COLLECTION: %s", name.c_str());
    for(Direction* direction : *this) {
        Logger->log(direction->toString().c_str());
    }
}

/**
 * calculate all crosses between this collection and one passed in as an argument
 * @param directionCollection DirectionCollection which to calculate all crosses with
 * @return all calculated crosses
 */
DirectionCrossCollection DirectionCollection::calculateCrosses(DirectionCollection* directionCollection) {

    DirectionCrossCollection crosses;

    //loop over all possible direction pairs
    for (auto direction1 : *this)
    {
        for (auto direction2 : *directionCollection)
        {
            DirectionCross cross(direction1, direction2);
            crosses.push_back(cross);
        }
    }
    return crosses;
}

/**
 * calculate all crosses between this collection and itself without double counting
 * @return all calculated crosses
 */
DirectionCrossCollection DirectionCollection::calculateCrosses() {

    DirectionCrossCollection crosses;

    //loop over all possible direction pairs
    int i=0;
    for (auto direction1 : *this)
    {
        direction1->setId(i);
        crosses.underlyingDirections.push_back(direction1);
        for (auto direction2 : *this) {
            if (direction1->value > direction2->value) {
                DirectionCross cross(direction1, direction2);
                crosses.push_back(cross);
            }
        }
        i++;
    }
    return crosses;
}

map<std::string, vector<double>> DirectionCollection::calculateLikelihood(double cLon, double cLat) {

    int nIncluded = 0;
    for (const auto& direction : *this) {
        if (direction->included) {
            nIncluded++;
        }
    }

    if (nIncluded < 1) throw NoIncludedDirectionsException();

    VectorXd measuredDirections(nIncluded);
    MatrixXd sigmas(nIncluded, nIncluded); sigmas.setZero();
    MatrixXd locations(nIncluded, 2);
    int iRow = 0;
    for (const auto& direction : *this) {
        if (direction->included) {
            measuredDirections(iRow) = direction->value;
            sigmas(iRow, iRow) = direction->sigma * direction->sigma;
            locations(iRow,0) = direction->longitude/*replace1*/;
            locations(iRow,1) = direction->latitude/*replace1*/;
            iRow++;
        }
    }

    return rfdfgeo::calculateLikelihood(cLon, cLat, measuredDirections, sigmas, locations);
}

void DirectionCollection::selectDirections() {
    Logger->log("SELECTING DIRECTIONS: accept when >= %i (of max %i) primary sharers", (int) ceil(0.6 * (float) this->size()), this->size());
    for (const auto& direction : *this) {
        if (direction->included && direction->primarySharers.size() < ceil(0.6 * (float) this->size())) {
            direction->included = false;
        }
    }
}

vector<Vector2d> DirectionCollection::estimatePosition(double guessCoord1, double guessCoord2, double &angle, double &ev1, double &ev2, int &fitStatus) {

    Vector2d positionGuess;
    positionGuess << guessCoord1, guessCoord2;

    int nIncluded = 0;
    for (const auto& direction : *this) {
        if (direction->included) {
            nIncluded++;
        }
    }

    if (nIncluded <= 1) {
        angle = 0;
        ev1 = 0;
        ev2 = 0;
        fitStatus = this->size() <= 1 ? -1 : -2;
        vector<Vector2d> returnVector;
        returnVector.push_back(positionGuess);
        return returnVector;
    };

    MatrixXd locationCoordinates(nIncluded, 2);
    VectorXd directions(nIncluded);
    VectorXd sigmas(nIncluded);
    int iRow = 0;
    for (const auto& direction : *this) {
        if (direction->included) {
            locationCoordinates.row(iRow) << direction->longitude/*replace1*/, direction->latitude/*replace1*/;
            directions(iRow) = direction->value;
            sigmas(iRow) = direction->originalSigma;
            iRow++;
        }
    }

    positionEstimates = rfdfgeo::positionEstimate(
            nIncluded, locationCoordinates, directions, sigmas, positionGuess, //in
            &angle, &ev1, &ev2, &fitStatus, &finalEigenvecs //out
    );

    //do some modifications to the errors if there are more than 2 directions
//    if (crossErrorModel != SIGMA && nDirections > 2) {
    if (nIncluded > 2 && fitStatus == 0) {

        VectorXd positionDirections(nIncluded), newSigmas(nIncluded), positionDiff(nIncluded);

        for (int i = 0; i< nIncluded; i++) {
            positionDirections(i) = rfdfgeo::geodesicInverse(positionEstimates.back(), locationCoordinates.row(i));
        }
        //absolute difference to the measured directions
        positionDiff = (positionDirections - directions).cwiseAbs();

        double sigmaNorm = sigmas.mean();
        double diffNorm = positionDiff.mean();
        newSigmas = sigmas;
        //condition for using modified sigmas
//        bool modifySigma = diffNorm > sigmaNorm && diffNorm/sigmaNorm < 5;
        bool modifySigma = true;

        Logger->log("SIGMAS     %f %f %f", sigmas(0), sigmas(1), sigmas(2));
        Logger->log("%s SIGMAS %f %s %f", modifySigma ? "MODIFYING" : "NOT MODIFYING", diffNorm, modifySigma ? ">" : "<", sigmaNorm);

        if (modifySigma) {
            //scale original errors with mean value of the spread of the crosses
            if (crossErrorModel == SIGMA) {
                for (const auto &direction : *this) {
                    if (direction->included) {
                        direction->sigma = direction->originalSigma;
                    }
                }
            }

            //scale original errors with mean value of the spread of the crosses
            if (crossErrorModel == CROSS_NORM) {
                newSigmas = (diffNorm * sigmas / sigmaNorm).transpose();
                iRow = 0;
                for (const auto &direction : *this) {
                    if (direction->included) {
                        direction->sigma = newSigmas(iRow);
                        iRow++;
                    }
                }
            }
            //set all errors to the mean value of the spread of the crosses
            if (crossErrorModel == CROSS_EQUAL) {
                iRow = 0;
                for (const auto &direction : *this) {
                    if (direction->included) {
                        newSigmas(iRow) = positionDiff.mean();
                        direction->sigma = positionDiff.mean();
                        iRow++;
                    }
                }
            }

            Logger->log("NEW SIGMAS %f %f %f", newSigmas(0), newSigmas(1), newSigmas(2));

            positionEstimates.push_back(rfdfgeo::positionEstimate(
                    nIncluded, locationCoordinates, directions, newSigmas, positionEstimates.back(), //in
                    &angle, &ev1, &ev2, &fitStatus, &finalEigenvecs //out
            ).back());
        }
    }

    return positionEstimates;
}

double DirectionCollection::calculateDirectionDensity() {
    directionDensity = VectorXd::Zero(360);
    for(Direction* direction: *this) {
        directionDensity = directionDensity + rfdfgeo::kernelDensity(direction->value, 4);
    }
    maxDirectionDensity = Direction(rfdfgeo::kernelPeaks(directionDensity), 0, to_string(-1));
    densityCalculated = true;
    return maxDirectionDensity.value;
}

Direction DirectionCollection::calculateMeanDirection() {

    double sumWeight=0, sumSigma=0;
    int nIncluded = 0;
    std::complex<double> m1(0,0), m2(0,0), complexDirection(0,0);
    Direction* lastProcessedDirection;

    for(Direction* direction: *this) {
        if (direction->included) {
            nIncluded++;
            double theta = -direction->value + 90;
            double weight = 1/direction->sigma;
            sumWeight += weight;
            sumSigma += direction->sigma;
            complexDirection = polar(1.0, theta * PI/180);
            sinValues.push_back(complexDirection.real());
            cosValues.push_back(complexDirection.imag());
            m1 += weight * complexDirection;
            m2 += weight * pow(complexDirection, 2);
            lastProcessedDirection = direction;
        }
    }

    if (nIncluded < 1) throw NoIncludedDirectionsException();

    m1 = m1/sumWeight;
    m2 = m2/sumWeight;

    double R1 = abs(m1);
    double R2 = abs(m2);

    //variance
    double sigmaOfMean = sqrt(2*(1-R1))*180/PI;
//    //standard error
//    double sigmaOfMean = sqrt((1-R2)/(2*R1*R1))*180/PI;
//    //mean of sigma
//    double sigmaOfMean = sumSigma/this->size()/sqrt(this->size());

//    double meanDir = -atan2(sumy,sumx)*180/PI+90;
    double meanDir = -atan2(m1.imag(), m1.real())*180/PI+90;
    Direction returnDirection =
            (nIncluded == 1) ?
            *lastProcessedDirection :
            Direction(meanDir < 0 ? 360+meanDir : meanDir, sigmaOfMean, to_string(-1));

    this->meanDirectionValue = returnDirection.value;
    this->meanDirectionSigma = returnDirection.sigma;
    this->meanDirection = returnDirection;

    if (densityCalculated) Logger->log("MEAN: %f +- %f, MAX DENSITY AT ~%.0f ", this->meanDirectionValue, this->meanDirectionSigma, maxDirectionDensity.value);
    else Logger->log("MEAN: %f +- %f", this->meanDirectionValue, this->meanDirectionSigma);

    if (!densityCalculated) densityCalculated = true;

    return returnDirection;
}

Direction DirectionCollection::calculateMedianDirection() {

    int nIncluded = sinValues.size();

    if (nIncluded < 1) throw NoIncludedDirectionsException();

    sort(sinValues.begin(), sinValues.end());
    sort(cosValues.begin(), cosValues.end());
    double sinMedian = nIncluded%2 == 1 ? sinValues[nIncluded/2] : (sinValues[nIncluded/2] + sinValues[nIncluded/2 + 1])/2;
    double cosMedian = nIncluded%2 == 1 ? cosValues[nIncluded/2] : (cosValues[nIncluded/2] + cosValues[nIncluded/2 + 1])/2;

    this->medianDirection = nIncluded > 1 ? -atan2(cosMedian, sinMedian)*180/PI+90 : this->meanDirectionValue;

    Direction returnDirection = Direction(this->medianDirection < 0 ? 360+this->medianDirection : this->medianDirection, 0, to_string(-1));

//    for (int i=0; i<sinValues.size(); i++) {
//        Logger->log("SIN COS: %i %f %f", i, sinValues.at(i), cosValues.at(i));
//    }
//
//    Logger->log("MEDIAN: %i %i %f %f %f", nIncluded, nIncluded/2, sinMedian, cosMedian, -atan2(cosMedian, sinMedian)*180/PI+90);

    Logger->log("MEDIAN: %i %f", nIncluded, this->medianDirection);

    return returnDirection;
}

Direction DirectionCollection::calculateMeanDirection(std::string id) {
    try {
        Direction md = this->calculateMeanDirection();
        md.name = id;
        return md;
    }
    catch (NoIncludedDirectionsException e) {
//        std::cout << "MEAN DIRECTION CALCULATION FOR " << this->name << ": " << e.what() << std::endl;
        throw e;
    }
//
//
//    return Direction(DfLocation(), this->meanDirectionValue, this->meanDirectionSigma, -1);
}

void DirectionCollection::setName(const string &name) {
    DirectionCollection::name = name;
}

std::string DirectionCollection::directionsToJSON()
{
    int i = 0;
    std::string jsonArray = "[";
    for (auto direction : *this) {
        jsonArray += direction->toJSON();
        if (i < this->size() - 1) jsonArray += ',';
        i++;
    }
    jsonArray += "]";
    return jsonArray;
}

std::string DirectionCollection::estimatesToJSON()
{
    int i = 0;
    std::string jsonArray = "[";
    for (auto estimate : positionEstimates) {
        jsonArray += "[" + to_string(estimate(1)) + ", " + to_string(estimate(0)) + "]";
        if (i < positionEstimates.size() - 1) jsonArray += ',';
        i++;
    }
    jsonArray += "]";
    return jsonArray;
}


std::string DirectionCollection::densityToJSON()
{
    std::string jsonArray = "[";
    for (int i = 0; i<360; i++) {
        jsonArray += to_string(directionDensity[i]);
        if (i < 359) jsonArray += ',';
    }
    jsonArray += "]";
    return jsonArray;
}


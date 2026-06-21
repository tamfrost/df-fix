#include <utility>
#include "fixlib.h"

// #ifndef GEOFIX_VERSION
// #define GEOFIX_VERSION "1.0.0"
// #endif 

// #ifndef COMMIT_HASH
// #define COMMIT_HASH "abc123"
// #endif 

DfAnalyzer::DfAnalyzer() {
    allCrosses.setName("all calculated crosses");
    meanCrosses.setName("mean direction crosses");
    allDirections.setName("all directions");
    meanDirections.setName("mean directions");
    Logger->Configure("NONE");
}

DfAnalyzer::DfAnalyzer(int loggingEnabled) {
    allCrosses.setName("all calculated crosses");
    meanCrosses.setName("mean direction crosses");
    allDirections.setName("all directions");
    meanDirections.setName("mean directions");
    Logger->Configure(loggingEnabled == 0 ? "NONE" : "DEBUG");
}

DfAnalyzer::~DfAnalyzer() {
    Logger->log("CLEANING UP");
    deleteContainerItems();
}

/**
 * delete items from containers with pointers
 */
void DfAnalyzer::deleteContainerItems() {
    int nItems = 0;
    for (const auto& dfLocation : dfLocations) {
        delete dfLocation.second;
        nItems++;
    }
    for(const auto&  direction : allDirections) {
        delete direction;
        nItems++;
    }
    for(const auto&  direction : meanDirections) {
        delete direction;
        nItems++;
    }
    Logger->log("DELETED "  + to_string(nItems) + " CONTAINER ITEMS");
}

/**
 * empty all the containers
 */
void DfAnalyzer::clear() {
    for (auto location: dfLocations) {
        location.second->clear();
    }
    allCrosses.clear();
    meanCrosses.clear();

    deleteContainerItems();
    allDirections.clear();
    meanDirections.clear();
    dfLocations.clear();

    Logger->clearLog();
}

void DfAnalyzer::logCompilationTime() {
    Logger->log("COMPILE TIME: " + std::string(__DATE__) + " " + std::string(__TIME__));
}

std::string DfAnalyzer::getCompilationTime() {
    return std::string(__DATE__) + " " + std::string(__TIME__);
}

void DfAnalyzer::logVersion() {
    Logger->log("GEOFIX VERSION: " + std::string(GEOFIX_VERSION));
}

std::string DfAnalyzer::getVersion() {
    return std::string(GEOFIX_VERSION);
}

void DfAnalyzer::logCommitHash() {
    Logger->log("COMMIT HASH: " + std::string(COMMIT_HASH));
}

std::string DfAnalyzer::getCommitHash() {
    return std::string(COMMIT_HASH);
}

void DfAnalyzer::configure(const std::string& jsonConfigString) {
}

/**
 * calculate fix point for a number of bearings
 * @param nDirections          number of directions added
 * @param locationCoordinates  pointer to coordinates of location for each direction, must be of length 2 x nDirections
 * @param directionsPtr        pointer to the direction values followed by the associated errors must be of length 2 x nDirections
 */
std::string DfAnalyzer::getFixpoint(DirectionErrorModel directionErrorModel, int nDirections, const double* locationCoordinates, double* directionsPtr) {

    double angle, ev1, ev2;
    int fitStatus;

    clear();

    for(int i=0; i<nDirections; i++) {
        auto *direction = new Direction(locationCoordinates[2 * i], locationCoordinates[2 * i + 1], directionsPtr[i], directionsPtr[i + nDirections], "B" + to_string(i));
        direction->included = true;
        allDirections.insert(direction);
        Logger->log("ADDING DIRECTION: " + direction->toString());
    }

    // DirectionCrossCollection crosses = allDirections.calculateCrosses();
    allCrosses = allDirections.calculateCrosses();
    Logger->log("CROSSES: " + allCrosses.toString());
    for (auto cross : allCrosses) {
        Logger->log("CROSSES: " + cross.toString());
    }

    allDirections.selectDirections();

    // Vector2d firstGuess = allCrosses.getMeanCrossLocation();

    allDirections.list();

    // allDirections.estimatePosition(firstGuess[0], firstGuess[1], angle, ev1, ev2, fitStatus);

    // std::cout << firstGuess.transpose() << std::endl;
    // std::cout << "{\"bearings\": " + allDirections.directionsToJSON() + "," + "\"crosses\": " + crosses.toJSON() + "}";

    return "{\"directions\": " + allDirections.directionsToJSON() + "," + "\"crosses\": " + allCrosses.toJSON() + "," + "\"estimates\": " + allDirections.estimatesToJSON() + "}";
}

/**
 * add directions from different df-locations
 * @param nDirections          number of directions added
 * @param locationCoordinates  pointer to coordinates of location for each direction, must be of length 2 x nDirections
 * @param directionsPtr        pointer to the direction values followed by the associated errors must be of length 2 x nDirections
 */
void DfAnalyzer::loadDirections(DirectionErrorModel directionErrorModel, int nDirections, const double* locationCoordinates, double* directionsPtr) {

    clear();

    //all directions
    for(int i=0; i<nDirections; i++) {

        double lat = locationCoordinates[2 * i + 1];
        double lon = locationCoordinates[2 * i];

        //id for the location based on the integer parts of the coordinates (lon+1000*lat)
        std::string locationIdentifier =std:: to_string((int)(round(lon) + 1000 * round(lat)));

        if (dfLocations.count(locationIdentifier) == 0) {
            Logger->log("ADDING NEW LOCATION: " + locationIdentifier);
            auto* dfLocation = new DfLocation(lat, lon, locationIdentifier);
            dfLocation->directionErrorModel = directionErrorModel;
            dfLocations[locationIdentifier] = dfLocation;
        }

        // auto *bearing = new Bearing(dfLocations[locationIdentifier], directionsPtr[i], directionsPtr[i + nDirections], "A" + to_string(i));
        // std::cout << "Serialized JSON:\n" << bearing->to_json() << std::endl;
        auto *direction = new Direction(dfLocations[locationIdentifier], directionsPtr[i], directionsPtr[i + nDirections], "A" + to_string(i));
        direction->included = true;
        allDirections.insert(direction);
        dfLocations.at(direction->dfLocation->name)->insert(direction);
        Logger->log("ADDING DIRECTION: " + direction->toString());
    }

    //get mean directions for all locations
    int nMeans = 1;
    for (auto location : dfLocations) {
        try {
            Direction meanDirection = location.second->calculateMeanDirection(location.second->name);
            auto direction = new Direction(meanDirection.dfLocation, meanDirection.value, meanDirection.sigma, meanDirection.name);
            meanDirections.insert(direction);
            nMeans++;
        }
        catch (NoIncludedDirectionsException &e) {
            Logger->log("no included directions");
        }
    }

    //calculate mean direction crosses
    meanCrosses = meanDirections.calculateCrosses();
    meanCrosses.setName("mean crosses");
    if (meanCrosses.size() > 3) {
        for(auto direction: meanDirections) {
            meanCrosses.findOutliers(direction);
        }
    }
    meanCrosses.analyse();
    meanCrosses.list();
    meanDirections.list();

//    cout << "FINDING DISJOINT SETS" << endl;
//    auto disjointSets = meanCrosses.findDisjointSets();
//    int maxSetSize = -1;
// //    int biggestSetKey = -1;
//    DirectionCollection biggestSet;
//    for (auto &set: disjointSets) {
// //        cout << set.first << " " << set.second.name << " " <<  set.second.size() << endl;
//        if ((int)set.second.size() > maxSetSize) {
//            maxSetSize = (int)set.second.size();
// //            biggestSetKey = set.first;
//            biggestSet = set.second;
//        }
//    }
//    cout << "BIGGEST SET: " << " " << biggestSet.name << " " << biggestSet.size() << endl;

}

int DfAnalyzer::getMeanPositionEllipse(int collection, CrossErrorModel errorModel, int nPoints, double confidenceLevelIn, double* ellipse, double* firstGuess, double* center, double* confidenceLevelOut, std::string* estimates) {
    double angle, ev1, ev2;
    int fitStatus;

    Logger->log("GETTING POSITION FOR DIRECTIONS (CONFIDENCE LEVEL %f)", confidenceLevelIn);

    DirectionCrossCollection usedCrossCollection = collection == 0 ? meanCrosses : allCrosses;
    DirectionCollection usedDirectionCollection = collection == 0 ? meanDirections: allDirections;

    auto positionGuess = usedCrossCollection.getMeanCrossLocation(-1);
    // auto positionGuess = usedCrossCollection.getPositionFirstGuess();

    meanDirections.crossErrorModel = errorModel;
    vector<Vector2d> positionEstimates = usedDirectionCollection.estimatePosition(positionGuess[0], positionGuess[1], angle, ev1, ev2, fitStatus);
    Vector2d positionEstimate = positionEstimates.back();
    GeoLocation confidenceIntervalCenter(positionEstimate(1), positionEstimate(0));
    *estimates = usedDirectionCollection.estimatesToJSON();

    double minorAxisDeg = sqrt(-2 * log(1 - confidenceLevelIn) * ev1);
    double majorAxisDeg = sqrt(-2 * log(1 - confidenceLevelIn) * ev2);
    double cfLevel1 = 1-exp(majorAxisDeg * majorAxisDeg / (-2 * ev2));

    //
    double latitudeCorrection = cos(positionEstimate[1] * PI / 180);
    majorAxisDeg = majorAxisDeg * latitudeCorrection;

    double majorAxisKm = majorAxisDeg * PI / 180 * EARTH_RADIUS;

    if (collection == 0) {
        //recursively calculate mean coordinates for all DF-locations and its distance to the estimate center
        std::vector<GeoLocation*> allLocations;
        for (const auto& location : this->dfLocations) {allLocations.push_back(location.second);}
        GeoLocation locationMean = *allLocations.at(0);
        for (int i=1; i < allLocations.size() - 1; i++) {locationMean = locationMean.meanWith(*allLocations.at(i));}
        double distanceToEstimate = confidenceIntervalCenter.distanceTo(locationMean);
        
        Logger->log("MEAN DISTANCE TO ESTIMATE %.2f km", distanceToEstimate);
        Logger->log("MAJOR AXIS OF CONFIDENCE INTERVAL %.2f km", majorAxisKm);

        if (majorAxisKm > distanceToEstimate) {
    //        double confidenceCorrection = exp(-0.01*majorAxisDeg);
            double confidenceCorrection = 0.75 * distanceToEstimate/majorAxisKm;
            majorAxisDeg = majorAxisDeg * confidenceCorrection;
            minorAxisDeg = minorAxisDeg * confidenceCorrection;
            cfLevel1 =  1-exp(majorAxisDeg*majorAxisDeg/(-2*ev2));
            Logger->log("ADJUSTING SIZE OF CONFIDENCE INTERVAL BY A FACTOR %.2f", pow(confidenceCorrection,2));
            Logger->log("NEW CONFIDENCE LEVEL IS %.2f PERCENT", 100*cfLevel1);
        }
    }

    calculateEllipse(
            nPoints,
            positionEstimate,
            majorAxisDeg,
            minorAxisDeg,
            angle,
            usedDirectionCollection.finalEigenvecs,
            ellipse
    );

    firstGuess[0] = positionGuess(1);
    firstGuess[1] = positionGuess(0);
    center[0] = positionEstimate(1);
    center[1] = positionEstimate(0);

    *confidenceLevelOut = fitStatus < 0 ? 0 : cfLevel1;

    return fitStatus;
}

void DfAnalyzer::getLikelihood(double lat, double lon)  {
    likelihoodData = meanDirections.calculateLikelihood(lon, lat);
}

//void DfAnalyzer::getDirectionLines(std::string locationTag, int nPoints, double distance, double* mainLine, double* upperLine, double* lowerLine, int* directionId, int* included) {
////    Logger->log("GETTING DIRECTION LINES FOR LOCATION: %s", locationTag.c_str());
//    DfLocation* dfLocation = dfLocations[locationTag];
//    dfLocation->getLines(nPoints, distance, mainLine, upperLine, lowerLine, directionId, included);
//}

double DfAnalyzer::getDirection(double lat1, double lon1, double lat2, double lon2) {
    Vector2d location1, location2;
    location1 << lat1, lon1;
    location2 << lat2, lon2;
    return rfdfgeo::geodesicInverse(location2, location1);
}

void DfAnalyzer::greatCircle(int nPoints, double lat, double lon, double direction, double distance, double* main) {
    const Geodesic& geod = Geodesic::WGS84();
    const GeodesicLine& mainLine = geod.Line(lat, lon, direction, Geodesic::ALL);
    for (int iPoint=0; iPoint < nPoints; iPoint++) {
        mainLine.Position(distance * iPoint / nPoints, main[iPoint], main[nPoints + iPoint]);
    }
};

void DfAnalyzer::calculateEllipse(int n, Vector2d center, double major, double minor, double theta, MatrixXd finalEigenvecs, double* result) {

    center.reverseInPlace();
    double clon = center[1];
    double clat = center[0];

    //calculate the angle of the major axes in a local north-east coordinate system
    Matrix2d rotationMatrix;
    rotationMatrix << cos(theta), -sin(theta), sin(theta), cos(theta);
    Vector2d mAxis;
    mAxis << -0.01, 0;
    Vector2d majorAxis = (rotationMatrix * mAxis).reverse() + center;
    Vector3d centerCartesian = DirectionCross::geographic2cartesian_(center);
    Vector3d majorAxisCartesian = DirectionCross::geographic2cartesian_(majorAxis) - DirectionCross::geographic2cartesian_(center);
    Vector3d east, north, up;
    up = centerCartesian;  // pointing up
    east << -sin(clon*PI/180), cos(clon*PI/180), 0.0; // pointing east
    north = up.cross(east);  //pointing north
    up.normalize();
    east.normalize();
    north.normalize();
    double majorAxisAngle = atan2(majorAxisCartesian.dot(north), majorAxisCartesian.dot(east));

    Vector3d zAxis, yAxis, xAxis;
    zAxis << 0,0,1; yAxis << 0,1,0; xAxis << 1,0,0;

    Matrix3d yAxisRotation = rfdfgeo::rotationMatrix3d(yAxis, -clat);
    Matrix3d zAxisRotation = rfdfgeo::rotationMatrix3d(zAxis, clon);

    for (int i=0; i<n; i++) {
//        Vector2d x;
        Vector2d xx;

        xx << minor*cos(2*PI*i/n), major*sin(2*PI*i/n);
        Vector3d xxMoved = zAxisRotation * yAxisRotation * DirectionCross::geographic2cartesian_(xx);
        Vector3d newXAxis = zAxisRotation * yAxisRotation * xAxis;
        Vector3d xxRotated = rfdfgeo::rotationMatrix3d(newXAxis,majorAxisAngle*180/PI) * xxMoved;
        Vector2d xxRotatedGeo = DirectionCross::cartesian2geographic_(xxRotated);
        result[i] = xxRotatedGeo(0);
        result[i + n] = xxRotatedGeo(1);

//        x << major*cos(-2*PI*i/n), minor*sin(-2*PI*i/n);
//        Vector2d xRotated = rotationMatrix * x;
//
//        xyEllipse(1,:) = major * sin(t);
//        xyEllipse(2,:) = minor * cos(t);
//        obj.ellipsePoints = GeoLocation.rmat([0,0,1], obj.lon) * GeoLocation.rmat([0,1,0], -obj.lat) * GeoLocation.geo2cart(xyEllipse);
//        nxRotated = GeoLocation.rmat([0,0,1], obj.lon) * GeoLocation.rmat([0,1,0], -obj.lat) * [1,0,0]';
//        obj.ellipsePoints = GeoLocation.rmat(nxRotated, angle) * obj.ellipsePoints;
//        result[i] = clat + xRotated(1);
//        result[i + n] = clon + xRotated(0);

    }

    Logger->log("MAJOR: %.2f km, %.2f deg", major * PI/180 * EARTH_RADIUS, major);
    Logger->log("MINOR: %.2f km, %.2f deg", minor * PI/180 * EARTH_RADIUS, minor);
    Logger->log("TILT: %.2f deg", majorAxisAngle * 180 / PI);
    Logger->log("CENTER: (%.2f,%.2f)", center[0], center[1]);
}

const map<std::string, DfLocation *> &DfAnalyzer::getDfLocations() const {
    return dfLocations;
}

DirectionCrossCollection DfAnalyzer::getCrosses(int collection) const {
    return collection == 0 ? meanCrosses : allCrosses;
}

DirectionCrossCollection DfAnalyzer::getMeanCrosses() const {
    return meanCrosses;
}

DirectionCollection DfAnalyzer::getMeanDirections() const {
    return meanDirections;
}

std::vector<std::string> DfAnalyzer::getLogMessages() const {
    return Logger->GetLogger()->getLoglog();
}

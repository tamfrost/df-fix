#include "LikelihoodTool.h"
#include "DirectionCollection.h"

LikelihoodTool::LikelihoodTool(double lonStart, double latStart, VectorXd measuredDirections, MatrixXd sigmas, MatrixXd locations) {
    this->lonCenter = lonStart;
    this->latCenter = latStart;
    this->measuredDirections = std::move(measuredDirections);
    this->sigmas = std::move(sigmas);
    this->locations = std::move(locations);
    walkLonLat(lonStart, latStart, stepSize);
}

void LikelihoodTool::walkLonLat(double lonStart, double latStart, double latDiff) {
//    cout << measuredDirections << endl;
//    cout << sigmas << endl;
//    cout << locations << endl;
//    cout << "WALKING NORTH" << endl;
    walkNS(lonStart, latStart, latDiff); //walk north
//    cout << "WALKING SOUTH" << endl;
    walkNS(lonStart, latStart-latDiff, -latDiff); //walk south
}

void LikelihoodTool::walkNS(double lonStart, double latStart, double latDiff) {
    lhoodMaxLat = 1; // must set to bigger than 0.001 to start the while loop
    lonNextMax = lonStart;
    double iLat = 0;
    while(lhoodMaxLat > 0.0001) {
        iLat++;
        if (iLat > 1000) break;
        walkLon(lonNextMax, latStart, latDiff);
        latStart = latStart + latDiff;
    }
}

void LikelihoodTool::walkLon(double lonStart, double latStart, double latDiff) {
    lhoodMaxLat = 0;
    lonNextMax = 0;
//    cout << "WALKING EAST" << endl;
    walkEW(0, stepSize, lonStart, latStart, latDiff); //walk east
//    cout << "WALKING WEST" << endl;
    walkEW(-stepSize, -stepSize, lonStart, latStart, latDiff); //walk west
}

void LikelihoodTool::walkEW(double lonOffset, double lonDiff, double lonStart, double latStart, double latDiff) {
    double lhood = 1, lhoodNext;
    double iLon = 0;
    while (lhood > 0.0001) {
        iLon++;
        if (iLon>1000) break;
        lhood = likelihood(lonStart + lonOffset, latStart);
        lhoodNext = likelihood(lonStart + lonOffset, latStart + latDiff);
        lonData.push_back(lonStart+lonOffset);
        latData.push_back(latStart);
        lhData.push_back(lhood);
        lonIndex.push_back(round((lonStart+lonOffset-lonCenter) / stepSize));
        latIndex.push_back(round((latStart - latCenter) / stepSize));
//        cout << lonOffset/stepSize << " " << sign(latDiff)*(latStart-latCenter)/latDiff << " "  << lhood << " " << lhoodNext << endl;
        lonOffset += lonDiff;
        if (lhoodNext > lhoodMaxLat) {
            lhoodMaxLat = lhoodNext;
            lonNextMax = lonStart + lonOffset;
        }
    }
}

double LikelihoodTool::likelihood(double cLon, double cLat) {
    Vector2d position;
    position << cLon, cLat;
    VectorXd actualDirections = rfdfgeo::geodesicInverse2d(position, locations);
    VectorXd diff = measuredDirections - actualDirections;
    return exp((double) ((-diff.transpose() * sigmas.inverse() * diff)));
}



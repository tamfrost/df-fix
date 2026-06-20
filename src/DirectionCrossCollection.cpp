#include "DirectionCrossCollection.h"
#include "DfLocation.h"

Vector3d DirectionCrossCollection::getMeanCrossLocation(int exclusionIndex) {

    //count included crosses
    int nIncluded = 0;
    for (size_t i = 0; i < this->size(); ++i) {
        if (i == exclusionIndex) continue;
        const auto& cross = (*this)[i];
        if (cross.included) {
            nIncluded++;
        }
    }

    //if no crosses are included set guess to (-1,-1)
    if (nIncluded == 0) {
        return Vector3d(-1, -1, 0);
    }

    //loop and sum up the coordinates of all included crosses
    double x = 0; double y = 0; double z = 0;
    for (size_t i = 0; i < this->size(); ++i) {
        if (i == exclusionIndex) continue;
        const auto& cross = (*this)[i];
        if (cross.included) {
            auto cartCross = rfdfgeo::geo2cart(cross.crossLocation);
            x += cartCross[0]; y += cartCross[1]; z += cartCross[2];
        }
    }

    //mean location
    Vector2d cmean = rfdfgeo::cart2geo(Vector3d(x, y, z)).reverse();

    //circular standard deviation
    double cstd = sqrt(2*(1-(Vector3d(x, y, z).norm() / nIncluded)))*180/PI;
    
    // Append an element to the Vector2d by creating a new vector with an additional element
    Vector3d cmeanstd;
    cmeanstd << cmean, cstd;

    Logger->log("CROSS COLLECTION STAT: %d, (%f,%f), %f", exclusionIndex, cmean[0], cmean[1], cstd);

    return cmeanstd;
}



double DirectionCrossCollection::findOutliers(Direction* direction) {

    std::queue<bool> icludedOriginal;
    for (auto& cross : *this) {
        icludedOriginal.push(cross.included);
        if (direction == cross.direction1 || direction == cross.direction2) {
            cross.included = false;
        }
        else {
            cross.included = true;
            // Logger->log("FIND OUTTLIERS: %s %s %s", direction->name.c_str() , cross.direction1->name.c_str(), cross.direction2->name.c_str());
        }
    }

    Vector3d crossStatistics = getMeanCrossLocation(-1);

    for (auto& cross : *this) {
        cross.included = icludedOriginal.front();
        icludedOriginal.pop();
    }

    Vector3d crossStatisticsAll = getMeanCrossLocation(-1);

    Logger->log("FIND OUTTLIERS: %f %f %f", crossStatistics[2] , crossStatisticsAll[2] , crossStatistics[2]/crossStatisticsAll[2]);

    if (crossStatistics[2]/crossStatisticsAll[2] < 0.5) {
        direction->included = false;
        for (auto& cross : *this) {
            if (direction == cross.direction1 || direction == cross.direction2) {
                cross.included = false;
            }
            else {
                cross.included = true;
            }
        }
    } 

    return crossStatistics[2]/crossStatisticsAll[2];

}


Vector2d DirectionCrossCollection::getPositionFirstGuess() { //obsolete, use getMeanCrossLocation instead

    //count included crosses
    int nIncluded = 0;
    for (const auto& cross : *this) {
        if (cross.included) {
            nIncluded++;
        }
    }

    //if no crosses are included set guess to (-1,-1)
    if (nIncluded == 0) {
        VectorXd guess(2);
        guess << -1,-1;
        return guess;
    }

    //create a matrix with the coordinates of all included crosses
    MatrixXd crossCoordinates(nIncluded, 2);
    int iRow = 0;
    for (const auto& cross : *this) {
        if (cross.included) {
            crossCoordinates.row(iRow) = cross.crossLocation;
            iRow++;
        }
//        crossCoordinates.row(iRow) = cross.crossLocation;
    }
    return getPositionGuess(iRow, crossCoordinates);
}

/**
 * Guess the position location based on the coordiantes of all the n(n-1)/ calculateCrosses
 *
 * @param nLocations
 * @param crossCoordinates
 * @return
 */
VectorXd DirectionCrossCollection::getPositionGuess(int nLocations, MatrixXd crossCoordinates) {

    VectorXd guess(2);
    int nCrosses = crossCoordinates.rows();
    double crossMeanLat, crossMeanLon, crossMinDistance=1000000;

    //if only two DF-locations or number of crosses the guess is naturally the cross coordinates
    if (nLocations == 2 || nCrosses == 1) {
        guess << crossCoordinates(0,1), crossCoordinates(0,0);
        Logger->log("POSITION GUESS: (%f,%f)", guess[0], guess[1]);
        return guess;
    }

    Logger->log("CALCULATING FIRST POSITION GUESS");

    //loop over all cross pairs without double counting to find the shortest distance and the coordinates of the center of that distance segment
    for(int i=0; i<nCrosses; i++) {
        for(int j=0; j<nCrosses; j++) {
            if (i < j) {
                double crossDistance = sqrt(
                        pow(crossCoordinates(i,0)-crossCoordinates(j,0),2) +
                        pow(crossCoordinates(i,1)-crossCoordinates(j,1),2)
                );
                if (crossDistance < crossMinDistance) {
                    crossMinDistance = crossDistance;
                    crossMeanLat = crossCoordinates(i,0);
                    crossMeanLon = crossCoordinates(i,1);
//                    crossMeanLat = (crossCoordinates(i,0)+crossCoordinates(j,0))/2;
//                    crossMeanLon = (crossCoordinates(i,1)+crossCoordinates(j,1))/2;
                }
                Logger->log("CROSS SEGMENT: %i,%i %f ... (%.3f,%.3f) - (%.3f,%.3f)", i, j, crossDistance,
                            crossCoordinates(i,0),
                            crossCoordinates(i,1),
                            crossCoordinates(j,0),
                            crossCoordinates(j,1)
                            );
            }
        }
    }

    // guess << crossMeanLon, crossMeanLat;
    Logger->log("POSITION GUESS: (%f,%f)", guess[0], guess[1]);
//    std::cout << "POSITION GUESS: " << guess.transpose() << std::endl;

    return guess;
}

void DirectionCrossCollection::analyse() {

    auto nIncluded = count_if(this->begin(), this->end(), [](DirectionCross cross) { return cross.included; });

    if (nIncluded == 2) {
        DirectionCross* closeCross;
        DirectionCross* farCross;
        double minDistance = std::numeric_limits<double>::max();
        double maxDistance = std::numeric_limits<double>::min();
        for(auto& cross: *this) {
            if (cross.included) {
                if (cross.distanceToLocation < minDistance) {
                    minDistance = cross.distanceToLocation;
                    closeCross = &cross;
                }
                if (cross.distanceToLocation > maxDistance) {
                    maxDistance = cross.distanceToLocation;
                    farCross = &cross;
                }
            }
        }

        //first exclude all crosses and directions
        for(DirectionCross &cross: *this) {
            cross.direction1->included = false;
            cross.direction2->included = false;
            cross.included = false;
            cross.includedStatistics = false;
//            cout << cross.distanceToLocation << endl;
        }

//        if (farCross->distanceToLocation > 5000) {
            closeCross->included = true;
            closeCross->direction1->included = true;
            closeCross->direction2->included = true;
//        }
//        else {
//            farCross->included = true;
//            farCross->direction1->included = true;
//            farCross->direction2->included = true;
//        }


//        cout << "2 CROSS ANALYSIS" << endl;
//        cout << "CROSS WITH MIN DISTANCE: " << closeCross->distanceToLocation << " " << closeCross->toString() << endl;
//        cout << "CROSS WITH MAX DISTANCE: " << farCross->distanceToLocation << " " << farCross->toString() << endl;
//        std::vector<DirectionCross> includedCrosses;
//        std::copy_if(this->begin(), this->end(), std::back_inserter(includedCrosses), [](const auto cross) { return cross.included; });
//
//        for(auto cross: includedCrosses) {
//            cout << cross.distanceTo(*(cross.direction1->dfLocation)) << endl;
//            cout << cross.distanceTo(*(cross.direction2->dfLocation)) << endl;
//        }
    }
//    cout << nIncluded << endl;

}


void DirectionCrossCollection::list() {
    Logger->log("LIST OF CROSSES IN COLLECTION: %s", this->name.c_str());
    for (DirectionCross dc : *this) {
//        Logger->log("%s - %s (%f,%f)", to_string((*dc.direction1).id).c_str(), to_string((*dc.direction2).id).c_str(), dc.crossLocation[0], dc.crossLocation[1]);
        Logger->log(dc.toString().c_str());
    }
}

std::string DirectionCrossCollection::toString()
{
    char ss[1000];
    snprintf(ss, 1000, "CROSS COLLECTION, size %i", (int)this->size());
    return std::string(ss);
}

std::string DirectionCrossCollection::toJSON()
{
    std::string jsonArray = "[";
    for (auto cross = this->begin(); cross != this->end(); ++cross) {
        jsonArray += cross->toJSON();
        if (cross != this->end() - 1) jsonArray += ',';
    }
    jsonArray += "]";
    return jsonArray;
}

std::ostream& operator<<(std::ostream &strm, const DirectionCrossCollection &bcc) {
    return strm << "CROSS COLLECTION: " << bcc.size();
}





#include "geolib.h"
#include "LikelihoodTool.h"

static const double inv_sqrt_2pi = 0.398942280401433;

Vector3d rfdfgeo::geo2cart(Vector2d location) {
    double latitude = location[0];
    double longitude = location[1];
    double theta = (90 - latitude) * PI / 180;
    double phi = longitude * PI / 180;
    Vector3d ret;
    ret << sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta);
    return ret;
}

Vector2d rfdfgeo::cart2geo(Vector3d x) {
    x.normalize();
    double phi = PI/2;
    if (x[0] > 0) phi = atan(x[1]/x[0]);
    if (x[0] < 0) phi = atan(x[1]/x[0]) + PI;
    double theta = acos(x[2]);
    double latitude = 90 - (theta * 180 / PI);
    double longitude = phi * 180 / PI;
    Vector2d ret;
    ret << latitude, longitude;
    return ret;
}

double rfdfgeo::addDirection(double b1, double b2) {
    double xCoord = cos(b1*PI/180) + cos(b2*PI/180);
    double yCoord = sin(b1*PI/180) + sin(b2*PI/180);
    return atan(yCoord/xCoord) * 180 / PI;
}

VectorXd rfdfgeo::addDirections(VectorXd b1, VectorXd b2) {
    VectorXd xCoord = (b1*PI/180).array().cos() + (b2*PI/180).array().cos();
    VectorXd yCoord = (b1*PI/180).array().sin() + (b2*PI/180).array().sin();
    VectorXd div = yCoord.cwiseQuotient(xCoord);
    return div.array().atan() * 180 / PI;
}

VectorXd rfdfgeo::subtractDirections(VectorXd b1, VectorXd b2) {
    VectorXd res(b1.rows());
    for (int i = 0; i<b1.rows(); i++) {
        double X1 = cos(b1(i) * PI/180);
        double X2 = cos(b2(i) * PI/180);
        double Y1 = sin(b1(i) * PI/180);
        double Y2 = sin(b2(i) * PI/180);
        res(i) = atan2(X1*Y2-Y1*X2, X1*X2+Y1*Y2) * 180/PI;
    }
    return res;
}

double rfdfgeo::subtractDirections(double b1, double b2) {
    double X1 = cos(b1 * PI/180);
    double X2 = cos(b2 * PI/180);
    double Y1 = sin(b1 * PI/180);
    double Y2 = sin(b2 * PI/180);
    return atan2(X1*Y2-Y1*X2, X1*X2+Y1*Y2) * 180/PI;
}

double rfdfgeo::geodesicInverse(VectorXd p1, VectorXd p2) {
    const Geodesic& geodesic = Geodesic::WGS84();
    double s12, azi1, azi2;
    geodesic.Inverse(p2[1], p2[0], p1[1], p1[0], s12, azi1, azi2);
    if (std::isnan(azi1)) throw DivergentFit();
    if (azi1 < 0) {
        return 360+azi1;
    }
    else {
        return azi1;
    }
}

VectorXd rfdfgeo::geodesicInverse2d(Vector2d position, MatrixXd locations) {
    const Geodesic& geodesic = Geodesic::WGS84();
    double s12, azi1, azi2;
    VectorXd reply(locations.rows());
    for (int i = 0; i < locations.rows(); i++) {
        geodesic.Inverse(locations(i,1), locations(i,0), position(1), position(0), s12, azi1, azi2);
        if (azi1 < 0) {
            reply(i) = 360+azi1;
        }
        else {
            reply(i) = azi1;
        }
    }
    return reply;
}

double rfdfgeo::getDirection(double lat1, double lon1, double lat2, double lon2) {
    Vector2d location1, location2;
    location1 << lat1, lon1;
    location2 << lat2, lon2;
    return geodesicInverse(location2, location1);
}

Matrix3d rfdfgeo::rotationMatrix3d(Vector3d axis, double angle) {
    angle = angle*PI/180;
    double x = axis[0];
    double y = axis[1];
    double z = axis[2];
    Matrix3d rmat;
    rmat << cos(angle)+x*x*(1-cos(angle)),   x*y*(1-cos(angle))-z*sin(angle), x*z*(1-cos(angle))+y*sin(angle),
            y*x*(1-cos(angle))+z*sin(angle), cos(angle)+y*y*(1-cos(angle)),   y*z*(1-cos(angle))-x*sin(angle),
            z*x*(1-cos(angle))-y*sin(angle), z*y*(1-cos(angle))+x*sin(angle), cos(angle)+z*z*(1-cos(angle));
    return rmat;
}


VectorXd rfdfgeo::solveIterationSphere(int nDirections, MatrixXd locationCoord, VectorXd directionMeasured, VectorXd sigma, VectorXd crossGuess, double* angle, double* ev1, double* ev2, MatrixXd* finalEigenvecs) {

    if (nDirections < 1) throw NoIncludedDirectionsException();

    MatrixXd J(nDirections, 2), P, directionDiff(nDirections, 1), N(nDirections, nDirections), eigenvectors;
    VectorXd newDirections(nDirections), crossImproved(2), dx1(2), dx2(2);
    double diff1Lower, diff2Lower, diff1Upper, diff2Upper;

    double dx = 1e-5;
    dx1 << dx/2, 0;
    dx2 << 0, dx/2;

    for (int i=0; i < nDirections; i++) {
        newDirections(i) = rfdfgeo::geodesicInverse(crossGuess, locationCoord.row(i));

        //differentials
        diff1Lower = rfdfgeo::geodesicInverse(crossGuess - dx1, locationCoord.row(i));
        diff1Upper = rfdfgeo::geodesicInverse(crossGuess + dx1, locationCoord.row(i));
        diff2Lower = rfdfgeo::geodesicInverse(crossGuess - dx2, locationCoord.row(i));
        diff2Upper = rfdfgeo::geodesicInverse(crossGuess + dx2, locationCoord.row(i));

//        cout << crossGuess.transpose() << endl;
//        cout << diff1Lower << " " << diff1Upper << " " << diff2Lower << " " << diff2Upper << " " << endl;

        //jacobian
        J(i, 0) = (-diff1Lower + diff1Upper) / dx;
        J(i, 1) = (-diff2Lower + diff2Upper) / dx;

        for (int j=0; j < nDirections; j++) { N(i, j) = 0;}
        N(i,i) = sigma(i)*sigma(i);
    }

    // where the magic happens
    P = (J.transpose() * N.inverse() * J).inverse();
    directionDiff << rfdfgeo::subtractDirections(newDirections, std::move(directionMeasured));
    VectorXd crossDiff = (P * J.transpose() * N.inverse()) * directionDiff;
    crossImproved = crossGuess + crossDiff;
    if (crossImproved[1] < -90) crossImproved[1] = -89;
    if (crossImproved[1] > 90) crossImproved[1] = 89;

    //calculate the ellipse
    SelfAdjointEigenSolver<MatrixXd> eigensolver(P);
    eigenvectors = eigensolver.eigenvectors();

    double theta = atan2(eigenvectors(1,1),eigenvectors(0,1));

    *finalEigenvecs = eigenvectors;

//    std::cout << "J: " << std::endl << J << std::endl;
//    std::cout << "N: " << std::endl << N << std::endl;
//    std::cout << "P: " << std::endl << P << std::endl;
//    std::cout << "Evec: " << std::endl << eigenvectors << std::endl;
//    std::cout << "Eval: " << std::endl << eigensolver.eigenvalues() << std::endl;

    *angle = theta;
    *ev1 = eigensolver.eigenvalues()(0);
    *ev2 = eigensolver.eigenvalues()(1);

    return crossImproved;
}

vector<Vector2d> rfdfgeo::positionEstimate(int nDirections, MatrixXd locationCoord, VectorXd locationDirections, VectorXd sigmas, VectorXd crossGuess, double* angle, double* ev1, double* ev2, int* fitStatus, MatrixXd* finalEigenvecs) {

    VectorXd crossImprovedOld;
    VectorXd crossImproved = crossGuess;
    vector<Vector2d> returnVector;

    *fitStatus = 0;
    for(int iit=0; iit < 20; iit++) {
        crossImprovedOld = crossImproved;
        try {
            crossImproved = rfdfgeo::solveIterationSphere(nDirections, locationCoord, locationDirections, sigmas, crossImproved, angle, ev1, ev2, finalEigenvecs);
            returnVector.push_back(crossImproved);
        }
        catch (DivergentFit &e) {
            Logger->log("DIVERGENT FIT, ENDING RECURSION!");
            *fitStatus = -1;
            return returnVector;
        }
        Logger->log("1ST POSDIFF %i: %f - (%f,%f)", iit, (crossImprovedOld - crossImproved).norm(), crossImproved[0], crossImproved[1]);
        double crossMove = (crossImprovedOld - crossImproved).norm();
        if (crossMove < 0.01) break;
    }

    double chi2 = 0;
    for (int iLocation=0; iLocation < nDirections; iLocation++) {
        double directionMeanOffset = rfdfgeo::addDirection(
                rfdfgeo::getDirection(locationCoord(iLocation, 0), locationCoord(iLocation, 1), crossImproved(0), crossImproved(1)),
                -locationDirections(iLocation));
        chi2 += pow(directionMeanOffset, 2) / sigmas(iLocation);
    }
    Logger->log("CHI2: %f", chi2 / (nDirections - 1));

    return returnVector;
}

map<std::string, vector<double>> rfdfgeo::calculateLikelihood(double lonStart, double latStart, VectorXd measuredDirections, MatrixXd sigmas, MatrixXd locations) {

//    cout << lonStart << " " << latStart << endl;
//    cout << measuredDirections.transpose() << endl;
//    cout << sigmas << endl;
//    cout << locations << endl;

    LikelihoodTool lht(lonStart, latStart, std::move(measuredDirections), std::move(sigmas), std::move(locations));

    map<std::string, vector<double>> returnMap;
    returnMap["lonData"] = lht.lonData;
    returnMap["latData"] = lht.latData;
    returnMap["lonIndex"] = lht.lonIndex;
    returnMap["latIndex"] = lht.latIndex;
    returnMap["lhData"] = lht.lhData;

    return returnMap;
}

VectorXd rfdfgeo::kernelDensity(double mean, double sigma) {
    VectorXd returnValues = VectorXd::Zero(360);
    double theta = 180 + mean - floor(mean);
    int index = -181 + (int)floor(mean);

//    std::string debugString = "RRR ... " + to_string(mean) + "\n";

    for (int direction = 0; direction < 360; direction++) {
        double a = ((double)direction - theta) / sigma;
        int rotatedIndex = (abs(direction + index))%360;
        returnValues[rotatedIndex] = inv_sqrt_2pi / sigma * exp(-0.5 * a * a);
//        debugString += (to_string(direction) + " " + to_string(index) + " " + to_string(rotatedIndex) + " " + to_string(returnValues[rotatedIndex]) + "\n");
    }
    return returnValues;
}

double rfdfgeo::kernelPeaks(VectorXd kernelDensity) {
    double diff, sdiff1, sdiff2, dum, maxDensityDirection = 0;
    double maxDensity = kernelDensity.maxCoeff();
    for (int dir = 0; dir < 362; dir++) {
        int i = dir % 360;
        int ii = (dir + 1) % 360;
        diff = kernelDensity[ii] - kernelDensity[i];
        sdiff2 = sdiff1;
        sdiff1 = (diff < 0) - (diff > 0);
        dum = sdiff2 - sdiff1;
        if ((dum > 0) - (dum < 0) == -1) {
//                std::cout << " " << dir << " " << kernelDensity[i] << " " << maxDensity <<" " << (dum > 0) - (dum < 0) << std::endl;
            if (kernelDensity[i] == maxDensity) maxDensityDirection = (double)dir;
        }
    }

    return maxDensityDirection;
}

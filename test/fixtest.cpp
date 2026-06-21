#include "geolib.h"
#include "fixlib.h"

int main() {

    DfAnalyzer dfa(1);

    double directions[6] = {90, 225, 315, 2, 2, 2};
    double locations[6] = {2.350128676656633, 48.86129501083692, 13.409665746048544, 52.521323096219966, 13.779348093551762, 45.64638482429674};

    dfa.getFixpoint(CIRCULAR ,3, locations, directions);

    // dfa.loadDirections(CIRCULAR ,3, locations, directions);
    // int nEllipsePoints = 50;
    // auto* ellipse = new double[nEllipsePoints*2];
    // auto* firstGuess = new double[2];
    // auto* center = new double[2];
    // double cl;
    // int status = dfa.getMeanPositionEllipse(CROSS_EQUAL, nEllipsePoints, 0.95, ellipse, firstGuess, center, &cl);
    // std::cout << "STATUS ........ " << status << std::endl;
    // std::cout << "CL ............ " << cl << std::endl;
    // std::cout << "BUILD TIME .... " << dfa.getCompilationTime() << std::endl;
}




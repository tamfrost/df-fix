#ifndef WEBASSEMBLY_DFANALYZER_H
#define WEBASSEMBLY_DFANALYZER_H

#include "geolib.h"
#include "DirectionCrossCollection.h"
#include "DfLocation.h"
#include "Bearing.h"

class DfAnalyzer {

private:
    std::map<int, std::string> dfLocationMap; //a map between location id and location name
    std::map<std::string, DfLocation*> dfLocations; //container for dfLocations
    DirectionCrossCollection allCrosses;
    DirectionCrossCollection meanCrosses;
    DirectionCollection allDirections; //container for all directions
    DirectionCollection meanDirections; //container for mean directions
    static void calculateEllipse(int n, Vector2d center, double major, double minor, double theta, MatrixXd finalEigenvecs, double* result);
    void deleteContainerItems();

    template<typename T>
    std::uintptr_t getPointer(T *array) {
        return reinterpret_cast<std::uintptr_t>(array);
    }

public:
    DfAnalyzer();
    explicit DfAnalyzer(int loggingEnabled);
    ~DfAnalyzer();
    void clear();
    void configure(const std::string& jsonConfigString);
    int getMeanPositionEllipse(int collection, CrossErrorModel errorModel, int nPoints, double confidenceLevelIn, double* ellipse, double* firstGuess, double* center, double* confidenceLevelOut, std::string* estimates);
    void getLikelihood(double lat, double lon);
    void loadDirections(DirectionErrorModel directionErrorModel, int nDirections, const double* locationCoordinates, double* directionsPtr);
    std::string getFixpoint(DirectionErrorModel directionErrorModel, int nDirections, const double* locationCoordinates, double* directionsPtr);
    static void logCompilationTime();
    static void logVersion();
    static void logCommitHash();
//    void getDirectionLines(std::string locationTag, int nPoints, double distance, double* mainLine, double* upperLine, double* lowerLine, int* directionId, int* included) ;
    double getDirection(double lat1, double lon1, double lat2, double lon2);
    static void greatCircle(int nPoints, double lat, double lon, double direction, double distance, double* main);
    static std::string getCompilationTime();
    static std::string getVersion();
    static std::string getCommitHash();
    DirectionCrossCollection getCrosses(int collection) const;
    DirectionCrossCollection getMeanCrosses() const;
    std::vector<std::string> getLogMessages() const;

    const map<std::string, DfLocation*> &getDfLocations() const;
    DirectionCollection getMeanDirections() const;

    map<std::string, vector<double>> likelihoodData;
};

#endif //WEBASSEMBLY_DFANALYZER_H

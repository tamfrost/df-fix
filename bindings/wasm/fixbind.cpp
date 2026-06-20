#include <emscripten.h>
#include <emscripten/bind.h>
#include <nlohmann/json.hpp>
#include <stdint.h>
#include "geolib.h"
#include "fixlib.h"

// EM_JS(void, get_commit_hash_resolver, (const char *hash), {
//    getCommitHashResolver(UTF8ToString(hash));
// });

using namespace emscripten;
using json = nlohmann::json;

class RFDFGeo {
private:
    DfAnalyzer* dfAnalyzer;
    double centerLon{}, centerLat{};
public:
    static std::string commitHash;
    static std::string compilationTime;
    RFDFGeo();
    explicit RFDFGeo(int loggingEnabled);
    ~RFDFGeo();
    emscripten::val getFixpoint(int directionErrorModel, int nDirections, uint32_t locationCoordsPtr, uint32_t directionsPtr);
    void loadDirections(int directionErrorModel, int nDirections, uint32_t locationCoordsPtr, uint32_t directionsPtr);
    void getMeanPosition(int crossErrorModel, int collection);
    void getCrosses(int collection);
    void getMeanCrosses();
    void getLikelihoodData();
    void getDfLocations();
    emscripten::val getLog();
    static void logCompilationTime();
    static void logVersion();
    static void logCommitHash();
};

// Static member variable definition
std::string RFDFGeo::commitHash = DfAnalyzer::getCommitHash();
std::string RFDFGeo::compilationTime = DfAnalyzer::getCompilationTime();

RFDFGeo::~RFDFGeo() {
    delete this->dfAnalyzer;
}

RFDFGeo::RFDFGeo() {
    this->dfAnalyzer = new DfAnalyzer();
}

RFDFGeo::RFDFGeo(int loggingEnabled) {
    this->dfAnalyzer = new DfAnalyzer(loggingEnabled);
}

emscripten::val RFDFGeo::getLog() {
    vector<string> messages = this->dfAnalyzer->getLogMessages();
    json jsonArray = json::array();
    for (const auto& message : messages) {
        jsonArray.push_back(message);
    }
    return emscripten::val(jsonArray.dump());
}

emscripten::val RFDFGeo::getFixpoint(int directionErrorModel, int nDirections, uint32_t locationCoordsPtr, uint32_t directionsPtr) {
    double locationCoordinates[2 * nDirections];
    memcpy(locationCoordinates, reinterpret_cast<double*>(locationCoordsPtr), 2 * nDirections * sizeof(double));
    double directions[2*nDirections];
    memcpy(directions, reinterpret_cast<double*>(directionsPtr), 2*nDirections*sizeof(double));
//    this->dfAnalyzer->setCrossErrorModel(static_cast<CrossErrorModel>(crossErrorModel));
    std::string fpJSON = this->dfAnalyzer->getFixpoint(
            static_cast<DirectionErrorModel>(directionErrorModel),
            nDirections,
            locationCoordinates,
            directions
            );
    return emscripten::val(fpJSON);
    // std::cout << fpJSON << std::endl;
    // const char* jsonCharArray = fpJSON.data();
    // return (uintptr_t) jsonCharArray;
}


void RFDFGeo::loadDirections(int directionErrorModel, int nDirections, uint32_t locationCoordsPtr, uint32_t directionsPtr) {
    double locationCoordinates[2 * nDirections];
    memcpy(locationCoordinates, reinterpret_cast<double*>(locationCoordsPtr), 2 * nDirections * sizeof(double));
    double directions[2*nDirections];
    memcpy(directions, reinterpret_cast<double*>(directionsPtr), 2*nDirections*sizeof(double));
//    this->dfAnalyzer->setCrossErrorModel(static_cast<CrossErrorModel>(crossErrorModel));
    this->dfAnalyzer->loadDirections(
            static_cast<DirectionErrorModel>(directionErrorModel),
            nDirections,
            locationCoordinates,
            directions
            );

//    auto meanDirections = dfAnalyzer->getMeanDirections();
//    std::string data = "[";
//    int i = 0;
//    for (auto meanDirection : meanDirections) {
//        data += meanDirection->toJSON();
//        if (i < meanDirections.size() - 1) data += ',';
//        i++;
//    }
//    data += "]";

    auto dfLocations = dfAnalyzer->getDfLocations();
    std::string data = "[";
    int i = 0;
    for (auto dfLocation : dfLocations) {
        data += dfLocation.second->directionsToJSON();
        if (i < dfLocations.size() - 1) data += ',';
        i++;
    }
    data += "]";

    EM_ASM({
             loadDirectionsResolver(JSON.parse(new TextDecoder().decode(HEAPU8.slice($0, $0 + $1))));
           }, &data[0], data.length());
}

void RFDFGeo::getMeanPosition(int crossErrorModel, int collection) {
    int nPoints = 40;
    double ellipsePtr[2*nPoints];
    double firstGuessPtr[2];
    double centerPtr[2];
    double clOut;
    std::string estimates;
    int fitStatus = dfAnalyzer->getMeanPositionEllipse(collection, static_cast<CrossErrorModel>(crossErrorModel), nPoints, 0.95, ellipsePtr, firstGuessPtr, centerPtr, &clOut, &estimates);
    centerLon = centerPtr[0];
    centerLat = centerPtr[1];

    auto meanDirections = dfAnalyzer->getMeanDirections();
    std::string data = "[";
    int i = 0;
    for (auto meanDirection : meanDirections) {
        data += meanDirection->toJSON();
        if (i < meanDirections.size() - 1) data += ',';
        i++;
    }
    data += "]";

    EM_ASM({
            getPositionResolver({
               center:     [...HEAPF64.slice($0>>3, ($0>>3) + 2)],
               firstGuess: [...HEAPF64.slice($1>>3, ($1>>3) + 2)],
               ellipse:    [...HEAPF64.slice($2>>3, ($2>>3) + 2*$3)],
               meanDirections: JSON.parse(new TextDecoder().decode(HEAPU8.slice($4, $4 + $5))),
               estimates: JSON.parse(new TextDecoder().decode(HEAPU8.slice($8, $8 + $9))),
               fitStatus: $6,
               confidenceLevel: $7
            });
           }, centerPtr, firstGuessPtr, ellipsePtr, nPoints, &data[0], data.length(), fitStatus, clOut, &estimates[0], estimates.length());
}


void RFDFGeo::getDfLocations() {
    auto dfLocations = dfAnalyzer->getDfLocations();
    std::string data = "[";
    int i = 0;
    for (auto dfLocation : dfLocations) {
        data += dfLocation.second->directionsToJSON();
        if (i < dfLocations.size() - 1) data += ',';
        i++;
    }
    data += "]";
    EM_ASM({
            getDfLocationsResolver(JSON.parse(new TextDecoder().decode(HEAPU8.slice($0, $0 + $1))));
           }, &data[0], data.length());
}

void RFDFGeo::getCrosses(int collection) {
    std::string data = (dfAnalyzer->getCrosses(collection)).toJSON();
    EM_ASM({
//            console.log(new TextDecoder().decode(HEAPU8.slice($0, $0 + $1)));
            getCrossesResolver(JSON.parse(new TextDecoder().decode(HEAPU8.slice($0, $0 + $1))));
           }, &data[0], data.length());
}


void RFDFGeo::getMeanCrosses() {
    std::string data = (dfAnalyzer->getMeanCrosses()).toJSON();
    EM_ASM({
//            console.log(new TextDecoder().decode(HEAPU8.slice($0, $0 + $1)));
            getMeanCrossesResolver(JSON.parse(new TextDecoder().decode(HEAPU8.slice($0, $0 + $1))));
           }, &data[0], data.length());
}

void RFDFGeo::getLikelihoodData() {
    dfAnalyzer->getLikelihood(centerLon, centerLat);
    EM_ASM({
           getLikelihoodResolver({
               lonData:      [...HEAPF64.slice($0>>3, ($0>>3) + $5)],
               latData:      [...HEAPF64.slice($1>>3, ($1>>3) + $5)],
               lonIndex:     [...HEAPF64.slice($2>>3, ($2>>3) + $5)],
               latIndex:     [...HEAPF64.slice($3>>3, ($3>>3) + $5)],
               lhData:       [...HEAPF64.slice($4>>3, ($4>>3) + $5)]
           });
       },
           &(dfAnalyzer->likelihoodData["lonData"])[0],
           &(dfAnalyzer->likelihoodData["latData"])[0],
           &(dfAnalyzer->likelihoodData["lonIndex"])[0],
           &(dfAnalyzer->likelihoodData["latIndex"])[0],
           &(dfAnalyzer->likelihoodData["lhData"])[0],
           (dfAnalyzer->likelihoodData["lonData"]).size()
           );
}

void RFDFGeo::logCompilationTime() {
    DfAnalyzer::logCompilationTime();
}

void RFDFGeo::logVersion() {
    DfAnalyzer::logVersion();
}

void RFDFGeo::logCommitHash() {
    DfAnalyzer::logCommitHash();
}

/**
Some emscripten class functions
*/
void estimatePosition(int nDirections, uint32_t locationCoordsPtr, uint32_t directionsPtr, int directionErrorModel) {
}

void calculateLikelihood(uint32_t jsonInput) {

    char* jsonString = reinterpret_cast<char *>(jsonInput);

    json ac = json::parse(jsonString);

    auto lon = ac["lon"].get<double>();
    auto lat = ac["lat"].get<double>();
    vector<double> values = ac["values"].get<std::vector<double>>();
    vector<double> sigmas = ac["sigmas"].get<std::vector<double>>();
    std::vector<std::vector<double>> positions = ac["position"].get<std::vector<std::vector<double>>>();

    int nDirections = (int)values.size();

    VectorXd measuredDirections(nDirections);
    MatrixXd sigmaMatrix(nDirections, nDirections); sigmaMatrix.setZero();
    MatrixXd locations(nDirections, 2);
    for (int iRow=0; iRow < nDirections; iRow++) {
        measuredDirections(iRow) = values.at(iRow);
        sigmaMatrix(iRow, iRow) = sigmas.at(iRow) * sigmas.at(iRow);
        locations(iRow,0) = positions.at(iRow).at(0);
        locations(iRow,1) = positions.at(iRow).at(1);
    }

    map<std::string, vector<double>> ret = rfdfgeo::calculateLikelihood(lon, lat, measuredDirections, sigmaMatrix, locations);

    EM_ASM({
        calculateLikelihoodResolver({
                 lonData:      [...HEAPF64.slice($0>>3, ($0>>3) + $5)],
                 latData:      [...HEAPF64.slice($1>>3, ($1>>3) + $5)],
                 lonIndex:     [...HEAPF64.slice($2>>3, ($2>>3) + $5)],
                 latIndex:     [...HEAPF64.slice($3>>3, ($3>>3) + $5)],
                 lhData:       [...HEAPF64.slice($4>>3, ($4>>3) + $5)]
         });
       },
       &(ret["lonData"])[0],
       &(ret["latData"])[0],
       &(ret["lonIndex"])[0],
       &(ret["latIndex"])[0],
       &(ret["lhData"])[0],
       (ret["lonData"]).size()
    );
}

emscripten::val greatCircle(int nPoints, double lat, double lon, double direction, double distance) {
    double gcPtr[2*nPoints];
    DfAnalyzer::greatCircle(nPoints, lat, lon, direction, distance, gcPtr);
    std::string jsonArray = "[";
    for (int i = 0; i < nPoints; i++) {
        jsonArray += "[" + std::to_string(gcPtr[nPoints + i]) + ", " + std::to_string(gcPtr[i]) + "]";
        if (i == nPoints - 1) jsonArray += "]";
        else jsonArray += ",";
    }

    return emscripten::val(jsonArray);
    // const char* jsonCharArray = jsonArray.data();
    // std::cout << jsonCharArray << " " << jsonArray << std::endl;
    // char* jsonCharArray = new char[jsonArray.length() + 1];
    // strcpy(jsonCharArray, jsonArray.c_str());
    // return jsonCharArray;

    // std::cout << "JJJJJJJJJJJ " << (uintptr_t) jsonCharArray << std::endl;
    // return (uintptr_t) jsonCharArray;
}


EMSCRIPTEN_BINDINGS(RfdfGeo) {
    emscripten::function("estimatePosition", &estimatePosition);
    emscripten::function("calculateLikelihood", &calculateLikelihood);
    emscripten::function("greatCircle", &greatCircle);
    register_vector<double>("vector<double>");
    register_map<std::string, std::vector<double>>("map<string, vector<double>>");

    class_<RFDFGeo>("RFDFGeo")
        .constructor<>()
        .constructor<int>()
        .function("getFixpoint", &RFDFGeo::getFixpoint)
        .function("loadDirections", &RFDFGeo::loadDirections)
        .function("getMeanPosition", &RFDFGeo::getMeanPosition)
        .function("getCrosses", &RFDFGeo::getCrosses)
        .function("getMeanCrosses", &RFDFGeo::getMeanCrosses)
        .function("getDfLocations", &RFDFGeo::getDfLocations)
        .function("getLikelihoodData", &RFDFGeo::getLikelihoodData)
        .function("getLog", &RFDFGeo::getLog)
        .class_function("logCompilationTime", &RFDFGeo::logCompilationTime)
        .class_function("logVersion", &RFDFGeo::logVersion)
        .class_function("logCommitHash", &RFDFGeo::logCommitHash)
        .class_property("commitHash", &RFDFGeo::commitHash)
        .class_property("compilationTime", &RFDFGeo::compilationTime)
        ;
}

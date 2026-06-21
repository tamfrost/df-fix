#define BOOST_PYTHON_STATIC_LIB
#define eigen_assert(x) \
    if (!(x)) throw std::runtime_error("Eigen assertion failed: " #x)
#include <iostream>
#include <stdexcept>
#include <vector>
#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "fixlib.h"

namespace py = boost::python;

template<typename T>
inline
std::vector<T> to_std_vector( const py::object& iterable ) {
    return std::vector<T>(py::stl_input_iterator<T>(iterable),py::stl_input_iterator<T>());
}

template<class T>
py::tuple to_tuple(const std::vector<T>& v) {
    py::object get_iter = py::iterator<std::vector<T>>();
    py::object iter = get_iter(v);
    py::tuple tup(iter);
    return tup;
}

typedef std::vector<double> MyList;

class Analyzer {

private:
    DfAnalyzer* dfAnalyzer;

public:

    Analyzer() {
        dfAnalyzer = new DfAnalyzer(1);
    };

    Analyzer(bool enableLogging) {
        dfAnalyzer = new DfAnalyzer(enableLogging ? 1 : 0);
    };

    ~Analyzer() {
        delete dfAnalyzer;
    };

    std::string getCompileTime() {
        return dfAnalyzer->getCompilationTime();
    }

    std::string getVersion() {
        return dfAnalyzer->getVersion();
    }

    std::string getCommitHash() {
        return dfAnalyzer->getCommitHash();
    }

    void loadDirections(int nDirections, py::list& siteCoordinates, py::list& directions) {
        std::vector<double> siteCoordnatesVector = to_std_vector<double>(siteCoordinates);
        std::vector<double> directionsVector = to_std_vector<double>(directions);
        dfAnalyzer->loadDirections(CIRCULAR, nDirections, &siteCoordnatesVector[0], &directionsVector[0]);
    }

    py::dict getFixEllipse(int nPoints, double confidenceInterval) {
        vector<double> ellipseVector(2*nPoints);
        vector<double> firstGuessVector(2);
        vector<double> centerVector(2);
        double clOut;
        //int fixStatus = dfAnalyzer->getMeanPositionEllipse(CROSS_EQUAL, nPoints, confidenceInterval, ellipseVector.data(), firstGuessVector.data(), centerVector.data(), &clOut);
        std::string estimates;
        int fixStatus = dfAnalyzer->getMeanPositionEllipse(0, SIGMA, nPoints, confidenceInterval, ellipseVector.data(), firstGuessVector.data(), centerVector.data(), &clOut,  &estimates);
        py::dict returnDict;
        returnDict["ellipse"] = to_tuple<double>(ellipseVector);
        returnDict["firstGuess"] = to_tuple<double>(firstGuessVector);
        returnDict["center"] = to_tuple<double>(centerVector);
        returnDict["status"] = fixStatus;
        return returnDict;
    }

    void clear() {
        dfAnalyzer->clear();
    }


//    py::dict getDirectionLines(std::string siteTag, int nPoints, double distance) {
//        double* mainLinePtr = new double[2*nPoints];
//        double* lowerLinePtr = new double[2*nPoints];
//        double* upperLinePtr = new double[2*nPoints];
//        int directionId;
//        int included;
//        dfAnalyzer->getDirectionLines(siteTag, nPoints, distance, mainLinePtr, lowerLinePtr, upperLinePtr, &directionId, &included);
//        py::dict returnDict;
////        returnDict["ellipse"] = to_tuple<double>(std::vector<double>(ellipsePtr, ellipsePtr + 2*nPoints));
////        returnDict["firstGuess"] = to_tuple<double>(std::vector<double>(firstGuessPtr, firstGuessPtr + 2));
////        returnDict["center"] = to_tuple<double>(std::vector<double>(centerPtr, centerPtr + 2));
//        return returnDict;
//    }

};

BOOST_PYTHON_MODULE(PYTHON_MODULE_NAME)
{
    using namespace boost::python;

    register_exception_translator<std::runtime_error>(
        [](const std::runtime_error& e) { PyErr_SetString(PyExc_RuntimeError, e.what()); });
    register_exception_translator<std::invalid_argument>(
        [](const std::invalid_argument& e) { PyErr_SetString(PyExc_ValueError, e.what()); });
    register_exception_translator<std::out_of_range>(
        [](const std::out_of_range& e) { PyErr_SetString(PyExc_IndexError, e.what()); });
    register_exception_translator<std::exception>(
        [](const std::exception& e) { PyErr_SetString(PyExc_Exception, e.what()); });

    class_<MyList>("MyList").def(vector_indexing_suite<MyList>() );

    class_<Analyzer>("Analyzer", init<bool>())
    .def("loadDirections", &Analyzer::loadDirections)
    .def("getFixEllipse", &Analyzer::getFixEllipse)
    .def("getCompileTime", &Analyzer::getCompileTime)
    .def("getVersion", &Analyzer::getVersion)
    .def("getCommitHash", &Analyzer::getCommitHash)
    .def("clear", &Analyzer::clear)
    ;
}

// const moduleGenerator = require('../lib/libgeofix_javascript');
const moduleGenerator = require('../lib/fix');

const globalObject = typeof window === 'undefined' ? global : window;

class GeoFix {

    constructor(config, fixModule) {
        this.fixModule = fixModule;
        this.rfdfGeo = new fixModule.RFDFGeo(config.silent ? 0 : 1);
        this.commitHash = fixModule.RFDFGeo.commitHash;
        this.compilationTime = fixModule.RFDFGeo.compilationTime;
    }

    emArray(array) {
        const ptr = this.fixModule._malloc(array.length * 64);
        (new Float64Array(this.fixModule.HEAPU8.buffer, ptr, array.length)).set(array);
        return ptr;
    }

    emString(string) {
        const ptr = this.fixModule._malloc(string.length * 8);
        (new Float64Array(this.fixModule.HEAPU8.buffer, ptr, string.length)).set(string);
        return ptr;
    }

    loadDirections(errorModel, locations, directions) {
        return new Promise((resolve, reject) => {
            globalObject.loadDirectionsResolver = resolve;
            try {
                this.rfdfGeo.loadDirections(Number.parseInt(errorModel), directions.length / 2, this.emArray(locations), this.emArray(directions));
            } catch (error) {
                reject('WASM loading directions crached!');
            }
        })
    }

    getFixpoint(errorModel, locations, directions) {
        try {
            return this.rfdfGeo.getFixpoint(Number.parseInt(errorModel), directions.length / 2, this.emArray(locations), this.emArray(directions));
        } catch (error) {
            reject('WASM individual fix calculation crashed!')
        }
        // return this.fixModule.UTF8ToString(this.rfdfGeo.getFixpoint(Number.parseInt(errorModel), directions.length / 2, this.emArray(locations), this.emArray(directions)));
        // return new Promise((resolve, reject) => {
        //     globalObject.getFixpointResolver = resolve;
        //     this.rfdfGeo.getFixpoint(Number.parseInt(errorModel), directions.length / 2, this.emArray(locations), this.emArray(directions));
        // })
    }

    getPosition(type, errorModel, collection) {
        return new Promise((resolve, reject) => {
            try {
                globalObject.getPositionResolver = resolve;
                if (type === 'all') this.rfdfGeo.getPosition();
                if (type === 'mean') this.rfdfGeo.getMeanPosition(Number.parseInt(errorModel), collection);
            } catch (error) {
                reject('WASM group fix calculation crashed!')
            }
        })
    }

    getCrosses(collection) {
        return new Promise((resolve, reject) => {
            try {
                globalObject.getCrossesResolver = resolve;
                this.rfdfGeo.getCrosses(collection);
            } catch (error) {
                reject('WASM crosss fetching crashed!')
            }
        })
    }

    getMeanCrosses() {
        return new Promise((resolve, reject) => {
            globalObject.getMeanCrossesResolver = resolve;
            this.rfdfGeo.getMeanCrosses();
        })
    }

    getDfLocations() {
        return new Promise((resolve, reject) => {
            globalObject.getDfLocationsResolver = resolve;
            this.rfdfGeo.getDfLocations();
        })
    }

    getLog() {
        return JSON.parse(this.rfdfGeo.getLog());
    }

    getLikelihood() {
        return new Promise((resolve, reject) => {
            globalObject.getLikelihoodResolver = resolve;
            this.rfdfGeo.getLikelihoodData();
        })
    }

    calculateLikelihood(jsonInput) {
        return new Promise((resolve, reject) => {
            const globalObject = self;
            globalObject.calculateLikelihoodResolver = resolve;
            const bufferAddr = this.fixModule._malloc(jsonInput.length + 1);
            this.fixModule.stringToUTF8(jsonInput, bufferAddr, jsonInput.length + 1);
            this.fixModule.calculateLikelihood(bufferAddr);
        })
    }

    greatCircle(nPoints, lat, lon, direction, distance) {
        return JSON.parse(this.fixModule.greatCircle(nPoints, lat, lon, direction, distance));
    }

    logCompilationTime() {
        this.fixModule.RFDFGeo.logCompilationTime();
    }

    logVersion() {
        this.fixModule.RFDFGeo.logVersion();
    }

    logCommitHash() {
        this.fixModule.RFDFGeo.logCommitHash();
    }

    // getCommitHash() {
    //     return new Promise((resolve, reject) => {
    //         globalObject.getCommitHashResolver = resolve;
    //         this.rfdfGeo.getCommitHash();
    //     })
    // }

    // getCommitHashSync() {
    //     return this.rfdfGeo.getCommitHashSync();
    // }

    // getCompilationTime() {
    //     return new Promise((resolve, reject) => {
    //         globalObject.getCompilationTimeResolver = resolve;
    //         this.rfdfGeo.getCompilationTime();
    //     })
    // }
}

function loadGeoFix(config) {
    return new Promise(resolve => {
        moduleGenerator({
            onAbort: function (message) {
                console.log(message);
            },
            onRuntimeInitialized: function () {
                console.log('WASM runtime initialized');
            }
        }).then(fixModule => {
            resolve(new GeoFix(config, fixModule));
        })
    })
}

module.exports = {
    loadGeoFix
}

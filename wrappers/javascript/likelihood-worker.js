// const moduleGenerator = require('./lib/libgeofix_javascript');
const moduleGenerator = require('./lib/fix');
const d3 = require("d3");
// import * as d3array from "d3-array";
// import * as d3interpolate from "d3-interpolate";
// import * as d3scale from "d3-scale";

export class LHCalculator {

    constructor() {
        return new Promise(resolve => {
            moduleGenerator().then(fixModule => {
                console.log('module:', fixModule);
                this.fixModule = fixModule;
                // this.rfdfGeo = new fixModule.RFDFGeo(config.silent ? 0 : 1);
                resolve(this);
            })
        });
    }

    calculateLikelihood(jsonInput) {
        return new Promise((resolve, reject) => {
            self.calculateLikelihoodResolver = resolve;
            const bufferAddr = this.fixModule._malloc(jsonInput.length + 1);
            this.fixModule.stringToUTF8(jsonInput, bufferAddr, jsonInput.length + 1);
            this.fixModule.calculateLikelihood(bufferAddr);
        })
    }

}

(new LHCalculator()).then(lhCalculator => {
    self.lhCalculator = lhCalculator;
    postMessage({type: 'init'});
})


self.addEventListener('message', (message) => {

    if (message.data.command === 'calculate') {

        self.lhCalculator.calculateLikelihood(JSON.stringify(message.data.directions)).then(lhData => {
            const nLon = d3.max(lhData.lonIndex) - d3.min(lhData.lonIndex);
            const nLat = d3.max(lhData.latIndex) - d3.min(lhData.latIndex);
            const minLatIndex = d3.min(lhData.latIndex);
            const minLonIndex = d3.min(lhData.lonIndex);
            const imageDataArray = new Uint8ClampedArray(4*nLon*nLat);
            const colorScale = d3.scaleSequential(d3.extent(lhData.lhData), d3.interpolateRgb('rgba(255,255,255,0)', 'rgba(14,85,54,1.0)'));
            for (let i = 0; i<lhData.lonIndex.length; i++) {
                const color = colorScale(lhData.lhData[i]);
                const rgbArr = color.substring(5, color.length-1).replace(/ /g, '').split(',');
                const lonIndex = lhData.lonIndex[i] - minLonIndex;
                const latIndex = lhData.latIndex[i] - minLatIndex;
                const imageIndex = lonIndex + nLon * (nLat-latIndex);
                imageDataArray[4*imageIndex + 0] = 1*rgbArr[0];
                imageDataArray[4*imageIndex + 1] = 1*rgbArr[1];
                imageDataArray[4*imageIndex + 2] = 1*rgbArr[2];
                imageDataArray[4*imageIndex + 3] = 255*rgbArr[3];
            }

            postMessage({type: 'data', lhData: lhData, imageData: imageDataArray});
        })
    }
})


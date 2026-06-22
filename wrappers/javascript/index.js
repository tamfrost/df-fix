const { loadGeoFix } = require('./src/geofix');

class LikelihoodWorker {
    constructor() {
        
        this.worker = new Worker(new URL(/* webpackChunkName: "likelihood-worker" */'./likelihood-worker.js', import.meta.url));

        this.worker.addEventListener('message', (message) => {
            if (message.data.type === 'init') this.initResolver(this);
            if (message.data.type === 'data') this.dataResolver({data: message.data.lhData, image: message.data.imageData});
            else this.dataRejecter();
        })

        this.initResolver = () => {};
        this.dataResolver = () => {};
        this.dataRejecter = () => {};

        return new Promise(resolve => {
            this.initResolver = resolve;
        })
    }

    calculateLikelihood(directions, ellipse) {
        const args = {
            lon: ellipse.center[0],
            lat: ellipse.center[1],
            sigmas: directions.map(md => md.sigma),
            values: directions.map(md => md.value),
            position: directions.map(md => [md.lon, md.lat])
        }
        return new Promise((resolve, reject) => {
            this.worker.postMessage({command: 'calculate', directions: args});
            this.dataResolver = resolve;
            this.dataRejecter = reject;
        })
    }
}

module.exports = { loadGeoFix, LikelihoodWorker };

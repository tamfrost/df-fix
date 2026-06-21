import './elements/GlobeMapHTMLElement.js';
import * as d3 from 'd3';
import * as libgeo from './libgeo.js';
// import { Bearing } from './lib';

let geofix;
let latestCoordinates = [45,45];

function zipLatLon(linePoints) {
    const mainLat = linePoints.slice(0, linePoints.length/2);
    const mainLon = linePoints.slice(linePoints.length/2, linePoints.length);
    return mainLat.map((lat, i) => [mainLon[i], lat]);
}

function generateBearings(N, latStart, lonStart, latEnd, lonEnd, standardDeviation, mode) {
    
    const directions = [];
    const bearings = [];
    const sigmas = [];
    const siteCoordinates = [];
    
    // const latOffset = d3.randomUniform(-5, 5)();
    // const lonOffset = d3.randomUniform(-5, 5)();
    const latOffset = 0;
    const lonOffset = 0;

    const bearingOffset = mode === 'rogue' ? d3.randomUniform(0, 360)() : 0;

    for (let i = 0; i < N; i++) {
        const direction = libgeo.bearing(latStart, lonStart, latEnd, lonEnd);
        const bearing = direction + d3.randomNormal(0, standardDeviation)() + bearingOffset;
        directions.push({
            "area": {
                "startTime": (Date.now() + d3.randomUniform(0, 10000)()) * 1000000
            },
            "standardDeviation": standardDeviation,
            "bearing": bearing,
            "latitude": latStart + latOffset,
            "longitude": lonStart + lonOffset,
            "siteName": `${latStart}${lonStart}`
        })
        bearings.push(bearing);
        sigmas.push(standardDeviation)
        siteCoordinates.push(lonStart + lonOffset);
        siteCoordinates.push(latStart + latOffset);
    }
    
    return { directions, siteCoordinates, bearings, sigmas };
};

class Direction {
    
    constructor(lat, lon, value, sigma, time, included) {
        this.lat = lat;
        this.lon = lon;
        this.value = value;
        this.sigma = sigma;
        this.time = time;
        this.included = included;
        
        this.onClickHandler = () => {};
    }
    
    getGeoJson(nPoints, distance) {
        const mLine = {type: 'Feature', properties: {}, geometry: {type: 'LineString', coordinates: geofix.greatCircle(nPoints, this.lat, this.lon, this.value, distance*1000)}}
        mLine.properties['classes'] = 'meanline ' +  (this.included ? 'included' : 'excluded');
        mLine.properties['referenceObject'] = this;
        const uLine = {type: 'Feature', properties: {}, geometry: {type: 'LineString', coordinates: geofix.greatCircle(nPoints, this.lat, this.lon, this.value + this.sigma, distance*1000)}}
        const lLine = {type: 'Feature', properties: {}, geometry: {type: 'LineString', coordinates: geofix.greatCircle(nPoints, this.lat, this.lon, this.value - this.sigma, distance*1000)}}
        const sigmaPolygon = lLine;
        sigmaPolygon.properties['classes'] = 'sigmapolygon';
        sigmaPolygon.geometry.type = 'Polygon';
        sigmaPolygon.geometry.coordinates = [[...uLine.geometry.coordinates.reverse(), ...lLine.geometry.coordinates]];
        return {
            "type": "FeatureCollection",
            "features": [sigmaPolygon, mLine]
        };
    }
    
    writeInfo() {
        console.log('INFO ...');
    }
    
    onClick(handler) {
        this.onClickHandler = handler;
    }
    
}

class DirectionCollection extends Array {
    
    constructor(lat, lon) {
        super()
        this.directions = [];
        this.lon = lon;
        this.lat = lat;
        this.coordinates = [lat, lon];
        this.directionMean = null;
        this.directionSigma = null;
        this.directionDensity = [];
        this.maxDirectionDensityValue = 0;
    }
    
    getMeanDirection() {
        this.calculateMean();
        return 90;
    }
    
    getMeanSigma() {
        return 20;
    }
    
    addDirection(value, sigma) {
        this.push(new Direction(this.lat, this.lon, value, sigma, 0, true));
    }
    
    setDirections(values, sigmas) {
        values.forEach((v,i) => {
            this.push(new Direction(this.lat, this.lon, v, sigma[i], 0), true)
        })
    }
    
    clearDirections() {
        this.length = 0;
        this.directions = [];
        this.directionMean = null;
        this.directionSigma = null;
    }
    
    calculateMean() {
        let sumx = 0;
        let sumy = 0;
        let sumWeight = 0;
        let sumSigma = 0;
        
        this.forEach((direction, index) => {
            const theta = -direction.value + 90;
            const weight = 1/direction.sigma;
            
            sumx += weight * Math.cos(theta * Math.PI/180);
            sumy += weight * Math.sin(theta * Math.PI/180);
            sumWeight += weight;
            sumSigma += direction.sigma;
            
            this.directionSigma = sumSigma/this.length/Math.sqrt(this.length);
            this.directionMean = -Math.atan2(sumy,sumx)*180/Math.PI + 90;
            if (this.directionMean < 0) this.directionMean = 360 + this.directionMean;
        })
    }
}

class DirectionLocation extends DirectionCollection {
    
    constructor(name, lat, lon, md) {
        super(lat, lon);
        this.included = true;
        this.name = name;
        this.creationName = name;
        this.directionMean = md;
        this.onClickHandler = () => {};
    }
    
    getGeoJson() {
        return {
            type: 'Feature',
            geometry: {
                type: 'Point',
                coordinates: [this.lon, this.lat]
            },
            properties: {
                classes: `directionlocation${this.included ? '' : ' excluded'}`,
                referenceObject: this,
                label: this.name,
            }
        }
    }
    
    writeInfo() {
        console.log('INFO ...', this.name);
    }
    
    onClick(handler) {
        this.onClickHandler = handler;
    }
}

class DirectionCross {
    
    constructor(lat, lon, direction1, direction2) {
    }
    
    onClick(handler) {
        this.onClickHandler = handler;
    }
    
    writeInfo() {
        console.log('INFO ...');
    }
    
    
}

class DirectionCrossCollection {
}

let directionLocations = {}, allSiteCoordinates = [], allBearingsAndSigmas = [];
const labelOffset = [{'x':-6, 'y':0},{'x':20, 'y':0},{'x':-6, 'y':20},{'x':20, 'y':20}];
const bearingLength = 10000;
const CEMstandardDeviation = 0;
const CEMcrossequal = 1;
const CEMcrossWeighted = 2;

let collection = 0;
let dfData = null;

class DfData {

    constructor(data, filterHandler) {
        return new Promise((resolve, reject) => {
            this.rawData = data;
            this.directionLocations = [];
            this.filterHandler = filterHandler;

            const geofixFormattedData = this.reshapeRawData();

            //initial data generation
            geofix.loadDirections("0", geofixFormattedData.coordinates, geofixFormattedData.bearingsAndSigmas).then(async (locations) => {
                this.locations = locations;

                this.directionLocations = locations.map((l, index) => {
                    const dl = new DirectionLocation(l.name, l.lat, l.lon, new Direction(l.mean.lat, l.mean.lon, l.mean.value, l.mean.sigma, 0, true));
                    console.log(index, 'l', l);
                    console.log(index, 'dl', dl);
                    dl.directionDensity = l.density;
                    dl.maxDirectionDensityValue = l.maxDensityDirection;
                    l.directions.forEach(d => {dl.push(new Direction(l.lat, l.lon, d.value, d.sigma, 0, true))});
                    dl.onClick(async (selectedDl) => {
                        this.applyFilter({locations: [selectedDl]});
                        this.update();
                    });
                    return dl;
                });
                this.update();
                resolve(this);
            })
        });
    }

    async update() {
        let fixFeatures = null;
        try {
            fixFeatures = await this.calculateFix(collection);
        }
        catch(e) {
            console.warn(e);
            // return false;
        }
        const bearingFeatures = this.getBearingFeatureCollection();
        this.filterHandler({bearingFeatures, fixFeatures});
        // return true
    }

    applyFilter(filter) {
        this.directionLocations.forEach(dl => {
            if (filter.locations.includes(dl)) dl.included = !dl.included;
            dl.forEach(d => {
                if (filter.locations.includes(dl)) d.included = !d.included;            
            });
        })
    }

    reshapeLocationData() {
        const vectors = Object.values(this.directionLocations).reduce((res, dl) => {
            dl.filter(d => d.included).forEach(d => {
                res.coordinates.push(dl.lon);
                res.coordinates.push(dl.lat);
                res.bearings.push(d.value);
                res.sigmas.push(d.sigma);
           })
           return res;
        }, {coordinates: [], bearings: [], sigmas: []});    
        return {
            coordinates: vectors.coordinates,
            bearingsAndSigmas: [...vectors.bearings, ...vectors.sigmas],
        }    
    }

    reshapeRawData() {
        return {
            coordinates: this.rawData.reduce((res, d) => {
                res.push(d.longitude);
                res.push(d.latitude);
                return res;
            }, []),
            bearingsAndSigmas: [...this.rawData.map(d => d.bearing), ...this.rawData.map(d => d.standardDeviation)],
        }
    }

    getBearingFeatureCollection() {
        return  Object.values(this.directionLocations)
        .reduce((res, dl) => {
            const locationFeature = dl.getGeoJson();
            res.featureCollection.features.push(locationFeature);
            dl.forEach(d => {
                res.featureCollection.features.push(d.getGeoJson(10, bearingLength).features[1]);
            });
            return res;
        },{
            featureCollection: {type: 'FeatureCollection', features: []}
        });
    }    

    async calculateFix(collection) {
        return new Promise((resolve, reject) => {
            const data = this.reshapeLocationData();
            if (collection === 0) {
                geofix.loadDirections("0", data.coordinates, data.bearingsAndSigmas).then(() => {
                    geofix.getPosition('mean', CEMstandardDeviation, 0)
                    .then(async (ellipseData) => {
                        this.crosses = await geofix.getCrosses(collection);
                        console.log('ELLIPSE DATA0', ellipseData);
                        const logString = geofix.getLog().join('\n');
                        applog.innerHTML = `<pre style="margin-top: 0px; overflow: hidden;">${logString}</pre>`;
                        resolve(this.getFixFeatureCollections(ellipseData));
                    })
                    .catch((message) => {
                        reject(message);
                    })
                })
            }
            else {
                geofix.getFixpoint("0", data.coordinates, [...data.bearingsAndSigmas]);
                geofix.getPosition('mean', CEMstandardDeviation, 1)
                .then(async (ellipseData) => {
                    this.crosses = await geofix.getCrosses(collection);
                    console.log('ELLIPSE DATA1', ellipseData);
                    const logString = geofix.getLog().join('\n');
                    applog.innerHTML = `<pre style="margin-top: 0px;">${logString}</pre>`;
                    resolve(this.getFixFeatureCollections(ellipseData));
                })
                .catch((message) => {
                    reject(message);
                })
        }
        });
    }

    async getFixFeatureCollections(ellipseData) {

        console.log('ELLIPSE DATA', ellipseData);
        
        const fixFeatures = {};

        fixFeatures.meanDirections = ellipseData.meanDirections
            .map(md => new Direction(md.lat, md.lon, md.value, md.sigma, 0, md.included))
            .map(MD => {
                // mD.included = true;
                
                const featureCollection = MD.getGeoJson(30,bearingLength);
                featureCollection.included = MD.included;
                return (featureCollection);
        });
            
        if (ellipseData.fitStatus === -2) return fixFeatures;

        const ellipseFeatureCollection = {
            type: 'FeatureCollection',
            features: [
                {
                    type: 'Feature',
                    geometry: {
                        type: "Polygon",
                        coordinates: [zipLatLon(ellipseData.ellipse)]
                    },
                    properties: {
                        classes: 'ellipse'
                    }
                }
            ]
        }
        ellipseData.estimates.forEach((e, index) => {
            ellipseFeatureCollection.features.push({
                type: 'Feature',
                geometry: {
                    type: "Point",
                    coordinates: e.reverse()
                },
                properties: {
                    classes: 'estimate'
                }
            })
        })
        ellipseFeatureCollection.features.push({
            type: 'Feature',
            geometry: {
                type: "Point",
                coordinates: ellipseData.firstGuess.reverse()
            },
            properties: {
                classes: 'guess1'
            }
        })
        ellipseFeatureCollection.features.push({
            type: 'Feature',
            geometry: {
                type: "Point",
                coordinates: ellipseData.center.reverse()
            },
            properties: {
                classes: 'center'
            }
        })
        console.log('ELLIPSE FEATURE COLLECTION', ellipseFeatureCollection);
        
        fixFeatures.ellipse = ellipseFeatureCollection;
    
        if (ellipseData.fitStatus === -1) return fixFeatures;
        fixFeatures.crosses = (this.crosses).reduce((res, cross) => {
            res.features.push(
                {
                    type: 'Feature',
                    geometry: {
                        type: 'Point',
                        coordinates: cross.coordinates[0].reverse()
                    },
                    properties: {
                        classes: 'directioncross primarycross ' + (cross.included ? 'included' : 'excluded'),
                        reference: '',
                        // label: `N ${cross.primaryIndex[0]}-${cross.primaryIndex[1]}`
                        // label: `N ${window.locationNameMap[cross.direction1.direction.name]}-${window.locationNameMap[cross.direction2.direction.name]}`
                    }
                },
                {
                    type: 'Feature',
                    geometry: {
                        "type": 'Point',
                        coordinates: cross.coordinates[1].reverse()
                    },
                    properties: {
                        classes: 'directioncross secondarycross ' + (cross.included ? 'included' : 'excluded'),
                        reference: '',
                        // label: `S ${cross.primaryIndex[0]}-${cross.primaryIndex[1]}`
                        // label: `S ${window.locationNameMap[cross.direction1.direction.name]}-${window.locationNameMap[cross.direction2.direction.name]}`
                    }
                }
            )
            return res;
        }, {type: 'FeatureCollection', features: []})

        return fixFeatures;

    }

    onFilter(handler) {
        this.filterHandler = handler;
    }
}

async function generateData(coordinates) {

    latestCoordinates = coordinates;
    console.log('coordinates:', coordinates);

    const trueFixFeatureCollection = {
        type: 'FeatureCollection',
        features: [
            {
                type: 'Feature',
                geometry: {
                    type: "Point",
                    coordinates: latestCoordinates
                },
                properties: {
                    classes: 'estimate'
                }
            }
        ]
    }

    let rawData = [];
    try {
        if (generatetype.value === 'normal') {
            const shots1 = generateBearings(3, 53, 20, coordinates[1], coordinates[0], 5, 'normal');
            const shots2 = generateBearings(3, 45, 18, coordinates[1], coordinates[0], 5, 'normal');
            const shots3 = generateBearings(3, 49, 22, coordinates[1], coordinates[0], 5, 'normal');
            rawData = [...shots1.directions, ...shots2.directions, ...shots3.directions];
        }       
        if (generatetype.value === 'normalpartner') {
            const shots1 = generateBearings(3, 53, 20, coordinates[1], coordinates[0], 5, 'normal');
            const shots2 = generateBearings(3, 45, 18, coordinates[1], coordinates[0], 5, 'normal');
            const shots3 = generateBearings(3, 49, 22, coordinates[1], coordinates[0], 5, 'normal');
            const shots4 = generateBearings(3, 25, 16, coordinates[1], coordinates[0], 5, 'normal');
            rawData = [...shots1.directions, ...shots2.directions, ...shots3.directions, ...shots4.directions];
        }       
        if (generatetype.value === 'roughpartner') {
            const shots1 = generateBearings(3, 53, 20, coordinates[1], coordinates[0], 5, 'normal');
            const shots2 = generateBearings(3, 45, 18, coordinates[1], coordinates[0], 5, 'normal');
            const shots3 = generateBearings(3, 49, 22, coordinates[1], coordinates[0], 5, 'normal');
            const shots4 = generateBearings(3, 25, 16, coordinates[1], coordinates[0], 5, 'rogue');
            rawData = [...shots1.directions, ...shots2.directions, ...shots3.directions, ...shots4.directions];
        }       
        if (generatetype.value === 'clipboard') {
              rawData = JSON.parse(await navigator.clipboard.readText());
        }       
    }
    catch(e) {
        if (e.message.includes('JSON at position')) alert('JSON format error!');
        return;
    }

    // dfData = await new DfData([...shots1.directions, ...shots2.directions, ...shots3.directions], (filteredFeatures) => {
    dfData = await new DfData(rawData, (filteredFeatures) => {
        directionmap.clear();
        if (generatetype.value !== 'clipboard') directionmap.drawFeatureCollection(trueFixFeatureCollection, 'truefix');
        directionmap.drawFeatureCollection(filteredFeatures.bearingFeatures.featureCollection, 'directionlocations');
        if (filteredFeatures.fixFeatures) {
            fitstatus.innerHTML='';
            directionmap.drawFeatureCollection(filteredFeatures.fixFeatures.crosses, 'directioncrosses');
            directionmap.drawFeatureCollection(filteredFeatures.fixFeatures.ellipse, 'positionellipse');
            filteredFeatures.fixFeatures.meanDirections.forEach((mdFeatureCollection) => {
                // mdFeatureCollection.included = true;
                directionmap.drawFeatureCollection(mdFeatureCollection, `directionlines meandirection ${mdFeatureCollection.included ? 'included' : 'excluded'}`);
            })
        }
        else {
            fitstatus.innerHTML='Fix calculation failed!';
        }
    });
    console.log(dfData);
    return dfData
}

(async () => {

    const radioButtons = document.querySelectorAll('input[name="fittype"]');
    radioButtons.forEach(radio => {
        radio.addEventListener('change', () => {
            const selectedValue = document.querySelector('input[name="fittype"]:checked').value;
            collection = selectedValue === 'group' ? 0 : 1;
            console.log(collection);
            if (dfData) {
                dfData.update();
            }
        });
    });

    clipboardCopy.addEventListener('click', async () => {
        collection = document.querySelector('input[name="fittype"]:checked').value === 'group'? 0 : 1;
        generateData(latestCoordinates);
    })

    // const {loadGeoFix, LikelihoodWorker} = await import('../publish');
    const {loadGeoFix, LikelihoodWorker} = await import('../.');
    
    directionmap.onMapRightClick((coordinates) => {
        generateData(coordinates);
    })
    
    loadGeoFix({ silent: false }).then(geoWasmInterface => {
        geofix = geoWasmInterface;
        geofix.logCompilationTime();
        console.log(geofix.commitHash);
        console.log(geofix.compilationTime);
        console.log(geofix.getLog());
        commithash.innerHTML = geofix.commitHash;
        buildtime.innerHTML = geofix.compilationTime;
    });
})();


// const bearinginputValue = '[{"lat":24.745555555555555,"lon":9.4375,"dir":110.0,"std":1.0},{"lat":24.76444444444444,"lon":9.448611111111113,"dir":115.0,"std":1.0},{"lat":24.76722222222222,"lon":9.450833333333332,"dir":115.0,"std":1.0},{"lat":24.844722222222224,"lon":9.49361111111111,"dir":128.0,"std":1.0},{"lat":24.84,"lon":9.494722222222222,"dir":129.0,"std":1.0},{"lat":24.83888888888889,"lon":9.495,"dir":129.0,"std":1.0},{"lat":24.831388888888895,"lon":9.496944444444445,"dir":130.0,"std":1.0},{"lat":24.82222222222222,"lon":9.498055555555556,"dir":128.0,"std":1.0},{"lat":24.819722222222225,"lon":9.498611111111114,"dir":128.0,"std":1.0},{"lat":24.81722222222223,"lon":9.499444444444446,"dir":127.0,"std":1.0},{"lat":24.80638888888888,"lon":9.503333333333334,"dir":132.0,"std":1.0},{"lat":24.7975,"lon":9.505,"dir":356.0,"std":1.0},{"lat":24.79361111111111,"lon":9.505,"dir":109.0,"std":1.0},{"lat":24.793055555555554,"lon":9.504722222222224,"dir":105.0,"std":1.0},{"lat":24.75361111111111,"lon":9.46388888888889,"dir":112.0,"std":1.0},{"lat":24.75222222222222,"lon":9.462777777777777,"dir":113.0,"std":1.0},{"lat":24.75138888888889,"lon":9.461944444444445,"dir":112.0,"std":1.0},{"lat":24.74333333333333,"lon":9.453333333333333,"dir":22.0,"std":1.0},{"lat":24.74277777777778,"lon":9.4525,"dir":25.0,"std":1.0},{"lat":24.74277777777778,"lon":9.4525,"dir":25.0,"std":1.0},{"lat":24.72527777777778,"lon":9.434722222222224,"dir":107.0,"std":1.0},{"lat":24.724722222222226,"lon":9.434166666666666,"dir":109.0,"std":1.0}]';
const bearinginputValue = `[
{"lat":53,"lon":28,"dir":110,"std":1.0},
{"lat":45,"lon":18,"dir":90,"std":1.0},
{"lat":38,"lon":15,"dir":80,"std":1.0}
]`;

// bearinginput.value = '[{"lat":24.75361111111111,"lon":9.46472222222222,"dir":113.0,"std":1.0},{"lat":24.78,"lon":9.491111111111113,"dir":9.0,"std":1.0},{"lat":24.78472222222222,"lon":9.496111111111112,"dir":120.0,"std":1.0},{"lat":24.78777777777778,"lon":9.499166666666667,"dir":121.0,"std":1.0},{"lat":24.788333333333334,"lon":9.5,"dir":121.0,"std":1.0},{"lat":24.79416666666666,"lon":9.505,"dir":123.0,"std":1.0},{"lat":24.811388888888885,"lon":9.500555555555554,"dir":126.0,"std":1.0},{"lat":24.818333333333335,"lon":9.499166666666667,"dir":126.0,"std":1.0},{"lat":24.821111111111115,"lon":9.498611111111114,"dir":127.0,"std":1.0},{"lat":24.825,"lon":9.498055555555556,"dir":128.0,"std":1.0},{"lat":24.83972222222222,"lon":9.496111111111112,"dir":137.0,"std":1.0},{"lat":24.84527777777778,"lon":9.494444444444447,"dir":129.0,"std":1.0},{"lat":24.85333333333333,"lon":9.492222222222225,"dir":132.0,"std":1.0},{"lat":24.82944444444445,"lon":9.496666666666666,"dir":113.0,"std":1.0},{"lat":24.79527777777778,"lon":9.504722222222224,"dir":121.0,"std":1.0}]';
// bearinginput.value = '[{"lat":24.79416666666666,"lon":9.505,"dir":123.0,"std":1.0},{"lat":24.811388888888885,"lon":9.500555555555554,"dir":126.0,"std":1.0},{"lat":24.818333333333335,"lon":9.499166666666667,"dir":126.0,"std":1.0}]';    
// bearinginput.value = '[{"lat":24.79416666666666,"lon":9.505,"dir":123.0,"std":1.0},{"lat":24.811388888888885,"lon":9.500555555555554,"dir":126.0,"std":1.0}]';    


// let geofix;

// (async () => {
    
//     console.log('');


// })();


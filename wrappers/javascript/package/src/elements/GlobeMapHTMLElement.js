import * as d3 from "d3";
import {geoGraticule, geoOrthographic, geoPath, geoEquirectangular, geoMercator} from "d3-geo";
import world from "./world.json";
import * as topojson from "topojson";
import geoZoom from "d3-geo-zoom";

class GeoFixMap extends HTMLElement {

    constructor(props) {
        super(props)
        this.width = 700;
        this.height = 700;
        this.directionLocations = null;
        this.shotOpacity = 0.2;
        this.mapClickHandler = () => {};
        this.mapRightClickHandler = () => {};
        this.labelMouseOverHandler = () => {};
        this.labelMouseOutHandler = () => {};
    }

    setShotOpacity(value) {
        this.shotOpacity = value;
        this.shadow.querySelectorAll('.directionlocations path.meanline').forEach(shot => {
            shot.style['stroke-opacity'] = this.shotOpacity;
        })
    }

    showCrosses(value) {
        this.shadow.querySelectorAll('.directioncrosses text').forEach(label => {
            label.style['display'] = value ? 'block' : 'none';
        })
    }

    drawFeatureCollection() {}

    reset() {}

    select() {}

    clear() {}

    onMapClick(handler) {
        this.mapClickHandler = handler;
    }

    onMapRightClick(handler) {
        this.mapRightClickHandler = handler;
    }

    onLabelMouseOver(handler) {
        this.labelMouseOverHandler = handler;
    }

    onLabelMouseOut(handler) {
        this.labelMouseOutHandler = handler;
    }

}

export class GeofixDirectionMap extends GeoFixMap {

    constructor(props) {
        super(props);
        this.scale = 220;
        this.rotation = [-20, -60, 0];
        // this.rotation = [0, 0, 0];
    }

    connectedCallback() {
        const template = /*html*/`
            <style>
                #mapdiv {
                    position: relative;
                    width: 100%;
                    height: 100%;
                    /* padding: 10px; */
                }
                #mapdiv > #infodiv {
                    /*border: 1px solid black;*/
                    width: 200px;
                    height: 50px;
                    font-size: 14px;
                    font-weight: bold;
                    position: absolute;
                    display: none;
                }
                #mapdiv > #mapsvg text {cursor: pointer}
                #mapdiv > #mapsvg {
                    width: 100%;
                    height: 100%;
                }
                .land {fill: none; opacity: 0.5; stroke: #a093b1; stroke-opacity: 1;}
                .countries path {stroke: #a093b1; stroke-linejoin: round; stroke-width: 0.2; fill: none; pointer-events: none;}
                .graticule {fill: none; stroke: #4f2291; stroke-width: 0.5; opacity: 0.2;}
                .earth-circle {stroke: #4f2291;stroke-width: 0.5;opacity: 0.5}
                .directionlines path {fill: none; stroke: #002aff; stroke-opacity: 0.25}
                .directionlocations path.meanline.included {fill: none; stroke: #002aff; stroke-opacity: ${this.shotOpacity}; stroke-width: 1.0; cursor: pointer}
                .directionlocations path.meanline.excluded {fill: none; stroke: #626262; stroke-opacity: ${this.shotOpacity}; stroke-width: 1.0; cursor: pointer}
                .directionlocations text {stroke: darkgreen; stroke-width: 7px; stroke-linejoin:round; fill: white; paint-order:stroke; stroke-opacity: 1;}
                .directionlocations text.excluded {stroke: #999999;}
                .shotsigma path {fill: #002aff; stroke: none; fill-opacity: 0.2;}
                .meandirection path {fill: none; stroke-opacity: 1;}
                .meandirection.included path.sigmapolygon {fill: #0e5536; stroke: none; fill-opacity: 0.1; pointer-events: none}
                .meandirection.included path.meanline {fill: none; stroke: #0e5536;}
                .meandirection.excluded path.sigmapolygon {fill: #ff0000; stroke: none; fill-opacity: 0.1; pointer-events: none}
                .meandirection.excluded path.meanline {fill: none; stroke: #ff0000;}
                .directioncrosses {fill: #0e5536; stroke: #0e5536; stroke-opacity: 1; stroke-width: 2px; cursor: pointer}
                .directioncrosses path.excluded {fill: #ff0000; stroke: #ff0000}
                .directioncrosses text {stroke: none; fill: #000000; stroke-opacity: 1; stroke-width: 1px; display: none}
                /*.includeddirection path {stroke: #116000; stroke-opacity: 1; stroke-width: 1px;}*/
                .likelihoodpolygon {fill: none; stroke: red; stroke-opacity: 1;}
                .positionellipse .ellipse {fill: #de6b00; stroke: #de6b00; fill-opacity: 0; stroke-width: 2px; pointer-events: none}
                .positionellipse .center {fill: #de6b00; stroke: #de6b00; stroke-opacity: 1; stroke-width: 10px}
                .positionellipse .estimate {fill: #de6b00; stroke: #de6b00; stroke-opacity: 1; stroke-width: 3px}
                .positionellipse .guess1 {fill: #0e5536; stroke: #0e5536; stroke-opacity: 0.5; stroke-width: 10px}
                .truefix{fill:rgb(0, 0, 0); stroke: rgb(0, 0, 0); stroke-opacity: 0.5; stroke-width: 10px}
            </style>
            <div id="mapdiv">
                <div id="infodiv"></div>
                <svg id="mapsvg"></svg>
            </div>
        `
        
        this.style.position = 'relative';
        this.style.display = 'block';
        this.style.width = '100%';
        this.style.height = '100%';

        this.shadow = this.attachShadow({mode: 'open'});
        this.shadow.innerHTML = template;
        this.mapdiv = this.shadow.querySelector('#mapdiv');
        this.mapsvg = this.shadow.querySelector('#mapsvg');
        this.infodiv = this.shadow.querySelector('#infodiv');

        this.svg = d3.select(this.mapsvg);//.attr("width", this.width).attr("height", this.height).html("");
        this.mainGroup = this.svg.append("g").attr('class','main-group');

        this.svg.on("click", (event) => {
            this.mapClickHandler(this.proj.invert(d3.pointer(event)));
        });

        this.svg.on('contextmenu', (event) => {
            event.preventDefault();
            this.mapRightClickHandler(this.proj.invert(d3.pointer(event)));
            // this.mapRightClickHandler(olproj.transform(this.map.getEventCoordinate(event), 'EPSG:3857', 'EPSG:4326'));
        });

        (new ResizeObserver((entry) => {
            this.width = entry[0].contentRect.width;
            this.height = entry[0].contentRect.height;
            this.reset();
            // this.svg = d3.select(this.mapsvg).attr("width", entry[0].contentRect.width).attr("height", entry[0].contentRect.height).html("");
        })).observe(this.mapdiv);

        // Use parent element size if available, otherwise fallback to 700
        this.width = this.mapdiv.clientWidth || 700;
        this.height = this.mapdiv.clientHeight || 700;

        this.projectionSetup();

        this.pathGenerator = geoPath(this.proj).pointRadius(1.5);

        this.drawGlobe(world);
    }

    projectionSetup() {
        // this.proj = geoMercator()
        this.proj = geoOrthographic()
            .clipAngle(90)
            .scale(this.scale)
            .translate([this.width / 2, this.height / 2])
            .rotate(this.rotation);
    }

    drawPath(feature, classes, display) {
        return this.mainGroup
            .append("g")
            .attr("style", `display: ${display ? 'block' : 'none'}`)
            .attr("class", classes)
            .selectAll("path")
            .data([feature])
            .enter()
            .append("path")
            .attr("d", this.pathGenerator);
    }

    drawFeatureCollection(featureCollection, classes) {

        console.log('drawFeatureCollection', featureCollection, classes);
        

        this.mainGroup
            .append("g")
            .attr("class", classes)
            .selectAll("path")
            .data(featureCollection.features)
            .enter()
            .append("path")
            .attr("class", f => f.properties.classes)
            .attr("d", this.pathGenerator)
            .on('mouseover', (event, feature) => {
                if (feature.properties.referenceObject && event.ctrlKey) {
                    const shotPath = event.target;
                    shotPath.style['stroke-width'] = '2.0';
                    const shotPolygon = this.drawPath(feature.properties.referenceObject.getGeoJson().features[0], 'shotsigma', true);
                    setTimeout(() =>{
                        shotPath.style['stroke-width'] = '1.0';
                        shotPolygon.node().parentElement.remove();
                    },500)
                }
            })

        this.mainGroup
            .append("g")
            .attr("class", classes)
            .attr("font-size", "12px")
            .selectAll("text")
            .data(featureCollection.features.filter(f => f.properties.label))
            .enter()
            .append("text")
            .on('mouseover', (event, feature) => {
                if (feature.properties.referenceObject) {
                    feature.properties.referenceObject.writeInfo();
                }
            })
            .on('click', function(event, feature) {
                if (feature.properties.referenceObject) {
                    feature.properties.referenceObject.onClickHandler(feature.properties.referenceObject);
                }
            })
            .attr("class", f => f.properties.classes)
            .attr("x", f => {
                return isNaN(this.pathGenerator.centroid(f)[0]) ? null : this.pathGenerator.centroid(f)[0] - 15;
            })
            .attr("y", f => {
                return isNaN(this.pathGenerator.centroid(f)[1]) ? null : this.pathGenerator.centroid(f)[1] - 3;
            })
            .text(f => f.properties.label)
            .each(function(feature) {
                if (feature.properties.referenceObject) {
                    feature.properties.referenceObject.referenceNode = this;
                }
            })
        ;

        // const labels = this.shadow.querySelectorAll('#mapdiv > #mapsvg text');
        // labels.forEach(label => {
        //     label.addEventListener('mouseover', (event) => {
        //         console.log(event.target);
        //     })
        //     console.log(label);
        // })

    }

    clear() {
        try {
            this.mainGroup.selectAll('.meanline').remove();
            this.mainGroup.selectAll('.directionlocations').remove();
            this.mainGroup.selectAll('.directionlines').remove();
            this.mainGroup.selectAll('.directioncrosses').remove();
            this.mainGroup.selectAll('.likelihoodpolygon').remove();
            this.mainGroup.selectAll('.positionellipse').remove();
            this.mainGroup.selectAll('.truefix').remove();
        }
        catch (e) {}
    }

    drawGlobe(world) {
        let graticule = geoGraticule();
        graticule.step([30,30]);

        this.mainGroup
            .append("path")
            .datum(graticule)
            .attr("class", "graticule")
            .attr("d", this.pathGenerator);

        this.mainGroup
            .append("path")
            .data(topojson.feature(world, world.objects.land).features)
            .attr("class", "land")
            .attr("d", this.pathGenerator);

        this.mainGroup
            .append("g")
            .attr("class", "countries")
            .selectAll("path")
            .data(topojson.feature(world, world.objects.countries).features)
            .enter()
            .append("path")
            .attr("d", this.pathGenerator);

        this.earthCircle = this.mainGroup
            .append("circle")
            .attr("cx", this.width / 2)
            .attr("cy", this.height / 2)
            .attr("r", this.proj.scale())
            .attr("class", "earth-circle noclicks")
            .attr("fill", "none");

        this.zoomSetup();
    }

    zoomSetup() {
        geoZoom()
            .projection(this.proj)
            .onMove((f) => this.refresh())(this.svg.node());
    }

    refresh() {
        this.mainGroup.selectAll(".earth-circle").attr("r", this.proj.scale());
        this.mainGroup.selectAll(".graticule").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".land").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".countries path").attr("d", this.pathGenerator);

        this.mainGroup.selectAll(".directionlocations path").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".directionlocations text").attr("x", (f) => isNaN(this.pathGenerator.centroid(f)[0]) ? null : (this.pathGenerator.centroid(f)[0] - 15));
        this.mainGroup.selectAll(".directionlocations text").attr("y", (f) => isNaN(this.pathGenerator.centroid(f)[1]) ? null : (this.pathGenerator.centroid(f)[1] - 3));
        this.mainGroup.selectAll(".shotsigma path").attr("d", this.pathGenerator);

        this.mainGroup.selectAll(".directionline path").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".meandirection path").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".positionellipse path").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".likelihoodpolygon path").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".directioncrosses path").attr("d", this.pathGenerator);
        this.mainGroup.selectAll(".directioncrosses text").attr("x", (f) => isNaN(this.pathGenerator.centroid(f)[0]) ? null : (this.pathGenerator.centroid(f)[0] - 15));
        this.mainGroup.selectAll(".directioncrosses text").attr("y", (f) => isNaN(this.pathGenerator.centroid(f)[1]) ? null : (this.pathGenerator.centroid(f)[1] - 3));
        this.mainGroup.selectAll(".truefix path").attr("d", this.pathGenerator);
        // svg.selectAll(".directionlocation path").attr("d", pathGenerator);
        // svg.selectAll(".coordinatepoint path").attr("d", pathGenerator);
        // svg.selectAll(".directionlocationlabel text").attr("x", (d) => { return pathGenerator.centroid(d)[0]; });
        // svg.selectAll(".directionlocationlabel text").attr("y", (d) => { return pathGenerator.centroid(d)[1]; });
    }

    reset() {

        // const svgWidth = this.mapsvg.clientWidth;
        // const mapWidth = this.shadow.querySelector('.main-group').getBBox().width;
        // const svgHeight = this.mapsvg.clientHeight;
        // const mapHeight = this.shadow.querySelector('.main-group').getBBox().height;
        // const currentScale = this.proj.scale();
        // const newScale = currentScale * svgWidth / mapWidth;

        this.proj.translate([this.width / 2, this.height / 2]);

        this.earthCircle
            .attr("cx", this.width / 2)
            .attr("cy", this.height / 2)

        //TODO this is needed for the scale reset to stick?
        const zoom = d3.zoom()
        this.svg.call(zoom);
        this.svg.call(zoom.transform, d3.zoomIdentity);
        geoZoom()
            .projection(this.proj)
            .onMove((f) => this.refresh())(this.svg.node());

        this.refresh();
    }

}

window.customElements.define('direction-map', GeofixDirectionMap);

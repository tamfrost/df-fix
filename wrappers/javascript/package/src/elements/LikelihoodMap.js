export class LikelihoodMap extends GeofixDirectionMap {

    constructor(props) {
        super(props);
        // this.updateLikelihood = false;
    }

    clear() {
        try {
            this.mainGroup.selectAll('.directionlocations').remove();
            this.mainGroup.selectAll('.directionlines').remove();
            this.mainGroup.selectAll('.directioncrosses').remove();
            this.mainGroup.selectAll('.likelihoodpolygon').remove();
            this.mainGroup.selectAll('.positionellipse').remove();
            this.mainGroup.selectAll('.lhimage').remove();
        }
        catch (e) {}
    }

    select(wasmModule, lhData) {
        // if(this.updateLikelihood) {
        //     this.infodiv.style.display = 'block';
        //     this.infodiv.innerHTML = 'CALCULATING LIKELIHOOD';
        //     this.updateLikelihood = false;
        //     this.svg.call(this.zoom.scaleTo,1)
        //     this.svg.call(this.zoom.translateTo, 500, 0)
        //     setTimeout(() => {
        //         wasmModule.getLikelihood().then(lhData => {
        //             this.infodiv.style.display = 'none';
        //             this.infodiv.innerHTML = '';
        //             this.drawLikelihood(lhData);
        //         });
        //     },100)
        // }
        // this.reset();
    }

    drawLikelihood(lhData, lhImageData) {
        const nLon = d3.max(lhData.lonIndex) - d3.min(lhData.lonIndex);
        const nLat = d3.max(lhData.latIndex) - d3.min(lhData.latIndex);
        const minLon = d3.min(lhData.lonData);
        const maxLon = d3.max(lhData.lonData);
        const minLat = d3.min(lhData.latData);
        const maxLat = d3.max(lhData.latData);

        const canvas = document.createElement("canvas");
        canvas.setAttribute("width", `${nLon}`);
        canvas.setAttribute("height", `${nLat}`);
        const ctx = canvas.getContext('2d');
        ctx.putImageData(new ImageData(lhImageData, nLon, nLat), 0, 0);

        let image = new Image();
        image.src = canvas.toDataURL("image/png");

        const lBounds = this.proj([minLon, maxLat]);
        const hBounds = this.proj([maxLon, minLat]);
        this.mainGroup.selectAll('.lhimage').remove();
        this.mainGroup
            .insert("image",":first-child")
            .attr("class", 'lhimage')
            // .style("transform", `scaleY(-1) translateY(${-(lBounds[1]+hBounds[1])}px)`)
            .attr("href", canvas.toDataURL("image/png"))
            .attr("x", lBounds[0])
            .attr("y", lBounds[1])
            .attr("width", hBounds[0]-lBounds[0])
            .attr("height", hBounds[1]-lBounds[1])
    }

    zoomed(event) {
        this.mainGroup.attr("transform", event.transform);
        this.mainGroup.attr("stroke-width", 1/event.transform.k);
        this.mainGroup.node().querySelectorAll('.positionellipse path').forEach(e => {
            e.style['stroke-width'] = `${2/event.transform.k}px`;
        })
    }

    zoomSetup() {
        this.zoom = d3.zoom()
            .scaleExtent([0.5, 10])
            .extent([[0,0],[this.width,this.height]])
            .translateExtent([[0,0],[2*this.width,this.height]])
            .on("zoom", this.zoomed.bind(this));
        this.svg.call(this.zoom);
    }

    projectionSetup() {
        this.proj = geoEquirectangular();
    }

    reset() {

        const svgWidth = this.mapsvg.clientWidth;
        const mapWidth = this.shadow.querySelector('.main-group').getBBox().width;
        const svgHeight = this.mapsvg.clientHeight;
        const mapHeight = this.shadow.querySelector('.main-group').getBBox().height;
        const currentScale = this.proj.scale();
        const newScale = currentScale * svgWidth / mapWidth;

        // this.proj
        //     .scale(newScale)
        //     .translate([this.width / 2, this.height / 2]);

        if (mapHeight > 0) this.zoom.scaleTo(this.svg, svgHeight / mapHeight)
        this.zoom.translateTo(this.svg, 300, 0)

        this.refresh();
    }
}

window.customElements.define('likelihood-map', LikelihoodMap);

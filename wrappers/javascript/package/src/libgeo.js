'use strict';

// var xy2latlong = function(xy) {
//     return ol.proj.transform(xy, "EPSG:3857", "EPSG:4326");
// };
//
// var latlong2xy = function(latlong) {
//     return ol.proj.transform(latlong, "EPSG:4326", "EPSG:3857");
//     //return ol.proj.transform(latlong, "EPSG:4326", "EPSG:900913");
// };

let geo2sphere = function (lonlat) {
    let sphericalUnitVector = [1, (90 - lonlat[1]) * Math.PI/180, lonlat[0] * Math.PI/180];
    if (lonlat[0] < 0) sphericalUnitVector[2] = sphericalUnitVector[2] + 2*Math.PI;
    return sphericalUnitVector
};

let sphere2geo = function (sphericalVector) {
    let lonlat = [sphericalVector[2] * 180/Math.PI, 90 - (sphericalVector[1] * 180/Math.PI)];
    if (lonlat[0] > 180) lonlat[0] = lonlat[0] - 360;
    return lonlat
};

let sphere2cart = function (sphericalVector) {
    return [
        sphericalVector[0] * Math.sin(sphericalVector[1]) * Math.cos(sphericalVector[2]),
        sphericalVector[0] * Math.sin(sphericalVector[1]) * Math.sin(sphericalVector[2]),
        sphericalVector[0] * Math.cos(sphericalVector[1])
        ];
};

let rotate3dMatrix = function( axes, angle ) {
    let R;
    if (axes === 'x') R = [
        [1, 0, 0],
        [0, Math.cos(angle), -Math.sin(angle)],
        [0, Math.sin(angle), Math.cos(angle)],
    ];
    if (axes === 'y') R = [
        [Math.cos(angle),  0, Math.sin(angle)],
        [0, 1, 0],
        [-Math.sin(angle), 0, Math.cos(angle)],
    ];
    if (axes === 'z') R = [
        [Math.cos(angle), -Math.sin(angle), 0],
        [Math.sin(angle), Math.cos(angle), 0],
        [0, 0, 1],
    ];
    // console.log(angle, R);
    return R;
};

let matrixVectorMultiplication = function(M, v) {
    let V = [];
    V[0] = M[0][0]*v[0] + M[0][1]*v[1] + M[0][2]*v[2];
    V[1] = M[1][0]*v[0] + M[1][1]*v[1] + M[1][2]*v[2];
    V[2] = M[2][0]*v[0] + M[2][1]*v[1] + M[2][2]*v[2];
    return V;
};


let rotate3d = function( axes, angle, v ) {
    return matrixVectorMultiplication(rotate3dMatrix( axes, angle), v);
};

let crossProduct = function (x, y) {
    return [
        x[1] * y[2] - x[2] * y[1],
        x[2] * y[0] - x[0] * y[2],
        x[0] * y[1] - x[1] * y[0]
    ];
};

let vectorProduct = function (x, y) {
    return x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
};

let vectorNorm = function (x) {
    return Math.sqrt(vectorProduct(x, x));
};

let vectorLinearCombination = function (cx, x, cy, y) {
    return [
        cx * x[0] + cy * y[0],
        cx * x[1] + cy * y[1],
        cx * x[2] + cy * y[2]
    ];
};

let geographic2spherical = function (latitude, longitude) {
    return [
        (90 - latitude) * Math.PI / 180,
        longitude * Math.PI / 180
    ];
};

let geographic2cartesian = function (latitude, longitude) {
    let theta = (90 - latitude) * Math.PI / 180;
    let phi = longitude * Math.PI / 180;
    // console.log(latitude, longitude, theta, phi);
    return [
        Math.sin(theta) * Math.cos(phi),
        Math.sin(theta) * Math.sin(phi),
        Math.cos(theta)
    ];
};

let cartesian2geographic = function (x) {
    x = normalizeVector(x);
    let phi = Math.atan(x[1] / x[0]);
    if (x[0] < 0) phi = Math.PI + phi;
    let theta = Math.acos(x[2]);
//		console.log(x);
//		console.log(theta + " " + phi);
    let latitude = 90 - (theta * 180 / Math.PI);
    let longitude = phi * 180 / Math.PI;
    return [longitude, latitude];
};

let angleBetweenVectors = function (x, y) {
    return Math.acos(vectorProduct(x, y) / (vectorNorm(x) * vectorNorm(y)));
};

let normalizeVector = function (x) {
    let xNorm = vectorNorm(x);
    return [
        x[0] / xNorm,
        x[1] / xNorm,
        x[2] / xNorm
    ];
};

//	deviationWeight = function (deviation, sigma) {
//	weight = 0.0;
//	// weight = exp(-deviation*deviation/(2*sigma*sigma));
//	if (abs(deviation) <= sigma)
//	weight = 1.0;
//	return weight;
//	}

//	a vector pointing from a point along a specific bearing (expressed in geo-fixed coordinates)
let bearingVector = function (pointLat, pointLon, bearing) {

    // first define a cartesian coordinate system local to the DF site (expressed in the geo-fixed coordinates)
    // axis pointing straight up (same as the vector pointing to the site longitude and latitude)
    let local_zenith_vector = geographic2cartesian(pointLat, pointLon);
    // axis pointing east
    let local_EW_vector = [-Math.sin(pointLon * Math.PI / 180), Math.cos(pointLon * Math.PI / 180), 0.0];
    // axis pointing north
    let local_NS_vector = crossProduct(local_zenith_vector, local_EW_vector);

    // calculate the vector pointing in the bearing direction (expressed in the geo-fixed coordinates)
    return vectorLinearCombination(Math.sin(bearing * Math.PI / 180), local_EW_vector, Math.cos(bearing * Math.PI / 180), local_NS_vector);
};


let bearingPoints = function (pointLat, pointLon, bearing, length) {
    let EARTH_RADIUS = 6371;
    let phiMax = length / EARTH_RADIUS;
    let zenithVector = geographic2cartesian(pointLat, pointLon); // pointing up
    let bearing_vector = bearingVector(pointLat, pointLon, bearing);
    let vector = vectorLinearCombination(Math.cos(phiMax), zenithVector, Math.sin(phiMax), bearing_vector);
    return cartesian2geographic(vector);

//		result = [];
//		let nPoints = (phiMax * 180 / Math.PI)/2;
//		for (iphi=0; iphi<=nPoints; iphi++) {
//		phi = phiMax * iphi / nPoints;
//		let vector = vectorLinearCombination(Math.cos(phi), zenithVector, Math.sin(phi), bearingVector);
//		result.push(cartesian2geographic(vector));
//		}
//		return result;
};

let bearingPlane = function (pointLat, pointLon, bearing) {
    let local_coordinates_up_vector = geographic2cartesian(pointLat, pointLon); // pointing up
    let local_coordinates_EW_vector = [
        -Math.sin(pointLon * Math.PI / 180),
        Math.cos(pointLon * Math.PI / 180),
        0.0
    ]; // pointing east
    let local_coordinates_NS_vector = crossProduct(local_coordinates_up_vector, local_coordinates_EW_vector); // pointing north

    let bearing_vector = vectorLinearCombination(Math.sin(bearing * Math.PI / 180), local_coordinates_EW_vector, Math.cos(bearing * Math.PI / 180), local_coordinates_NS_vector);

    let bearing_plane_normal_vector = crossProduct(local_coordinates_up_vector, bearing_vector);
    // normalize_vector(bearing_plane_normal_vector);
    return bearing_plane_normal_vector;
};

//	geographical distance between two locations
let geoDistance = function(site1_latitude, site1_longitude, site2_latitude, site2_longitude) {
    let EARTH_RADIUS = 6371;
    return EARTH_RADIUS * angleBetweenVectors(
        geographic2cartesian(site1_latitude, site1_longitude),
        geographic2cartesian(site2_latitude, site2_longitude)
    );
};

//	spherical angle between two locations on the sphere
let geoAngle = function(site1_latitude, site1_longitude, site2_latitude, site2_longitude) {
    return angleBetweenVectors(
        geographic2cartesian(site1_latitude, site1_longitude),
        geographic2cartesian(site2_latitude, site2_longitude)
    );
};

//	the geographical distance between a location and a plane passing through the earth center
let geoPointToPlaneDistance = function (longitude, latitude, planeNormal) {
    let EARTH_RADIUS = 6371;
    // straight distance (through the earth)
    let straight_distance = vectorProduct(geographic2cartesian(latitude, longitude), planeNormal);
    // geographical distance (on top of the earth surface)
    return EARTH_RADIUS * Math.asin(straight_distance);
};

//	...
let geoCircle = function(point1Lat, point1Lon, point2Lat, point2Lon) {
    let EARTH_RADIUS = 6371;
    let angle = angleBetweenVectors(
        geographic2cartesian(point1Lat, point1Lon),
        geographic2cartesian(point2Lat, point2Lon)
    );
    let nPoints = 100;
    let points = Array.from({length: nPoints}, (x,i) => i).map(i => {
        return cartesian2geographic(
            rotate3d('z', (point1Lon)*Math.PI/180,
                rotate3d('y', (90-point1Lat)*Math.PI/180,
                    sphere2cart([1, angle, i*2*Math.PI/nPoints]))
            )
        );
    });

    return {
        points: points,
        radius: angle * EARTH_RADIUS
    }
};

let bearingLimits = function(pointLat, pointLon, aoiLat, aoiLon, radius) {
    let EARTH_RADIUS = 6371;

    let phi = angleBetweenVectors(
        geographic2cartesian(pointLat, pointLon),
        geographic2cartesian(aoiLat, aoiLon)
    );
    let theta = radius / EARTH_RADIUS;
    // console.log(radius, phi, theta);

    return Math.asin(theta/phi) * 180/Math.PI;
};


//	the geographic distance between the bearing and the center of the AOI
let poiBearingDistance = function (pointLat, pointLon, bearing, poiLat, poiLon) {

    let bearing_vector = bearingVector(pointLat, pointLon, bearing);
    let point_zenith_vector = geographic2cartesian(pointLat, pointLon);
    let bearing_plane_normal = crossProduct(point_zenith_vector, bearing_vector);
    bearing_plane_normal = normalizeVector(bearing_plane_normal);
    let poi_vector = geographic2cartesian(poiLat, poiLon);

    //check that AOI is on the right side of the DF site
    let closest_distance = -1.0;
    if (vectorProduct(bearing_vector, poi_vector) > 0) {
        let closest_distance = Math.abs(geoPointToPlaneDistance(poiLon, poiLat, bearing_plane_normal));
    }

    return closest_distance;
};

//	bearing from one coordinate to an other
let bearing = function (pointLat, pointLon, poiLat, poiLon) {

    let pointVector = geographic2cartesian(pointLat, pointLon);
    let poiVector = geographic2cartesian(poiLat, poiLon);
    let zenithVector = [0, 0, 1];
    let c1 = crossProduct(pointVector, poiVector);
    let c2 = crossProduct(pointVector, zenithVector);
    let bearing = angleBetweenVectors(c1,c2) * 180/Math.PI;

    if (c1[2] < 0) return 360 - bearing;
    return bearing;
};

// cross calculation
// var planeIntersectVector = normalizeVector(crossProduct(dfshots[ishot].bearing_plane, dfshots[jshot].bearing_plane));
// planeIntersectVector[0] *= signum(planeIntersectVector[2]);
// planeIntersectVector[1] *= signum(planeIntersectVector[2]);
// planeIntersectVector[2] *= signum(planeIntersectVector[2]);
// var crossLatitude = 90 - (Math.acos(planeIntersectVector[2]) * 180/Math.PI);
// var crossLongitude = Math.atan(planeIntersectVector[1]/planeIntersectVector[0]) * 180/Math.PI;
// distanceFromAoiCenter = distanceBetweenGeoPoints(crossLatitude, crossLongitude, aoiLatitude, aoiLongitude);


let vincenty = function () {
    var a = 6378137;
    var f = 1 / 298.257223563;
    var b = (1 - f) * a;
    var fi1 = 59;
    var fi2 = 60;
    var lambda1 = 17;
    var lambda2 = 18;

    var L = (lambda2 - lambda1) * Math.PI / 180;
    var tanU1 = (1 - f) * Math.tan(fi1 * Math.PI / 180), cosU1 = 1 / Math.sqrt((1 + tanU1 * tanU1)), sinU1 = tanU1 * cosU1;
    var tanU2 = (1 - f) * Math.tan(fi2 * Math.PI / 180), cosU2 = 1 / Math.sqrt((1 + tanU2 * tanU2)), sinU2 = tanU2 * cosU2;


    var lambda = L;
    var lambdap = L + 1;

//console.log(tanU2);
    var nit = 0;
    var lambda = L, lambdap, iterationLimit = 100;
    while (Math.abs(lambda - lambdap) > 1e-12) {
        var sinlambda = Math.sin(lambda), coslambda = Math.cos(lambda);
        var sinSqs = (cosU2 * sinlambda) * (cosU2 * sinlambda) + (cosU1 * sinU2 - sinU1 * cosU2 * coslambda) * (cosU1 * sinU2 - sinU1 * cosU2 * coslambda);
        var sins = Math.sqrt(sinSqs);
        if (sins === 0) (console.log("co-incident points"));  // co-incident points
        var coss = sinU1 * sinU2 + cosU1 * cosU2 * coslambda;
        var s = Math.atan2(sins, coss);
        var sina = cosU1 * cosU2 * sinlambda / sins;
        var cosSqa = 1 - sina * sina;
        var cos2sM = coss - 2 * sinU1 * sinU2 / cosSqa;
        var C = f / 16 * cosSqa * (4 + f * (4 - 3 * cosSqa));
        lambdap = lambda;
        lambda = L + (1 - C) * f * sina * (s + C * sins * (cos2sM + C * coss * (-1 + 2 * cos2sM * cos2sM)));
        nit++;
        if (nit === 1) break;
    }

    console.log(sinlambda);
    console.log(coslambda);
    console.log(sinSqs);
    console.log(sins);
    console.log(coss);
    console.log(sina);
    console.log(cosSqa);
    console.log(cos2sM);
    console.log(C);
    console.log(lambda);
    console.log(Math.sin(lambdap));

    var uSq = cosSqa * (a * a - b * b) / (b * b);
    var A = 1 + uSq / 16384 * (4096 + uSq * (-768 + uSq * (320 - 175 * uSq)));
    var B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74 - 47 * uSq)));
    var Ds = B * sins * (cos2sM + B / 4 * (coss * (-1 + 2 * cos2sM * cos2sM) - B / 6 * cos2sM * (-3 + 4 * sins * sins) * (-3 + 4 * cos2sM * cos2sM)));

    var s = b * A * (s - Ds);

    var fwdAz = Math.atan2(cosU2 * sinlambda, cosU1 * sinU2 - sinU1 * cosU2 * coslambda);
    var revAz = Math.atan2(cosU1 * sinlambda, -sinU1 * cosU2 + cosU1 * sinU2 * coslambda);

}

export {bearingPoints, geoCircle, bearingLimits, bearing, geo2sphere, sphere2geo, sphere2cart};
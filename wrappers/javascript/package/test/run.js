#!/usr/bin/env node

const loadGeoFix = require('../src/geofix.js').loadGeoFix;

loadGeoFix({silent: false}).then(async geoFix => {
    const d = await geoFix.loadDirections(0, [2.350128676656633, 48.86129501083692, 13.409665746048544, 52.521323096219966, 13.779348093551762, 45.64638482429674], [90, 225, 315, 2, 2, 2]);
    const p = await geoFix.getPosition('mean', 0, 0);
    geoFix.logCompilationTime();
    geoFix.logVersion();
    geoFix.logCommitHash();
    console.log(geoFix.greatCircle(20, 60, 20, 45, 10000));
})



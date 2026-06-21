#!/usr/bin/env node

const loadGeoFix = require('../src/geofix.js').loadGeoFix;

test('load directions', async () => {
    hash = process.env.COMMIT_HASH;

    geoFix = await loadGeoFix({silent: true});
    const d = await geoFix.loadDirections(0, [2.350128676656633, 48.86129501083692, 13.409665746048544, 52.521323096219966, 13.779348093551762, 45.64638482429674], [90, 225, 315, 2, 2, 2]);
    const p = await geoFix.getPosition('mean', 0, 0);
    const commitHash = await geoFix.getCommitHash();

    console.log("COMMIT HASH:", commitHash);
    
    expect(Math.floor(p.center[0])).toBe(48);
    expect(hash === '--hash--' || hash === commitHash).toBeTruthy();
})


import { loadGeoFix } from '@<namespace>/geofix';

loadGeoFix({silent: false}).then(async geoFixInstance => {
    console.log('GeoFix loaded', geoFixInstance);
    
    const d = await geoFixInstance.loadDirections(0, [2.350128676656633, 48.86129501083692, 13.409665746048544, 52.521323096219966, 13.779348093551762, 45.64638482429674], [90, 225, 315, 2, 2, 2]);
    const p = await geoFixInstance.getPosition('mean', 0, 0);
    geoFixInstance.logCompilationTime();
    geoFixInstance.logVersion();
    geoFixInstance.logCommitHash();
    console.log(geoFixInstance.greatCircle(20, 60, 20, 45, 10000));
})
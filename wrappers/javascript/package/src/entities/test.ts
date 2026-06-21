import { Bearing } from "./Bearing";
import { Site } from "./Site";

function main() {
    try {
        const site = new Site('site1', 78.910, 123.456);
        const bearing = new Bearing(85, 0.5, site);

        console.log(bearing);

        // Example of using another class if available, e.g., GeoLocation
        // Assuming GeoLocation takes two Coordinates (lat, lon) and a timestamp
        // import { GeoLocation } from "./GeoLocation";
        // import { Latitude } from "./Latitude";
        // import { Longitude } from "./Longitude";
        // const lat = new Latitude(123.456, 0.1);
        // const lon = new Longitude(78.910, 0.1);
        // const location = new GeoLocation(lat, lon, Date.now());
        // console.log(`GeoLocation created: Lat=${location.latitude.value}, Lon=${location.longitude.value}, Timestamp=${location.timestamp}`);

    } catch (e: any) {
        console.error(e.message);
    }
}

main();

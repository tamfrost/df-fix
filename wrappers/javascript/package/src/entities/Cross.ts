import { GeoLocation } from './GeoLocation';
import { Bearing } from './Bearing';

/**
 * 
 */
export class Cross extends GeoLocation {

    /**
     * 
     */
    public constructor(bearing1: Bearing, bearing2: Bearing)  {
        super(...Cross.calculateLocation(bearing1, bearing2));
    }

    static calculateLocation(bearing1: Bearing, bearing2: Bearing): [number, number] {
        // // Implement the logic to calculate the intersection point of two bearings
        // // This is a placeholder implementation and should be replaced with actual logic
        // const longitude = (bearing1.location.longitude.value + bearing2.location.longitude.value) / 2;
        // const latitude = (bearing1.location.latitude.value + bearing2.location.latitude.value) / 2;
        return [1, 2];
    }


}
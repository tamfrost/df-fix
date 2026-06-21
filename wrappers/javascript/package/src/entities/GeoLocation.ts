import { Longitude } from './Longitude';
import { Latitude } from './Latitude';
import { geo2sphere, sphere2cart, sphere2geo } from '../libgeo';
/**
 * 
 */
export class GeoLocation {

    /**
     * 
     */
    longitude: Longitude;
    /**
     * 
     */
    latitude: Latitude;

    /**
     * 
     */
    public constructor(longitude: number|Longitude, latitude: number|Latitude) {
        try {
            this.longitude = longitude instanceof Longitude ? longitude : new Longitude(longitude, 0);
            this.latitude = latitude instanceof Latitude ? latitude : new Latitude(latitude, 0);
        }
        catch (e) {
            throw new Error(`Invalid GeoLocation: ${e}`);
        }
    }

    public getGeographic(): [number, number] {
        return [this.longitude.value, this.latitude.value];
    }

    public getSpherical(): [number, number] {
        return [this.longitude.value * Math.PI / 180, this.latitude.value * Math.PI / 180];
    }

    public getCartesian(): [number, number, number] {
        return [
            Math.cos(this.latitude.value * Math.PI / 180), Math.cos(this.longitude.value * Math.PI / 180), 1
        ]
    }

}


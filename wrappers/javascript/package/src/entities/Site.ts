import { GeoLocation } from './GeoLocation';
import { Longitude } from './Longitude';
import { Latitude } from './Latitude';

/**
 * 
 */
export class Site extends GeoLocation {

    /**
     * 
     */
    public constructor(name: string, longitude: number|Longitude, latitude: number|Latitude) {
        super(
            longitude instanceof Longitude ? longitude : new Longitude(longitude, 0),
            latitude instanceof Latitude ? latitude : new Latitude(latitude, 0)
            );
    }


}
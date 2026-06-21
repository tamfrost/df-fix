import { Direction } from './Direction';
import { GeoLocation } from './GeoLocation';
import { Cross } from './Cross';

/**
 * A bearing is ...
 */
export class Bearing extends Direction {

    /**
     * 
     */
    location: GeoLocation;

    public constructor(value: number, error: number, location: GeoLocation) {
        super(value, error);
        try {
            this.location = location;
        }
        catch (e) {
            throw new Error(`Invalid Bearing: ${e}`);
        }
    }

    public getCross(bearing: Bearing) : Cross {
        return new Cross(this, bearing);
    }

}
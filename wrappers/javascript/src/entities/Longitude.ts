/**
 * 
 */
export class Longitude {

    /**
    * 
    */
    value: number;
    /**
     * 
     */
    error: number;

    /**
     * 
     */
    public constructor(value: number, error: number) {
        try {
            this.verify(value, error);
            this.value = value;
            this.error = error;
        }
        catch (e) {
            throw new Error(`Invalid coordinate: ${e}`);
        }
    }

    private verify(value: number, error: number): void {
        if (typeof value !== 'number' || typeof error !== 'number') {
            throw new Error('Value and error must be numbers');
        }
        if (error < 0) {
            throw new Error('Error must be a non-negative number');
        }
    }

}
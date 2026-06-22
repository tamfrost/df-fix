export declare enum ErrorModel {
    test = 0, test2 = 1
}

export declare interface DirectionApiObj {
    included: boolean,
    lat: number, 
    lon: number,
    name: string,
    sigma: number,
    value: number
 }

export declare interface DFLocationApiObj {
    density: number[],
    directions: DirectionApiObj[],
    lat: number,
    lon: number,
    maxDensityDirection: number,
    name: string,
}

export declare class GeoFix {
    loadDirections(errorModel : ErrorModel, locations : number[], directions: number[]) : Promise<DirectionApiObj[]>
    getFixpoint(errorModel : ErrorModel, locations : number[], directions: number[]) : Promise<DirectionApiObj[]>
    getPosition(type: string, errorModel: ErrorModel) : Promise<DFLocationApiObj[]>
    logCompilationTime(): void;
    logVersion(): void;
}

export declare function loadGeoFix(): Promise<GeoFix>
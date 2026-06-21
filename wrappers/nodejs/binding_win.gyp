{
    "targets": [{
        "target_name": "geofixaddon",
        "cflags!": [ "-fno-exceptions" ],
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "sources": [
            "./addon.cpp",
            "../../fixlib.cpp",
            "../../src/geolib.cpp",
            "../../src/DfLocation.cpp",
            "../../src/Direction.cpp",
            "../../src/DirectionCollection.cpp",
            "../../src/DirectionCross.cpp",
            "../../src/DirectionCrossCollection.cpp",
            "../../src/GeoLocation.cpp",
            "../../src/LikelihoodTool.cpp",
            "../../src/Logger.cpp",
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "../../include",
            "../../../include",
            "../../../include/fftw-3.3.10/api"
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS', 'BUILD_HASH=\"$(BUILD_HASH)\"' ],
        'conditions': [
            ['OS=="win"', {
                'libraries': [
                    "<(module_root_dir)/build/Release/fftw3.lib"
                ]
            }],
        ],
        "copies": [
            {
              "destination": "<(module_root_dir)/build/Release",
              "files": [
                "../../lib/fftw3.dll",
                "../../lib/fftw3.lib",
                ]
            }
        ]
    }]
}

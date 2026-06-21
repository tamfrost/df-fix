{
    "targets": [{
        "target_name": "geofixaddon",
        "cflags": [ "-fexceptions", "-std=c++11" ],
        "cflags_cc": [ "-fexceptions", "-std=c++11" ],
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
            "../..",
            "../../include",
            "/usr/local/include/eigen3",
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS', 'BUILD_HASH="<!(echo $BUILD_HASH)"', 'GEOFIX_VERSION="0.0.0"', 'COMMIT_HASH="<!(echo $BUILD_HASH)"'],
        'conditions': [
            ['OS=="win"', {
                'libraries': [
                    "<(module_root_dir)/build/Release/libfftw3-3.lib"
                ]
            }],
            ['OS=="linux"', {
                'libraries': [
                    "<(module_root_dir)/build/Release/libGeographic.so.19.0.1",
                    "<(module_root_dir)/build/Release/libfftw3.so.3.5.5",
                    "<(module_root_dir)/build/Release/liblog4cxx.so.10.0.0"
                ]
            }],
        ],
        "copies": [
            {
              "destination": "<(module_root_dir)/build/Release",
              "files": [
                "/usr/lib64/libGeographic.so.19.0.1",
                "/usr/lib64/libfftw3.so.3.5.5",
                "/usr/lib64/liblog4cxx.so.10.0.0"
                ]
            }
        ]
    }]
}

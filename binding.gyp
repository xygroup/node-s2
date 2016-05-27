{
    "targets": [
        {
            "target_name": "NativeExtension",
            "sources": [
                "src/NativeExtension.cc",
                "src/functions.cc",
                "./geometry/s2.cc",
                "./geometry/s1interval.cc",
                "./geometry/util/math/exactfloat/exactfloat.cc",
                "./geometry/strings/strutil.cc",
                "./geometry/strings/stringprintf.cc",
          	    "./geometry/s2cap.cc",
          	    "./geometry/s1angle.cc",
          	    "./geometry/s2cell.cc",
          	    "./geometry/s2cellunion.cc",
          	    "./geometry/s2cellid.cc",
          	    "./geometry/s2edgeindex.cc",
          	    "./geometry/s2edgeutil.cc",
          	    "./geometry/s2latlngrect.cc",
          	    "./geometry/s2loop.cc",
          	    "./geometry/s2pointregion.cc",
          	    "./geometry/s2latlng.cc",
          	    "./geometry/s2polygon.cc",
          	    "./geometry/s2polygonbuilder.cc",
          	    "./geometry/s2polyline.cc",
          	    "./geometry/s2r2rect.cc",
          	    "./geometry/s2region.cc",
          	    "./geometry/s2regioncoverer.cc",
          	    "./geometry/s2regionintersection.cc",
          	    "./viewfinder/viewfinder.cc"
            ],
            "defines": [
                'NDEBUG'
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "./viewfinder/",
                "./geometry/",
                "./geometry/base/",
                "./geometry/util/",
                "./geometry/util/math/",
                "./geometry/strings/"
            ],
            'conditions': [
                ['OS=="mac"', {
                    'xcode_settings': {
                        'OTHER_CPLUSPLUSFLAGS':[
                            '-Wno-deprecated',
                            '-Wno-ignored-qualifiers',
                            '-Wno-absolute-value',
                            '-Wno-dynamic-class-memaccess',
                            '-DARCH_K8 -DS2_USE_EXACTFLOAT',
                            '-stdlib=libc++',
                            '-std=c++11'
                        ],
                        'OTHER_LDFLAGS':[
                            '-flat_namespace -undefined suppress',
                            '-stdlib=libc++',
                            '-std=c++11'
                        ],
                        'GCC_ENABLE_CPP_RTTI': 'YES',
                        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                        'MACOSX_DEPLOYMENT_TARGET': '10.7',
                    }
                }],
                ['OS=="linux"', {
                    'cflags_cc' : [
                    '-Wno-deprecated',
                    '-Wno-ignored-qualifiers',
                    '-DARCH_K8 -DS2_USE_EXACTFLOAT',
                    '-std=c++11'
                    ],
                }]
            ]
        }
    ]
}
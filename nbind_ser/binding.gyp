{
    "targets": [
        {
            "includes": [
                "auto.gypi"
            ],
            "sources": [
                "libteonet-js.cpp"
            ],
            "include_dirs": [
                "/usr/include/teonet/", 
                "/home/kirill/Projects/teonet/src/",
                "/home/kirill/Projects/teonet/embedded/libpbl/src/",
                "/home/kirill/Projects/teonet/embedded/teocli/libteol0",
                "/home/kirill/Projects/teonet/embedded/libtrudp/src"
            ],
            "libraries": [ "-lteonet" ]
        }
    ],
        "link_settings": {
    	    "libraries": []
    },
    "includes": [
        "auto-top.gypi"
    ]}

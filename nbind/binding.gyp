{
    "targets": [
        {
            "includes": [
                "auto.gypi"
            ],
            "sources": [
                "node_modules/teocli-l0/teonet_l0_client.c",
                "main_select_cpp.cpp"
            ],
            "cflags_cc": ["-std=c++14"],
            "cflags_cc!": ["-std=c++11","-std=gnu++0x"]
        }
    ],
    "includes": [
        "auto-top.gypi"
    ]
}

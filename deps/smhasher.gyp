{
    "targets": [
        {
            "target_name": "smhasher",
            "type": "static_library",
            "sources": [
                '<!@(ls -1 smhasher/*.cpp)'
            ],
            "conditions": [
                [
                    "OS != 'win'", {
                        "cflags_cc": [
                            "-Wno-unused-but-set-variable",
                            "-Wno-strict-aliasing"
                        ]
                    }
                ]
            ]
        }
    ]
}
{
    "targets": [
        {
            "target_name": "node-bloomfilter",
            "dependencies": [
                "deps/smhasher.gyp:smhasher"
            ],
            "sources": [
                "src/module.cc"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")",
                "<!(node -e \"require('nnu')\")",
                "deps"
            ]
        }
    ]
}
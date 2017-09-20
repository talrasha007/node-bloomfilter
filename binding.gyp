{
    "targets": [
        {
            "target_name": "node-bloomfilter",
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
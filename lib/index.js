const binding = require('../build/Release/node-bloomfilter.node');

class Bloomfilter {
    constructor(numBits, numHash) {
        this._bloom = new binding.Bloomfilter(numBits, numHash);
    }

    put(val) {
        this._bloom.put(val);
    }

    mightContain(val) {
        return this._bloom.mightContain(val);
    }
}

module.exports = Bloomfilter;
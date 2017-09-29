const binding = require('../build/Release/node-bloomfilter.node');

const MAX_BLOOM_FILTER_SIZE = 32 * 1024 * 1024 * 1024;
const MAX_HASH_FUNCS = 255;
const MIN_HASH_FUNCS = 1;
const LN2SQUARED = Math.pow(Math.log(2), 2); // 0.4804530139182014246671025263266649717305529515945455
const LN2 = Math.log(2); // 0.6931471805599453094172321214581765680755001343602552

class BloomStream {
    
}

class Bloomfilter {
    static bestFor(elementNum, falsePositiveRate) {
        let bitSize = -1.0 / LN2SQUARED * elementNum * Math.log(falsePositiveRate);
        bitSize = Math.min(MAX_BLOOM_FILTER_SIZE * 8, bitSize);
    
        // The ideal number of hash functions is:
        // filter size * ln(2) / number of elements
        // See: https://github.com/bitcoin/bitcoin/blob/master/src/bloom.cpp
        let nHashFuncs = Math.floor(bitSize / elementNum * LN2);
        if (nHashFuncs > MAX_HASH_FUNCS) {
          nHashFuncs = MAX_HASH_FUNCS;
        }
        if (nHashFuncs < MIN_HASH_FUNCS) {
          nHashFuncs = MIN_HASH_FUNCS;
        }
    
        return new Bloomfilter(bitSize, nHashFuncs)        
    }

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
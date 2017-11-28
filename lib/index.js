const Readable = require('stream').Readable;
const binding = require('../build/Release/node-bloomfilter.node');

const MAX_BLOOM_FILTER_SIZE = 32 * 1024 * 1024 * 1024;
const MAX_HASH_FUNCS = 255;
const MIN_HASH_FUNCS = 1;
const LN2SQUARED = Math.pow(Math.log(2), 2); // 0.4804530139182014246671025263266649717305529515945455
const LN2 = Math.log(2); // 0.6931471805599453094172321214581765680755001343602552

class BloomStream extends Readable {
    constructor(bloom) {
        super();

        this._bloom = bloom;
        this._offset = 0;
    }

    _read() {
        if (this._offset < this._bloom.memSize) {
            const toRead = Math.min(32 * 1024, this._bloom.memSize - this._offset);
            this.push(this._bloom._bloom.getBuffer(this._offset, toRead));
            this._offset += toRead;
        } else {
            this.push(null);
        }
    }
}

class Bloomfilter {
    static bestFor(elementNum, falsePositiveRate) {
        let bitSize = -1.0 / LN2SQUARED * elementNum * Math.log(falsePositiveRate);
        bitSize = Math.min(MAX_BLOOM_FILTER_SIZE * 8, Math.ceil(bitSize));
    
        // The ideal number of hash functions is:
        // filter size * ln(2) / number of elements
        // See: https://github.com/bitcoin/bitcoin/blob/master/src/bloom.cpp
        let nHashFuncs = Math.ceil(bitSize / elementNum * LN2);
        if (nHashFuncs > MAX_HASH_FUNCS) {
            nHashFuncs = MAX_HASH_FUNCS;
        }
        if (nHashFuncs < MIN_HASH_FUNCS) {
            nHashFuncs = MIN_HASH_FUNCS;
        }
    
        return new Bloomfilter(bitSize, nHashFuncs)        
    }

    constructor(numBits, numHash) {
        if (numBits instanceof binding.Bloomfilter) {
            this._bloom = numBits;
            numBits = this._bloom.getNumBits();
            numHash = this._bloom.getNumHash();
        } else {
            this._bloom = new binding.Bloomfilter(numBits, numHash);
        }

        Object.defineProperties(this, {
            memSize: {
                get: () => Math.floor(8 + numBits / 8 + 1)
            },

            bitvecSize: {
                get: () => Math.floor(numBits / 8 + 1)
            },

            numBits: {
                get: () => numBits
            },

            numHash: {
                get: () => numHash
            }
        });
    }

    put(val) {
        this._bloom.put(val);
    }

    mightContain(val) {
        return this._bloom.mightContain(val);
    }

    toStream() {
        return new BloomStream(this);
    }

    static fromStream(stream) {
        return new Promise((resolve, reject) => {
            let bloom,
                firstPass = true,
                offset = 0;

            stream.on('error', reject);
            stream.on('end', () => {
                if (bloom && offset === bloom.bitvecSize) {
                    resolve(bloom);                    
                } else {
                    reject(new Error('Input stream is smaller than expected.'));
                }
            });
            stream.on('data', buf => {
                if (!bloom) {
                    if (buf.length < 8 || !firstPass) {
                        return reject(new Error('Input stream is invalid.'));
                    }

                    firstPass = false;
                    const bloomCpp = new binding.Bloomfilter(buf);
                    bloom = new Bloomfilter(bloomCpp);
                    offset = buf.length - 8;
                    return ;
                }

                if (offset + buf.length <= bloom.bitvecSize) {
                    bloom._bloom.setBuffer(buf, offset);
                    offset += buf.length;
                } else {
                    reject(new Error('Input stream is larger than expected.'));
                }
            });            
        });
    }
}

module.exports = Bloomfilter;
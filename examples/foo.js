const fs = require('fs');
const Bloomfilter = require('../');

// const bloom = Bloomfilter.bestFor(128, 0.005);
const bloom = new Bloomfilter(512 * 1024 * 1024, 16);
bloom.put('1');
bloom.put('2');

console.log(bloom.mightContain('1'));
console.log(bloom.mightContain('2'));
console.log(bloom.mightContain('3'));

function inspect(bl) {
    const { memSize, bitvecSize, numBits, numHash } = bl;
    console.log({ memSize, bitvecSize, numBits, numHash });
}

bloom.toStream().pipe(fs.createWriteStream('/tmp/test.bloom')).on('finish', () => {
    Bloomfilter.fromStream(fs.createReadStream('/tmp/test.bloom')).then(deser => {
        console.log('Origin:');
        inspect(bloom);
        console.log('Deserialize:');
        inspect(deser);

        deser.toStream().pipe(fs.createWriteStream('/tmp/test_dser.bloom'));
    });
});
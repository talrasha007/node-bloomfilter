# node-bloomfilter
A native node bloom filter implementation, using 64bit murmur hash, suport more than 4G bits (It's the reason why i write this lib, most implementation use 32-bit hash or 32-bit array index, so bit vector cannot be larger than 4G).

```js
const Bloomfilter = require('node-bloomfilter');

const bloom = Bloomfilter.bestFor(128 /* elemNum */, 0.005 /* flase positive rate */);

const bloomXXX = new Bloomfilter(888 /* num bits */, 8 /* num hash func */);

bloom.put('1');
bloom.put('2');

console.log(bloom.mightContain('1'));
console.log(bloom.mightContain('2'));
console.log(bloom.mightContain('3'));

bloom.toStream().pipe(fs.createWriteStream('/tmp/test.bloom')).on('finish', () => {
    Bloomfilter.fromStream(fs.createReadStream('/tmp/test.bloom')).then(deser => {
        console.log(bloom, deser);
        deser.toStream().pipe(fs.createWriteStream('/tmp/test_dser.bloom'));
    });
});
```

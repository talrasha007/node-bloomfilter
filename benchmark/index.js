const Bloomfilter = require('../');

suite('Bloomfilter', () => {
    const bloom = new Bloomfilter(1024 * 1024, 16);
    const buf = new Buffer(32);

    bench('put const string', () => bloom.put('1'));
    bench('put const buffer', () => bloom.put(buf));
    bench('test const string', () => bloom.mightContain('1'));
    bench('test const buffer', () => bloom.mightContain(buf));

    bench('put rand', () => bloom.put(Math.random()));
    bench('test rand', () => bloom.mightContain(Math.random()));
});

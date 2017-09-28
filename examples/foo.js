const Bloomfilter = require('../');

const bloom = Bloomfilter.bestFor(128, 0.005);
bloom.put('1');
bloom.put('2');

console.log(bloom.mightContain('1'));
console.log(bloom.mightContain('2'));
console.log(bloom.mightContain('3'));
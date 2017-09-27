const Bloomfilter = require('../');

const bloom = new Bloomfilter(128, 5);
bloom.put('1');
bloom.put('2');

console.log(bloom.mightContain('1'));
console.log(bloom.mightContain('2'));
console.log(bloom.mightContain('3'));
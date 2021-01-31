/*
 * Tests elasticlunr's index size generation.
 * Run: `npm install elasticlunr` first.
 */
var fs = require('fs');
var elasticlunr = require('elasticlunr');

var idx = elasticlunr(function () {
  this.addField('title');
  this.addField('body');
  this.setRef('id');
});

var doc = JSON
  .parse(fs.readFileSync('./index.json', 'utf8'))
  .forEach((obj) => {
    idx.addDoc(obj);
  });

fs.writeFile('./output.json', JSON.stringify(idx), function (err) {
  if (err) throw err;
  console.log('done');
});


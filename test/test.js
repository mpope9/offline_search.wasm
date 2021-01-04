var assert = require('assert');
var fs = require('fs');
var FilterBuilder = require('../lib/filter_builder.js');

describe('FilterBuilder', function() {
  describe('serialize / deserialize test', function() {
  
    it('should serialize the on small JSON index.', function() {

      var builder = new FilterBuilder();
      return builder.build('test/index.json').then(function() {
        assert.equal(true, fs.existsSync('build/filters'));
      });

    });
  });
});

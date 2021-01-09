/*
 * Test that buils a serialized index. To be used in `test/test_search.js`.
 */
var assert = require('assert');
var fs = require('fs');
var FilterBuilder = require('../lib/filter_builder.js');

describe('FilterBuilder', () => {
  describe('generation / serialize test', () => {
  
    it('should serialize the on small JSON index.', () => {

      var builder = new FilterBuilder();
      return builder.build('test/index.json').then(() => {
        assert.equal(true, fs.existsSync('build/filters'));
      });

    });
  });
});


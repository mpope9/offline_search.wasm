var assert = require('assert');
var fs = require('fs');
var FilterBuilder = require('../lib/filter_builder.js');
//var OfflineFilter = require('../lib/offline_filter.js');
//var WASM = require('../build/offilne_search_wasm.js');

describe('FilterBuilder', function() {
  describe('serialize / deserialize test', function() {
  
    it('should serialize the on small JSON index.', function() {

      var builder = new FilterBuilder();
      return builder.build('test/index.json').then(function() {
        assert.equal(true, fs.existsSync('build/filters'));
      });

    });

    //it('should deserialize and query the small JSON index', function() {
    //  
    //  var filter = new OfflineFilter(WASM);
    //  var result = filter.search('Kubernetes');
    //  assert.equal(result, []);
    //});

    //it('an uninitialized filter should not be queryable', function() {
    //});
  
  });
});

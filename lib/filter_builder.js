/* !
 * filter_builder.js
 * Copyright (C) @YEAR Matthew Pope
 */

/** 
 * filter_builder is used to parse the index.json file or HTML and construct serialized xor_filter
 * binaries through the WASM interface.
 *
 * The is the Node.js index builder script.
 */

const Module = require('../build/xor_builder.js');
var fs = require('fs');

module.exports = class FilterBuilder {

  index_file;

  constructor() { }

  /**
   * Callback to read the index.json on WASM module initialization.
   */
  build(indexFile) {

    return Module().then(function(mod) {
    
      JSON.parse(fs.readFileSync(indexFile, 'utf8')).forEach((obj) => {
    
        var inputString = '';
        for(const property in obj) {
          if(property != 'url') {
            inputString = inputString + ' ' + obj[property];
          }
        }

        var url = obj.url;

        var urlInts = mod.intArrayFromString(url, false);
        var urlPtr = mod.allocate(urlInts, mod.ALLOC_NORMAL);
  
        var inputStringInts = mod.intArrayFromString(inputString);
        var inputStringPtr = mod.allocate(inputStringInts, mod.ALLOC_NORMAL);
  
        var result = mod._build_xor(inputStringPtr, urlPtr);
  
        mod._free(urlPtr);
        mod._free(inputStringPtr);
      });
    
    });
  }
}

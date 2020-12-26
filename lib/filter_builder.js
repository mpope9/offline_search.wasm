/* !
 * filter_builder.js
 * Copyright (C) @YEAR Matthew Pope
 */

/** 
 * filter_builder is used to parse the index.json file or HTML and construct serialized xor_filter
 * binaries through the WASM interface.
 */

const Module = require('../_build/xor_builder.js');
const fs = require('fs');

/**
 * Callback to read the index.json on WASM module initialization.
 */
Module().then(function(mod) {

  // TODO: iterate over these objects.
  var [obj] = JSON.parse(fs.readFileSync('index.json', 'utf8'));

  inputString = '';
  for(const property in obj) {
    if(property != 'url') {
      inputString + ' ' + obj[property];
    }
  }

  index_content(mod, inputString, obj.url);
});

/**
 * Build and serialize xor_filter to a file from the input string.
 *
 * @param {object} mod WASM module.
 * @param {string} post The item to index.
 * @param {string} name The index mapping.
 */
function index_content(mod, post, url) {

  var urlInts = mod.intArrayFromString(url, false);
  var urlPtr = mod.allocate(urlInts, mod.ALLOC_NORMAL);

  var postInts = mod.intArrayFromString(post);
  var postPtr = mod.allocate(postInts, mod.ALLOC_NORMAL);

  var result = mod._build_xor(postPtr, urlPtr);

  mod._free(urlPtr);
  mod._free(postPtr);
}

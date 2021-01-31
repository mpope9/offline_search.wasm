# offline_search.wasm

This is a library to generate Webassembly bindings for a full-text offline search index backed by [xor filters](https://github.com/FastFilter/xor_singleheader). It is heavily inspired by `tinysearch` & `elasticlunr.js`. This library strives to strike a balance between the highest level of size efficiency, performance, and features.

## :muscle: Index Size Comparisons From [tinysearch Example File](https://github.com/tinysearch/tinysearch/blob/master/fixtures/index.json)

TL;DR: `offline_search.wasm`'s index is **~30%** smaller than `tinysearch` and **~190%** smaller than `elasticlunr.js`.

| Source | Size |
| --- | --- |
| offline_search.wasm | 11k |
| tinysearch | 15k |
| raw | 192k |
| elasticlunr.js | 437K |

We're cheating just a little bit, because `offline_search.wasm` only returns URLs right now, and not article descriptions ;). This will be updated in future versions.

`elasticlunr.js` test code can be found in `test/elasticlunr/`

## :mag: :ant: Features
* Slim size through [Webassembly](https://webassembly.org/) using [Emscripten](https://emscripten.org/index.html).
* Extremely small index size.
* [Stemming](https://en.wikipedia.org/wiki/Stemming).
* [Stop word filtering](https://www.elastic.co/guide/en/elasticsearch/reference/current/analysis-stop-tokenfilter.html).
* [Fuzzy search](http://aspell.net/metaphone/) (matching on misspelled words).
* Full text search.

## :zap: Usage

The recommended installation uses Docker or [Podman](https://developers.redhat.com/blog/2019/02/21/podman-and-buildah-for-docker-users/). This ensures the highest level of compatibility for building the Webassembly code, in the most contained way. However, if this is a limiting factor, the build scripts are located in `scripts/` and can be used to directly run against Emscripten's `emcc`. [Instructions on how to install it can be found here](https://emscripten.org/docs/getting_started/downloads.html#sdk-download-and-install).

To build a filter, the following `npm` command can be used:
```
npm run build
```
This will run the provided 0 dependency Node.js script in `lib/filter_builder.js`.

The WASM, example Javascript code, and example HTML script will be generated to the `build/` directory. You can copy that into your site for usage. It includes minified versions of the Javascript code, which should be suited for production. Be sure to copy the `offline_search_wasm.data` file into the root directory of your webserver. This contains the search index.

The example Javascript code provided in `offline_search.js` should be enough for production. However, it is also a good starting point if a more advanced implementation is needed.

To build without Docker use the `scripts/build_without_docker` script. This will not call [`terser`](https://github.com/terser/terser) to minify the Javascript files.

Here is how to use it from HTML:
```html
<html>

<head>
  <meta content="text/html;charset=utf-8" http-equiv="Content-Type" />
</head>

<body>

  <!-- Import JS glue code + WASM. -->
  <script type="text/javascript" src="offline_search_wasm.js"></script>

  <!-- Import offline_search lib. -->
  <script type="module">
    import { OfflineSearch } from './offline_search.js';

    // Create new search object, 'Module' is defined in 'offline_search_wasm.js'
    var offlineSearch = new OfflineSearch(Module());

    // Initialize index from 'offline_search_wasm.data' file.
    offlineSearch.init().then(() => {

      // Keyword search.
      offlineSearch
        .search('Cal Naughton Jr.')
        .then((value) => {
          console.log(value); // https://www.imdb.com/title/tt0415306/
        });

      // Free memory.
      offlineSearch.free();
    });

  </script>
</body>
```


## :sparkles: Implementation Comparisons
Here we compare `offline_search.wasm` to both [elasticlunr.js](https://github.com/weixsong/elasticlunr.js) and [tinysearch](https://github.com/tinysearch/tinysearch). These libraries are very high quality, and their work is greatly appreciated.

#### In comparision to `elasticlunr.js`:
1) Smaller and more efficient.
   * elasticlunr.js ships indexes as a list of strings.
   * offline_search.wasm ships indexes as a [xor_filter](https://github.com/FastFilter/xor_singleheader), which are more space efficient.
2) Does not support languages outside of English.
   * Yet. Stemming and tokenization algorithms in C are welcome for other languages.
3) Support for [stemming](https://en.wikipedia.org/wiki/Stemming).
   * elasticlunr.js uses a [Javascript stemmer](https://github.com/weixsong/elasticlunr.js/blob/master/lib/stemmer.js) based off of the [PorterStemmer](https://tartarus.org/martin/PorterStemmer/index.html).
   * offline_search.wasm compiles the [threadsafe C version of PorterStemmer](https://tartarus.org/martin/PorterStemmer/c_thread_safe.txt) into the WASM. It results in a smaller, optimized binary.
4) Query-Time Boosting.
   * `elasticlunr.js` supports it.
   * `offline_search.wasm` does not.

#### In comparision to `yinysearch`:
1) Written in C.
   * `tinysearch` is written in Rust.
   * offline_search.wasm is written mostly in C and Javascript, and the example implementation relies on Node.js to build the index.
   * Using Javascript should be more familiar to those in the web world, C is a simpler language, and C is eaiser to optimize for size currently.
2) Smaller index size.
   * `tinysearch` utilizes [Bloom Filters](https://en.wikipedia.org/wiki/Bloom_filter) to create indexes.
   * offline_search.wasm utilizes [xor_filters](https://github.com/FastFilter/xor_singleheader) to create indexes. 
      * These are smaller in size than Bloom Filters, and have a smaller false positive rate.
3) More features
   * Stemming through the [PorterStemmer](https://tartarus.org/martin/PorterStemmer/index.html).
   * Stop word filtering.
   * [smaz](https://github.com/antirez/smaz) compression on urls.
   * Fuzzy search through [Metaphone](http://aspell.net/metaphone/), implementation found [here](https://metacpan.org/pod/Text::Metaphone).


## :rotating_light: Tests
Included is a [mocha](https://mochajs.org/) + [chai](https://www.chaijs.com/) + [karma.js](https://karma-runner.github.io/latest/index.html) test suite. It can be ran with the following:
```
npm test
```

To run the Emscripten memory leak analyzer, run:
```
npm run memory_profiler
```

Then to view it:
```
cd build
python3 -m http.server
```
And navigate to `localhost:8000`

As you can see, there are no memory leaks :).

Docker is required to run the tests.

## :rocket: 'Architecture'

### xor_builder
The first part to the library is the xor_builder.js script. This is a 0 dep Node.js script + WebAssembly module that takes the input JSON, and transforms it into a mapping of urls to xor_filter binaries. It tokenizes and applies stemming to the input, distills them down to hashes and a xor filter, then writes the files to the local file system. This is just used to transform the inputs into an index, and will not included in the final output or be used by the browser.

If using Node.js is an issue it shouldn't be too difficult to use that code as a guide to build an index builder in C, which can be run on a WASM Runtime locally, like [Wasmtime](https://github.com/bytecodealliance/wasmtime).

Why is this part written partially in Node.js? It is easier to parse JSON in Node.js than C by default. As time goes on, we may add a C JSON parser, however it isn't a priority as of now.

### offline_search.js
This is the library that your Javascript code should use. It will initialize the Webassembly code and load the index, and make it available for querying.

As of now, we have one global index (as that is the general case for most websites?). However, in the future there are plans to support multiple indexes.

The API is a bit complex. The goal of this library was to return the final result as a string to the Javascript side. This requires some juggling of memory, and should be used carefully.

The flow should be the following:
```c
initialize_index()

// 1..n times
initiate_search();
finalize_search();
// ...

free_index();
```


### Exposed WASM Functions

The following functions are exposed, and can be used for an independent JS implementation:
```c
/**
 * initialize_index
 *
 * Loads indexes from the 'filters' file. This returns the total amount of indexes generated.
 * The return value should be used initialize the memory for the search results.
 *
 * @returns {int} result The number of indicies generated. A result of -1 denotes an error.
 */

int initialize_index();
```

```c
/**
 * initiate_search
 *
 * This function tokenizes the input string and scans the indexes, 
 * and assigns the corresponding indexes to the output_array. The output_array should
 * have been initialized with the result of initialize_index().
 * The return result should be used to initialize memory for the finialize_search function.
 *
 * @param {char*} input The string to be tokenized and searched on. This is required to be
 *                      null terminated.
 * @param {bool*} output_array Array that the indexes will be assigned to.
 * @result {int} result The length of the output string. A value of -1 denotes an error. 
 *                      A value of 0 denotes no results.
 *
 */
int initiate_search(char* input, bool* output_array);
```

```c
/**
 * finalize_search
 *
 * This function finalizes the search results. It takes the passed index mapping, and
 * writes to a pre-allocated array that should have been initialized with the
 * value previously returned by initiate_search.
 * The urls will be deliniated by '||'.
 *
 * @param {bool*} should_add_array The array of urls that should be added.
 * @param {char*} output_array Array that will be filled with the url result.
 * @result {int} result The length of the output string. A value of -1 denotes an error. 
 *                      A value of 0 denotes no results.
 *
 */
void finalize_search(bool* should_add_array, char* output_array)
```

```c
/**
 * free_index
 *
 * This function frees the global index and associated memory.
 *
 */
void free_index()
```

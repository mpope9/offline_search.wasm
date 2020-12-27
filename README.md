# offline-search.wasm

### Warning
This library is a WIP, and not all features present in this README are implemented.

This is a library to generate Webassembly bindings for a full-text search index backed by [xor_filters](https://github.com/FastFilter/xor_singleheader). It is a bastardazation of concepts Tinysearch & lunr.js, and it strives to strike a balance between the highest level of size efficiency, performance, and features.

An article of ~550 words can be compressed to ~97K. This is up to a 147% size decrease from Tinysearch on a per-article basis..

## Features
* Slim size through [Webassembly](https://webassembly.org/).
* [Stemming](https://en.wikipedia.org/wiki/Stemming).
* [Stop word filtering](https://www.elastic.co/guide/en/elasticsearch/reference/current/analysis-stop-tokenfilter.html).
* Docker and non-docker based index building.
* Input from JSON or directly from HTML.

## Implementation Comparisons
Here we compare lunr.wasm to both [elasticlunr.js](https://github.com/weixsong/elasticlunr.js) and [Tinysearch](https://github.com/tinysearch/tinysearch). These libraries are very high quality, and their work is greatly appreciated.

In comparision to `elasticlunr.js`:
1) Smaller and more efficient.
   * elasticlunr.js ships indexes as a list of strings.
   * offline-search.wasm ships indexes as a [xor_filter](https://github.com/FastFilter/xor_singleheader), wihch are more space efficient.
2) Does not support languages outside of English.
   * Yet. Stemming and tokenization algorithms in C are welcome for other languages.
3) Support for [stemming](https://en.wikipedia.org/wiki/Stemming).
   * elasticlunr.js uses a [Javascript stemmer](https://github.com/weixsong/elasticlunr.js/blob/master/lib/stemmer.js) based off of the [PorterStemmer](https://tartarus.org/martin/PorterStemmer/index.html).
   * offline-search.wasm compiles the [C version of PorterStemmer](https://tartarus.org/martin/PorterStemmer/c.txt) into the WASM. It results in a smaller, optimized binary.
4) No support for Query-Time Boosting.
   * I have no intention of implementing this feature, however I am open to a pull request if it is mostly in C code :)

In comparision to `Tinysearch`:
1) Written in C.
   * Tinysearch is written in rust, and requires Cargo and a rather large toolchain for installation.
   * offline-search.wasm is written mostly in C and Javascript, and only relies on Node.js with few dependencies to build the index, and uses vanilla Javascript for the browser.
   * I believe this is more 'familiar' to those in the web world.
2) Smaller index size.
   * Tinysearch utilizes [Bloom Filters](https://en.wikipedia.org/wiki/Bloom_filter) to create indexes.
   * offline-search.wasm utilizes [xor_filters](https://github.com/FastFilter/xor_singleheader) to create indexes. These are smaller in size than Bloom Filters, and have a smaller false positive rate.
3) More features
   * Stemming through the PorterStemmer.
   * Stop word filtering.
   * Mapped URL compression.

The combination of stemming and stop word filtering ultimately end up creating a smaller binary. Removing non-critial words and reducing prefixes and suffixes allows for more overlap in overall words. this in turn allows for a smaller library.

# TODOs
* Non docker builds (pass cmd args to npm build)
* Read from html files directly
  * "mode" configuration option.
* Ship separate vanilla JS files for browser.
* Test suite?

## Usage & Configuration

Docker or Podman needs to be installed to build the binaries. This ensures the highest level of compatibility for building the Webassembly code, in the most contained way. However, if this is a limiting factor, the build scripts are located in `scripts/`.

Example input file with default configuration:
```json
{
   "configuration": 
      {
         "index_output": "_build/output/",
         "filter_binary_output": "filters"
      },
   "values": 
      [
         {
            "title": "Oracle released its latest database Oracle 12g",
            "body": "Yesterday Oracle has released its new database Oracle 12g, this would make more money for this company and lead to a nice profit report of annual year.",
            "mapping": "https://www.google.com"
         }
      ]
}
```

Example input file for reading HTML files located in a directory from specified IDs.
```json
{
   "configuration":
      {
         "index_output": "_build/output/",
         "filter_binary_output": "filters/",
         "mode": "html_parser"
      },
   "values":
      [
         {
            "directory": "content/blog/",
            "ids": ["title", "body"]
         }
      ]
}
```

This will build the WebAssembly and JS glue code to `_build/output/` and the filters to `_build/ouput/`. The `lunr` folder can be copied to your base Javascript directory. The offline-search.wasm
```javascript
```

## Architecture

### Producer
The first part to the library is the Producer. This is a NodeJS + WebAssembly module that takes the input JSON, and transforms it into a xor_filter binaries. It tokenizes and applies stemming to the input, distills them down to hashes, then writes the files to the local file system defined in the configuration chunk. This is just for transformation, and will not be used by the browser.

### offline-search.wasm
This is the library that your Javascript code should use. It will initialize the Webassembly code and load the index.


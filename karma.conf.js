// Karma configuration
// Generated on Sun Jan 03 2021 22:02:17 GMT-0800 (Pacific Standard Time)

module.exports = function(config) {
  config.set({

    // base path that will be used to resolve all patterns (eg. files, exclude)
    basePath: './',

    browserConsoleLogOptions: {
      terminal: true,
      level: ""
    },

    // frameworks to use
    // available frameworks: https://npmjs.org/browse/keyword/karma-adapter
    frameworks: ['mocha', 'chai'],

    // Added this to only run the second test in the test.js file.
    client: {
      args: ['--grep', 'OfflineSearch'],
    },

    // We just want to test `offline_search` for this.
    files: [
      { pattern: 'build/offline_search_wasm.wasm', included: false, served: true, watched: false, nocache: true},
      { pattern: 'build/offline_search_wasm.data', included: false, served: true, watched: false, nocache: true},
      'build/offline_search_wasm.js',
      { pattern: 'lib/offline_search.js', type: 'module' },
      { pattern: 'test/test_search.js', type: 'module' }
    ],

    // Required for .data and .wasm files to be properly loaded.
    proxies: {
      "/": "/base/build/"
    },

    // list of files / patterns to exclude
    exclude: [
      '**/*.swp',
      '**/*.html'
    ],


    // preprocess matching files before serving them to the browser
    // available preprocessors: https://npmjs.org/browse/keyword/karma-preprocessor
    preprocessors: {
    },


    // test results reporter to use
    // possible values: 'dots', 'progress'
    // available reporters: https://npmjs.org/browse/keyword/karma-reporter
    reporters: ['progress'],


    // web server port
    port: 9876,


    // enable / disable colors in the output (reporters and logs)
    colors: true,


    // level of logging
    // possible values: config.LOG_DISABLE || config.LOG_ERROR || config.LOG_WARN || config.LOG_INFO || config.LOG_DEBUG
    logLevel: config.LOG_INFO,

    // enable / disable watching file and executing tests whenever any file changes
    autoWatch: false,


    // start these browsers
    // available browser launchers: https://npmjs.org/browse/keyword/karma-launcher
    browsers: ['ChromeHeadless'],


    // Continuous Integration mode
    // if true, Karma captures browsers, runs the tests and exits
    //singleRun: true,

    // Concurrency level
    // how many browser should be started simultaneous
    concurrency: Infinity
  })
}

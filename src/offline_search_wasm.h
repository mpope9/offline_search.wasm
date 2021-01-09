#ifndef OFFLINE_SEARCH_WASM_H
#define OFFLINE_SEARCH_WASM_H

#include "xorfilter.h"
#include "utils.h"
#include "smaz.h"

typedef struct offline_search_t {
   char** urls;         // Array to store url strings.
   int* urls_lengths;   // Length of each url.
   xor8_t** indexes;    // Filters, 1-to-1 mapping to urls.
   int indexes_length;  // Length of index and urls arrays.
   int indexes_max;     // Length to realloc upon.
} offline_search_t;

#define DEFAULT_OUTPUT_SIZE 32
#define DEFAULT_URL_SIZE 2048

/**
 * initialize_index
 *
 * Loads indexes from the 'filters' file. This returns the total amount of indexes generated.
 * The return value should be used initialize the memory for the search results.
 *
 * @returns {int} result The number of indicies generated. A result of -1 denotes an error.
 */

int initialize_index();

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
 * @param {int*} output_array Array that the indexes will be assigned to.
 * @result {int} result The length of the output string. A value of -1 denotes an error. 
 *                      A value of 0 denotes no results.
 *
 */
int initiate_search(char* input, int* output_array);

/**
 * finalize_search
 *
 * This function finalizes the search results. It takes the passed index mapping, and
 * writes to a pre-allocated array that should have been initialized with the
 * value previously returned by initiate_search.
 * The urls will be deliniated by '||'.
 *
 * @param {int*} urls_index The mapping.
 * @param {char*} output_array Array that will be filled with the url result.
 * @result {int} result The length of the output string. A value of -1 denotes an error. 
 *                      A value of 0 denotes no results.
 *
 */
void finalize_search(int* urls_index, char* output_array);

/**
 * free_index
 *
 * This function frees the global index and associated memory.
 *
 */
void free_index();

#endif

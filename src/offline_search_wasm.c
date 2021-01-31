/**
 * Main driver create a xor filter from a list of strings.
 */
#include <string.h>
#include <emscripten/emscripten.h>

#include "offline_search_wasm.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global variable bad. Only allows fo one index. Move to `offline_search_t`.
char** urls = NULL;
int* urls_lengths = NULL;
xor8_t** indexes = NULL;
int indexes_length = 0;
int indexes_max = DEFAULT_OUTPUT_SIZE;

/**
 * initialize_index
 *
 * Loads indexes from the 'filters' file. This returns the total amount of indexes generated.
 * The return value should be used initialize the memory for the search results.
 *
 * @returns {int} result The number of indicies generated. A result of -1 denotes an error.
 */
EMSCRIPTEN_KEEPALIVE
int initialize_index()
{
   FILE* read_ptr;

   if(urls != NULL || indexes != NULL)
   {
      printf("Index already initialized.");
      return -1;
   }

   urls = (char**) malloc(sizeof(char*) * DEFAULT_OUTPUT_SIZE);
   urls_lengths = (int*) malloc(sizeof(int) * DEFAULT_OUTPUT_SIZE);
   indexes = (xor8_t**) malloc(sizeof(xor8_t*) * DEFAULT_OUTPUT_SIZE); 

   read_ptr = fopen("build/filters", "r");

   if(!read_ptr)
   {
      printf("unable to open filters file\n");
      return -1;
   }

   uint8_t compressed_size;
   while(fread(&compressed_size, sizeof(uint8_t), 1, read_ptr) == 1)
   {

      char compressed[compressed_size + 1];
      fread(compressed, sizeof(char), compressed_size, read_ptr);

      char* url = (char*) malloc(sizeof(char) * DEFAULT_URL_SIZE);

      if(!url)
      {
         printf("malloc failed");
         return -1;
      }

      int decompressed_size =
         smaz_decompress(compressed, compressed_size, url, DEFAULT_URL_SIZE);

      url[decompressed_size] = '\0';

      urls[indexes_length] = url;
      urls_lengths[indexes_length] = strlen(url);

      xor8_t* index = (xor8_t*) malloc(sizeof(xor8_t));

      fread(&(index->seed), sizeof(index->seed), 1, read_ptr);
      fread(&(index->blockLength), sizeof(index->blockLength), 1, read_ptr);
      index->fingerprints = (uint8_t*) malloc(sizeof(uint8_t) * 3 * index->blockLength);
      fread(index->fingerprints, sizeof(uint8_t) * 3 * index->blockLength, 1, read_ptr);

      indexes[indexes_length] = index;

      indexes_length++;

      // Realloc indexes.
      if(indexes_length == indexes_max)
      {
         indexes_max *= 2;
         urls = (char**) realloc(urls, indexes_max * sizeof(char*));
         indexes = (xor8_t**) realloc(indexes, indexes_max * sizeof(xor8_t*));
      }

   }
   fclose(read_ptr);

   return indexes_length;
}

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
 * @param {int8_t*} output_array Array that the indexes will be assigned to.
 * @result {int} result The length of the output string. A value of -1 denotes an error. 
 *                      A value of 0 denotes no results.
 *
 */
EMSCRIPTEN_KEEPALIVE
int initiate_search(char* input, int8_t* output_array)
{

   if(urls == NULL || indexes == NULL || indexes_length == 0)
   {
      printf("Index is not initialized.");
      return -1;
   }

   if(input == NULL || output_array == NULL)
   {
      printf("Input array not initialized.");
      return -1;
   }

   int tokens_length = 0;

   char** tokens = tokenize(input, &tokens_length);

   if(tokens_length == 0)
   {
      return 0;
   }

   uint64_t hashes[tokens_length];
   int hashes_length = 0;

   stem_n_hash(tokens, tokens_length, hashes, &hashes_length);

   // Free tokens.
   for(int i = 0; i < tokens_length; ++i)
   {
      free(tokens[i]);
   }
   free(tokens);

   // Skip the overhead of deduping hashes.

   int output_size = 0;
   bool should_add = true;
   for(int i = 0; i < indexes_length; i++) 
   {
      for(int j = 0; j < hashes_length; j++)
      {
         if(!xor8_contain(hashes[j], indexes[i])) 
         {
            should_add = false;
            break;
         }
      }

      if(should_add)
      {
         output_array[i] = 0;
         // Plus two for the '||' deliminators.
         output_size += urls_lengths[i] + 2;
      }
      else
      {
         output_array[i] = 1;
      }
      should_add = true;
   }

   return output_size;
}

/**
 * finalize_search
 *
 * This function finalizes the search results. It takes the passed index mapping, and
 * writes to a pre-allocated array that should have been initialized with the
 * value previously returned by initiate_search.
 * The urls will be deliniated by '||'.
 *
 * @param {int8_t*} should_add_array The array of urls that should be added.
 * @param {char*} output_array Array that will be filled with the url result.
 * @result {int} result The length of the output string. A value of -1 denotes an error. 
 *                      A value of 0 denotes no results.
 *
 */
EMSCRIPTEN_KEEPALIVE
void finalize_search(int8_t* should_add_array, char* output_array)
{

   if(urls == NULL)
   {
      printf("Index is not initialized.");
      return;
   }

   if(should_add_array == NULL)
   {
      printf("urls_index not initialized.");
      return;
   }
   
   if(output_array == NULL)
   {
      printf("output_array not initialized");
      return;
   }

   int output_index = 0;
   for(int i = 0; i < indexes_length; ++i)
   {
      if(should_add_array[i])
      {
         continue;
      }

      char* url = urls[i];
      int url_length = urls_lengths[i];

      for(int j = 0; j < url_length; j++)
      {
         output_array[output_index] = url[j];
         output_index++;
      }
      output_array[output_index] = '|';
      output_index++;
      output_array[output_index] = '|';
      output_index++;
   }
}

/**
 * free_index
 *
 * This function frees the global index and associated memory.
 *
 */
EMSCRIPTEN_KEEPALIVE
void free_index()
{
   if(urls != NULL)
   {
      for(int i = 0; i < indexes_length; i++)
      {
         free(urls[i]);
      }
      free(urls);
   }

   if(urls_lengths != NULL)
   {
      free(urls_lengths);
   }

   if(indexes != NULL)
   {
      for(int i = 0; i < indexes_length; i++)
      {
         free(indexes[i]->fingerprints);
         free(indexes[i]);
      }
      free(indexes);
   }
}

#ifdef __cplusplus
}
#endif

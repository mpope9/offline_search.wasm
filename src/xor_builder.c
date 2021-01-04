/**
 * Main driver create a xor filter from a list of strings.
 */
#include <stdio.h>
#include <string.h>
#include <emscripten/emscripten.h>

#include "xorfilter.h"
#include "utils.h"
#include "smaz.h"

#ifdef NODERAWFS
#define CWD "build/"
#else
#define CWD "/working/"
#endif

#ifdef __cplusplus
extern "C" {
#endif

bool fileSystemMounted = false;

/**
 * Function for Quicksort.
 */
int comp_fun(const void* a, const void* b)
{
   uint64_t i1 = *(uint64_t*) a;
   uint64_t i2 = *(uint64_t*) b;

   if(i1 < i2) return -1;
   else if (i1 == i2) return 0;
   else return 1;
}

/**
 * Quicksorts array, then iterates over to remove duplicates.
 */
void deduplicate_array(uint64_t* input, int length, int* new_length_input)
{
   qsort(input, length, sizeof(uint64_t), comp_fun);

   int new_length = 0;
   int i = 0;
   uint64_t prev_element = input[i];
   for(i = 1; i < length; ++i)
   {
      if(input[i] != prev_element)
      {
         input[new_length] = input[i];
         new_length++;
      }
      prev_element = input[i];
   }

   *new_length_input = new_length;
}

/**
 * build_xor
 *
 * Takes in list of strings, and a file name.
 * Performs stop word filtering and stemming on the input strings.
 */
EMSCRIPTEN_KEEPALIVE 
bool build_xor(char* input, char* url)
{
   xor8_t filter;
   FILE* write_ptr;

   char** tokens = NULL;
   int tokens_length = 0;

   tokenize(input, &tokens, &tokens_length);

   // Could move stemming and hashing into the tokenization loop. But, noodles to that.
   uint64_t hashes[tokens_length];
   int hashes_length = 0;

   // Stemming, hashing.
   stem_n_hash(tokens, tokens_length, hashes, &hashes_length);

   int deduped_hashes_length = 0;
   deduplicate_array(hashes, hashes_length, &deduped_hashes_length);

#ifndef NODERAWFS
   if(!fileSystemMounted)
   {
      printf("Node fs detected, mounting 'build' directory.\n");
      EM_ASM(
         FS.mkdir('/working');
         FS.mount(NODEFS, { root: './build/' }, '/working');
      );
      fileSystemMounted = true;
   }
#endif

   // Allocate and populate filter.
   xor8_allocate(deduped_hashes_length, &filter);
   
   if(!xor8_populate(hashes, deduped_hashes_length, &filter))
   {
      printf("Initialization of filter failed due to duplicates.\n");
      return false;
   }

   // Serialize url and filter to file.
   uint64_t seed = filter.seed;
   uint64_t block_length = filter.blockLength;

   write_ptr = fopen(CWD "filters", "a");

   if(!write_ptr)
   {
      printf("unable to open file\n");
      return false;
   }

   // Compress url using smaz.
   int url_size = strlen(url);
   char compressed[4096];
   uint8_t compressed_size = smaz_compress(url, url_size, compressed, sizeof(compressed));

   fwrite(&compressed_size, sizeof(compressed_size), 1, write_ptr);
   fwrite(&compressed, sizeof(char) * compressed_size, 1, write_ptr);

   fwrite(&seed, sizeof(seed), 1, write_ptr);
   fwrite(&block_length, sizeof(block_length), 1, write_ptr);
   fwrite(filter.fingerprints, sizeof(uint8_t) * 3 * block_length, 1, write_ptr);

   // Cleanup
   fclose(write_ptr);
   xor8_free(&filter);

   return true;
}

#ifdef __cplusplus
}
#endif

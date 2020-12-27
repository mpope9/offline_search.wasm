/**
 * Main driver create a xor filter from a list of strings.
 */
#include <stdio.h>
#include <string.h>
#include <emscripten/emscripten.h>

#include "porter_stemmer.h"
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
   for(int i = 0; i < tokens_length; i++)
   {
      char* token = tokens[i];
      int token_length = strlen(token);
      
      // Stemming, modifies string in-place, and needs null termination.
      int new_length = stem(token, 0, token_length - 1);
      if(new_length < token_length)
      {
         token[new_length + 1] = '\0';
      }

      hashes[hashes_length] = hash_token(token);
      hashes_length++;
   }

   int deduped_hashes_length = 0;
   deduplicate_array(hashes, hashes_length, &deduped_hashes_length);

   // Mount node FS.
#ifndef NODERAWFS
   printf("Node fs detected, mounting 'build' directory.\n");
   EM_ASM(
      FS.mkdir('/working');
      FS.mount(NODEFS, { root: './build/' }, '/working');
   );
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
   int compressed_size = smaz_compress(url, url_size, compressed, sizeof(compressed));

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

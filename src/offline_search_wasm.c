/**
 * Main driver create a xor filter from a list of strings.
 */
#include <string.h>
#include <emscripten/emscripten.h>

#include "porter_stemmer.h"
#include "xorfilter.h"
#include "utils.h"
#include "smaz.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BEGIN_URLS_SIZE 1
#define DEFAULT_URL_SIZE 2048

char** urls;
xor8_t** indexes;

/**
 * initialize_index
 *
 * Loads indexes from the 'filters' file.
 */
EMSCRIPTEN_KEEPALIVE
bool initialize_index()
{
   FILE* read_ptr;

   urls = (char**) malloc(sizeof(char*) * BEGIN_URLS_SIZE);
   indexes = (xor8_t**) malloc(sizeof(xor8_t*) * BEGIN_URLS_SIZE); 

   read_ptr = fopen("filters", "r");

   if(!read_ptr)
   {
      printf("unable to open filters file\n");
      return false;
   }

   int compressed_size;
   fread(&compressed_size, sizeof(compressed_size), 1, read_ptr);

   char* url = (char*) malloc(sizeof(char) * DEFAULT_URL_SIZE);

   if(!url)
   {
      printf("malloc failed");
      return false;
   }

   char compressed[compressed_size + 1];
   fread(compressed, compressed_size, 1, read_ptr);

   smaz_decompress(compressed, compressed_size, url, DEFAULT_URL_SIZE);

   printf("%s\n", url);
   urls[0] = url;

   xor8_t* index = (xor8_t*) malloc(sizeof(xor8_t));

   fread(&(index->seed), sizeof(index->seed), 1, read_ptr);
   fread(&(index->blockLength), sizeof(index->blockLength), 1, read_ptr);
   fread(index->fingerprints, sizeof(uint8_t) * 3 * index->blockLength, 1, read_ptr);
   fclose(read_ptr);

   indexes[0] = index;

   return true;
}

EMSCRIPTEN_KEEPALIVE
bool search(char* input)
{

   char** tokens = NULL;
   int tokens_length = 0;

   tokenize(input, &tokens, &tokens_length);

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

   for(int i = 0; i < hashes_length; i++)
   {
      if(xor8_contain(hashes[i], indexes[i])) {
         return true;
      }
   }
   return false;
}

#ifdef __cplusplus
}
#endif

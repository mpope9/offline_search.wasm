/**
 * utils.h
 *
 * Shared utilities between the xor_builder and indexing code.
 */

#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "stop_words.h"
#include "porter_stemmer.h"
#include "metaphone.h"
#include "xxhash64.h"

#define INITIAL_TOKENS_SIZE 256
#define STRTOK_SEPERATORS " \n\r\t;&.,:"

#define MAX_PHONEMES 6

#define HASH_SEED 100

/**
 * Tests if a word is a 'stop word'.
 */
bool is_stop_word(char* word)
{
   if(strlen(word) < 4)
   {
      return true;
   }
   for(int i = 0; i < STOPWORD_LENGTH; i++)
   {
      if(strcmp(word, STOP_WORD_CORPUS[i]) == 0)
      {
         return true;
      }
   }
   return false;
}


/**
 * Removes non-alphabetic characters from a string, converts it to lowercase, null terminates at
 * the new length.
 */
void clean_token(char* token)
{
   int j = 0;
   char chr = token[j];
   int token_length = 0;

   while(chr)
   {
      if(isalpha(chr))
      {
         token[token_length] = tolower(chr);
         token_length++;
      }
      j++;
      chr = token[j];
   }

   // Don't forget the null terminator.
   token[token_length] = '\0';
}


/**
 * Suboptimal tokenization. Okay for building the index, good enough for limited search input.
 * Breaks strings into sanitized tokens, checks if it is a stop word, then adds
 * to the token output.
 */
char** tokenize(char* input, int* tokens_length)
{
   int max_length = INITIAL_TOKENS_SIZE;
   int length = 0;
   char** tokens = (char**) malloc(max_length * sizeof(char*));

   char* current_substr = strtok(input, STRTOK_SEPERATORS);

   while(current_substr != NULL)
   {
      size_t current_substr_len = strlen(current_substr);
      char* curr = (char*) malloc(current_substr_len * sizeof(char));
      snprintf(curr, current_substr_len + 1, "%s", current_substr);

      clean_token(curr);

      if(!is_stop_word(curr))
      {
         tokens[length] = curr;
         length++;
      }
      else
      {
         free(curr);
      }
      current_substr = strtok(NULL, STRTOK_SEPERATORS);

      // Need more memory.
      if(length == max_length)
      {
         max_length *= 2;
         tokens = (char**) realloc(tokens, max_length * sizeof(char*));
      }
   }

   *tokens_length = length;
   return tokens;
}

/**
 * Combined stemming, fuzzysearch (metaphone), and hashing.
 */
void stem_n_hash(char** tokens, int tokens_length, uint64_t* hashes, int* hashes_length_out)
{

   // Stemming, modifies string in-place, and needs null termination.
   struct stemmer* stemmer = create_stemmer();

   int hashes_length = 0;
   for(int i = 0; i < tokens_length; i++)
   {
      char* token = tokens[i];
      int token_length = strlen(token);

      int new_length = stem(stemmer, token, token_length - 1);
      if(new_length < token_length)
      {
         token[new_length + 1] = '\0';
      }

      char* meta = metaphone(token, MAX_PHONEMES);

      hashes[hashes_length] = hash(meta, strlen(meta), HASH_SEED);
      hashes_length++;

      free(meta);

      // Reset stemmer
      stemmer->k = 0;
      stemmer->j = 0;
      stemmer->b = NULL;
   }

   free_stemmer(stemmer);

   *hashes_length_out = hashes_length;
}

#endif

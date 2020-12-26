/**
 * Utils!
 */
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#define INITIAL_TOKENS_SIZE 256

const char* STOP_WORD_CORPUS[] =
{
   "",
   "i",
   "me",
   "my",
   "myself",
   "we",
   "our",
   "ours",
   "ourselves",
   "you",
   "your",
   "yours",
   "yourself",
   "yourselves",
   "he",
   "him",
   "his",
   "himself",
   "she",
   "her",
   "hers",
   "herself",
   "it",
   "its",
   "itself",
   "they",
   "them",
   "their",
   "theirs",
   "themselves",
   "what",
   "which",
   "who",
   "whom",
   "this",
   "that",
   "these",
   "those",
   "am",
   "is",
   "are",
   "was",
   "were",
   "be",
   "been",
   "being",
   "have",
   "has",
   "had",
   "having",
   "do",
   "does",
   "did",
   "doing",
   "a",
   "an",
   "the",
   "and",
   "but",
   "if",
   "or",
   "because",
   "as",
   "until",
   "while",
   "of",
   "at",
   "by",
   "for",
   "with",
   "about",
   "against",
   "between",
   "into",
   "through",
   "during",
   "before",
   "after",
   "above",
   "below",
   "to",
   "from",
   "up",
   "down",
   "in",
   "out",
   "on",
   "off",
   "over",
   "under",
   "again",
   "further",
   "then",
   "once",
   "here",
   "there",
   "when",
   "where",
   "why",
   "how",
   "all",
   "any",
   "both",
   "each",
   "few",
   "more",
   "most",
   "other",
   "some",
   "such",
   "no",
   "nor",
   "not",
   "only",
   "own",
   "same",
   "so",
   "than",
   "too",
   "very",
   "can",
   "will",
   "just",
   "should",
   "now"
};

bool is_stop_word(char* word)
{
   for(int i = 0; i < 126; i++)
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
 * Breaks strings into tokens, cleans the tokens, checks if it is a stop word, then adds
 * to the token output.
 */
void tokenize(char* input, char*** tokens_input, int* tokens_length)
{
   int max_length = INITIAL_TOKENS_SIZE;
   int length = 0;
   char** tokens = (char**) malloc(max_length * sizeof(char*));

   char* current_substr = strtok(input, " \n\r\t");

   while(current_substr != NULL)
   {
      size_t current_substr_len = strlen(current_substr);
      char* curr = (char*) malloc(current_substr_len * sizeof(char));
      strncat(curr, current_substr, current_substr_len);

      clean_token(curr);

      if(!is_stop_word(curr))
      {
         tokens[length] = curr;
         length++;
      }
      current_substr = strtok(NULL, " \n\r\t");

      // Need more memory.
      if(length == max_length)
      {
         max_length *= 2;
         // TODO: Should check for realloc error.
         tokens = (char**) realloc(tokens, max_length * sizeof(char*));
      }
   }

   *tokens_length = length;
   *tokens_input = tokens;
}


/**
 * djb2 hashing.
 */
uint64_t hash_token(char* str)
{
   uint64_t hash = 5381;
   int c;

   while((c = *str++))
   {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   }

   return hash;
}

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

/*
 * Metaphone library, source:
 * https://metacpan.org/pod/Text::Metaphone
 *
 * @author: Michael G Schwern <schwern@pobox.com>
 */

#ifndef __METAPHONE_H__
#define __METAPHONE_H__

#include "metaphone.h"
#include "metachar.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Special encodings */
#define  SH    'X'
#define  TH '0'

/* I suppose I could have been using a character pointer instead of
 * accesssing the array directly... */

/* Look at the next letter in the word */
#define Next_Letter (toupper(word[w_idx+1]))
/* Look at the current letter in the word */
#define Curr_Letter (toupper(word[w_idx]))
/* Go N letters back. */
#define Look_Back_Letter(n) (w_idx >= n ? toupper(word[w_idx-n]) : '\0') 
/* Previous letter.  I dunno, should this return null on failure? */
#define Prev_Letter (Look_Back_Letter(1))
/* Look two letters down.  It makes sure you don't walk off the string. */
#define After_Next_Letter   (Next_Letter != '\0' ? toupper(word[w_idx+2]) \
      : '\0')
#define Look_Ahead_Letter(n) (toupper(Lookahead(word+w_idx, n)))


/* Allows us to safely look ahead an arbitrary # of letters */
/* I probably could have just used strlen... */
char Lookahead(char * word, int how_far) {
   char letter_ahead = '\0';  /* null by default */
   int idx;
   for(idx = 0;  word[idx] != '\0' && idx < how_far;  idx++);
   /* Edge forward in the string... */

   letter_ahead = word[idx];  /* idx will be either == to how_far or
                               * at the end of the string
                               */
   return letter_ahead;
}


/* phonize one letter */
#define Phonize(c)  {phoned_word[p_idx++] = c;}
/* How long is the phoned word? */
#define Phone_Len   (p_idx)

/* Note is a letter is a 'break' in the word */
#define Isbreak(c)  (!isalpha(c))


char *metaphone ( char *word, size_t max_phonemes ) {
   int w_idx   = 0;    /* point in the phonization we're at. */
   int p_idx   = 0;    /* end of the phoned phrase */
   char *phoned_word;

   /* Assume largest possible if we're given no limit */
   if( max_phonemes == 0 )
      max_phonemes = strlen(word);

   /* It's +2 because X -> KS can result in the phoned word being
      one larger than the original word.
      */
   phoned_word = calloc( max_phonemes + 2, sizeof(char) );

   /*-- The first phoneme has to be processed specially. --*/
   /* Find our first letter */
   for( ;  !isalpha(Curr_Letter);  w_idx++ ) {
      /* On the off chance we were given nothing but crap... */
      if( Curr_Letter == '\0' ) {
         return phoned_word;
      }
   }   

   switch (Curr_Letter) {
      /* AE becomes E */
      case 'A':
         if( Next_Letter == 'E' ) {
            Phonize('E');
            w_idx+=2;
         }
         /* Remember, preserve vowels at the beginning */
         else {
            Phonize('A');
            w_idx++;
         }
         break;
         /* [GKP]N becomes N */
      case 'G':
      case 'K':
      case 'P':
         if( Next_Letter == 'N' ) {
            Phonize('N');
            w_idx+=2;
         }
         break;
         /* WH becomes H, 
            WR becomes R 
            W if followed by a vowel */
      case 'W':
         if( Next_Letter == 'H' ||
               Next_Letter == 'R' ) 
         {
            Phonize(Next_Letter);
            w_idx+=2;
         }
         else if ( isvowel(Next_Letter) ) {
            Phonize('W');
            w_idx+=2;
         }
         /* else ignore */
         break;
         /* X becomes S */
      case 'X':
         Phonize('S');
         w_idx++;
         break;
         /* Vowels are kept */
         /* We did A already
            case 'A':
            case 'a':
            */
      case 'E':
      case 'I':
      case 'O':
      case 'U':
         Phonize(Curr_Letter);
         w_idx++;
         break;
      default:
         /* do nothing */
         break;
   }



   /* On to the metaphoning */
   for(;
         Curr_Letter != '\0'     && 
         (max_phonemes == 0 || Phone_Len < max_phonemes);  
         w_idx++
      ) {
      /* How many letters to skip because an eariler encoding handled     
       * multiple letters */
      unsigned short int skip_letter = 0; 


      /* THOUGHT:  It would be nice if, rather than having things like...
       * well, SCI.  For SCI you encode the S, then have to remember
       * to skip the C.  So the phonome SCI invades both S and C.  It would
       * be better, IMHO, to skip the C from the S part of the encoding.
       * Hell, I'm trying it.
       */

      /* Ignore non-alphas */
      if( !isalpha(Curr_Letter) )
         continue;

      /* Drop duplicates, except CC */
      if( Curr_Letter == Prev_Letter &&
            Curr_Letter != 'C' )
         continue;

      switch (Curr_Letter) {
         /* B -> B unless in MB */
         case 'B':
            if( Prev_Letter != 'M' )
               Phonize('B');
            break;
            /* 'sh' if -CIA- or -CH, but not SCH, except SCHW.
             * (SCHW is handled in S)
             *  S if -CI-, -CE- or -CY-
             *  dropped if -SCI-, SCE-, -SCY- (handed in S)
             *  else K
             */
         case 'C':
            if( MAKESOFT(Next_Letter) ) {   /* C[IEY] */
               if( After_Next_Letter == 'A' &&
                     Next_Letter == 'I' ) { /* CIA */
                  Phonize(SH);
               }
               /* SC[IEY] */
               else if ( Prev_Letter == 'S' ) {
                  /* Dropped */
               }
               else {
                  Phonize('S');
               }
            }
            else if ( Next_Letter == 'H' ) {
               if( After_Next_Letter == 'R' ||
                     Prev_Letter == 'S' ) { /* Christ, School */
                  Phonize('K');
               }
               else {
                  Phonize(SH);
               }
               skip_letter++;
            }
            else {
               Phonize('K');
            }
            break;
            /* J if in -DGE-, -DGI- or -DGY-
             * else T
             */
         case 'D':
            if( Next_Letter == 'G' &&
                  MAKESOFT(After_Next_Letter) ) {
               Phonize('J');
               skip_letter++;
            }
            else
               Phonize('T');
            break;
            /* F if in -GH and not B--GH, D--GH, -H--GH, -H---GH
             * else dropped if -GNED, -GN, 
             * else dropped if -DGE-, -DGI- or -DGY- (handled in D)
             * else J if in -GE-, -GI, -GY and not GG
             * else K
             */
         case 'G':
            if( Next_Letter == 'H' ) {
               if( !( NOGHTOF(Look_Back_Letter(3)) ||
                        Look_Back_Letter(4) == 'H' ) ) {
                  Phonize('F');
                  skip_letter++;
               }
               else {
                  /* silent */
               }
            }
            else if( Next_Letter == 'N' ) {
               if( Isbreak(After_Next_Letter) ||
                     ( After_Next_Letter == 'E' &&
                       Look_Ahead_Letter(3) == 'D' ) ) {
                  /* dropped */
               }
               else
                  Phonize('K');
            }
            else if( MAKESOFT(Next_Letter) &&
                  Prev_Letter != 'G' ) {
               Phonize('J');
            }
            else {
               Phonize('K');
            }
            break;
            /* H if before a vowel and not after C,G,P,S,T */
         case 'H':
            if( isvowel(Next_Letter) &&
                  !AFFECTH(Prev_Letter) )
               Phonize('H');
            break;
            /* dropped if after C
             * else K
             */
         case 'K':
            if( Prev_Letter != 'C' )
               Phonize('K');
            break;
            /* F if before H
             * else P
             */
         case 'P':
            if( Next_Letter == 'H' ) {
               Phonize('F');
            }
            else {
               Phonize('P');
            }
            break;
            /* K
            */
         case 'Q':
            Phonize('K');
            break;
            /* 'sh' in -SH-, -SIO- or -SIA- or -SCHW-
             * else S
             */
         case 'S':
            if( Next_Letter == 'I' &&
                  ( After_Next_Letter == 'O' ||
                    After_Next_Letter == 'A' ) ) {
               Phonize(SH);
            }
            else if ( Next_Letter == 'H' ) {
               Phonize(SH);
               skip_letter++;
            }
            else if ( Next_Letter == 'C' &&
                  Look_Ahead_Letter(2) == 'H' &&
                  Look_Ahead_Letter(3) == 'W' ) {
               Phonize(SH);
               skip_letter += 2;
            }
            else {
               Phonize('S');
            }
            break;
            /* 'sh' in -TIA- or -TIO-
             * else 'th' before H
             * else T
             */
         case 'T':
            if( Next_Letter == 'I' &&
                  ( After_Next_Letter == 'O' ||
                    After_Next_Letter == 'A' ) ) {
               Phonize(SH);
            }
            else if ( Next_Letter == 'H' ) {
               Phonize(TH);
               skip_letter++;
            }
            else {
               Phonize('T');
            }
            break;
            /* F */
         case 'V':
            Phonize('F');
            break;
            /* W before a vowel, else dropped */
         case 'W':
            if( isvowel(Next_Letter) )
               Phonize('W');
            break;
            /* KS */
         case 'X':
            Phonize('K');
            Phonize('S');
            break;
            /* Y if followed by a vowel */
         case 'Y':
            if( isvowel(Next_Letter) )
               Phonize('Y');
            break;
            /* S */
         case 'Z':
            Phonize('S');
            break;
            /* No transformation */
         case 'F':
         case 'J':
         case 'L':
         case 'M':
         case 'N':
         case 'R':
            Phonize(Curr_Letter);
            break;
         default:
            /* nothing */
            break;
      }

      w_idx += skip_letter;
   }

   return phoned_word;
}

#endif
/* awa5x - Extended AWA5.0
   Copyright © 2024 Starknights

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "opcodes.h"
#include "eval.h"
#include "abyss.h"
#include "strtoawa.h"

#ifndef HAVE_GETLINE
#include "getline.h"
#endif

static enum EvalCode
prn_bubble(struct Bubble bubble) {
     if (0 == bubble_double(bubble)) {
          uint8_t uvalue = bubble.value;
          if (uvalue > NALNUM) {
               return EVAL_ERROR;
          }

          fprintf(stdout, "%c", ALPHABET[uvalue]);

          return EVAL_OK;
     }

     enum EvalCode code = EVAL_OK;
     for (struct Bubble *b=bubble.head; NULL!=b; b=b->next) {
          code = prn_bubble(*b);
     }

     return code;
}

static enum EvalCode
pr1_bubble(struct Bubble bubble) {
     if (0 == bubble_double(bubble)) {
          if (bubble.value < 0) {
               fprintf(stdout, "~%d", -bubble.value);
          } else {
               fprintf(stdout, "%d", bubble.value);
          }

          return EVAL_OK;
     }

     enum EvalCode code = EVAL_OK;
     for (struct Bubble *b=bubble.head; NULL!=b; b=b->next) {
          code = pr1_bubble(*b);

          if (EVAL_OK == code && NULL != b->next) {
               fprintf(stdout, " ");
          }
     }

     return code;
}

struct EvalResult
eval_prn(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;

     struct Bubble top = abyss_top(abyss);
     result.code = prn_bubble(top);
     fflush(stdout);

     result.code = EVAL_NEW_STATE;
     result.state = abyss_big_pop(abyss);

     return result;
}

struct EvalResult
eval_pr1(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;

     struct Bubble top = abyss_top(abyss);
     result.code = pr1_bubble(top);
     fflush(stdout);

     result.code = EVAL_NEW_STATE;
     result.state = abyss_big_pop(abyss);

     return result;
}

struct EvalResult
eval_red(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     ssize_t bytes = getline(&(result.state.exbuffer), &(result.state.exsize), stdin);
     if (0 > bytes) {
          // errno should probably be checked here but whatever, users
          // should send empty strings instead of end-of-file
          result.code = EVAL_ERROR;
          return result;
     }

     if (1 < bytes) {
          // avoids the newline at the end
          result.state.exbuffer[bytes-1] = '\0';
          bytes = bytes-1;
     }

     int valid = 0;
     while (0 == valid) {
          // the whole thing works only in single bytes (check the alphabet)
          for (ssize_t i=0; i<bytes; ++i) {
               int8_t byte = result.state.exbuffer[i];

               for (int8_t c=0; c<NALNUM; ++c) {
                    if (ALPHABET[c] != byte) {
                         continue;
                    }

                    result.state.exbuffer[i] = c;
                    valid = valid + 1;
               }
          }

          if (bytes != valid) {
               // since this is basically an interactive command, it can't
               // really kill the whole thing if the user sends invalid input,
               // but recovering is also hard so we just loop until a valid
               // value is given
               fprintf(stderr, "input out of range\n");
               valid = 0;
               bytes = getline(&(result.state.exbuffer), &(result.state.exsize), stdin);
               if (0 > bytes) {
                    result.code = EVAL_ERROR;
                    return result;
               }
               bytes = bytes - 1;
          }
     }

     struct Bubble bubble = { 0 };
     for (ssize_t i=bytes-1; i>-1; --i) {
          bubble = bubble_wrap(result.state.exbuffer[i]);
          result.state = abyss_push(result.state, bubble);
     }
     result.code = EVAL_NEW_STATE;
     result.state = abyss_join(result.state, (int8_t)bytes); // it's fine to truncate

     return result;
}

struct EvalResult
eval_r3d(struct Abyss abyss, int8_t parameter) {
     // this is mostly the same as eval_red.
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     ssize_t bytes = getline(&(result.state.exbuffer), &(result.state.exsize), stdin);
     if (0 > bytes) {
          result.code = EVAL_ERROR;
          return result;
     }
     bytes = bytes - 1;
     result.state.exbuffer[bytes] = '\0';

     // strtoawa is just strtol where negative numbers start with ~
     // instead of the minus sign; strtol itself is not the best
     // choice for a number of reasons, but it's good enough™.
     char *tail = NULL;
     long int cnum = strtoawa(result.state.exbuffer, &tail);

     // again we loop until a valid input
     while ((NULL != tail && '\0' != tail[0]) || (INT32_MIN > cnum || INT32_MAX < cnum)) {
          fprintf(stderr, "input out of range\n");

          bytes = getline(&(result.state.exbuffer), &(result.state.exsize), stdin);
          if (0 > bytes) {
               result.code = EVAL_ERROR;
               return result;
          }
          bytes = bytes - 1;
          result.state.exbuffer[bytes] = '\0';

          cnum = strtoawa(result.state.exbuffer, &tail);
     }

     struct Bubble bubble = bubble_wrap((int32_t)cnum);
     result.code = EVAL_NEW_STATE;
     result.state = abyss_push(result.state, bubble);

     return result;
}

struct EvalResult
eval_blo(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     struct Bubble bubble = bubble_wrap(parameter);
     result.code = EVAL_NEW_STATE;
     result.state = abyss_push(result.state, bubble);

     return result;
}

struct EvalResult
eval_sbm(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_move(result.state, parameter);

     return result;
}

struct EvalResult
eval_pop(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_pop(result.state);

     return result;
}

struct EvalResult
eval_dpl(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_clone(result.state);

     return result;
}

struct EvalResult
eval_srn(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_join(result.state, parameter);

     return result;
}

struct EvalResult
eval_mrg(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_merge(result.state);

     return result;
}

struct EvalResult
eval_4dd(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_sum(result.state);

     return result;
}

struct EvalResult
eval_sub(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_sub(result.state);

     return result;
}

struct EvalResult
eval_mul(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_mul(result.state);

     return result;
}

struct EvalResult
eval_div(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     result.code = EVAL_NEW_STATE;
     result.state = abyss_div(result.state);

     return result;
}

struct EvalResult
eval_cnt(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     int value = bubble_count(*(result.state.head));
     struct Bubble bubble = bubble_wrap((uint8_t)value); // should be fine to truncate

     result.code = EVAL_NEW_STATE;
     result.state = abyss_push(result.state, bubble);

     return result;
}

struct EvalResult
eval_eql(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     if (NULL == result.state.head || NULL == result.state.head->next) {
          result.code = EVAL_NO;
          return result;
     }

     int value = bubble_equals(*(result.state.head), *(result.state.head->next));
     result.code = (0 == value) ? EVAL_NO : EVAL_YES;
     return result;
}

struct EvalResult
eval_lss(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     if (NULL == result.state.head || NULL == result.state.head->next) {
          result.code = EVAL_NO;
          return result;
     }

     int value = bubble_lessers(*(result.state.head), *(result.state.head->next));
     result.code = (0 == value) ? EVAL_NO : EVAL_YES;
     return result;
}

struct EvalResult
eval_gr8(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     if (NULL == result.state.head || NULL == result.state.head->next) {
          result.code = EVAL_NO;
          return result;
     }

     int value = bubble_greaters(*(result.state.head), *(result.state.head->next));
     result.code = (0 == value) ? EVAL_NO : EVAL_YES;
     return result;
}

struct EvalResult
eval_eqz(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;
     result.state = abyss;

     if (NULL == result.state.head) {
          result.code = EVAL_NO;
          return result;
     }

     int value = bubble_zero(*(result.state.head));
     result.code = (0 == value) ? EVAL_NO : EVAL_YES;
     return result;
}

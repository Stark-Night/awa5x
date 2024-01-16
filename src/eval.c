#include "config.h"

#include <stdio.h>
#include "opcodes.h"
#include "eval.h"
#include "abyss.h"

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
          fprintf(stdout, "%d", bubble.value);

          return EVAL_OK;
     }

     enum EvalCode code = EVAL_OK;
     for (struct Bubble *b=bubble.head; NULL!=b; b=b->next) {
          code = pr1_bubble(*b);
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

     return result;
}

struct EvalResult
eval_pr1(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;

     struct Bubble top = abyss_top(abyss);
     result.code = pr1_bubble(top);
     fflush(stdout);

     return result;
}

struct EvalResult
eval_red(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;

     ssize_t bytes = getline(&(abyss.exbuffer), &(abyss.exsize), stdin);
     if (0 > bytes) {
          // errno should probably be checked here but whatever, users
          // should send empty strings instead of end-of-file
          result.code = EVAL_ERROR;
          return result;
     }

     // avoids the newline at the end
     bytes = bytes - 1;

     int valid = 0;
     while (0 == valid) {
          // the whole thing works only in single bytes (check the alphabet)
          for (ssize_t i=0; i<bytes; ++i) {
               int8_t byte = abyss.exbuffer[i];

               for (int8_t c=0; c<NALNUM; ++c) {
                    if (ALPHABET[c] != byte) {
                         continue;
                    }

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
               bytes = getline(&(abyss.exbuffer), &(abyss.exsize), stdin);
               if (0 > bytes) {
                    result.code = EVAL_ERROR;
                    return result;
               }
               bytes = bytes - 1;
          }
     }

     struct Bubble bubble = { 0 };
     for (ssize_t i=0; i<bytes; ++i) {
          bubble = bubble_wrap(abyss.exbuffer[i]);
          result.state = abyss_push(result.state, bubble);
     }
     result.code = EVAL_NEW_STATE;
     result.state = abyss_join(result.state, (int8_t)bytes); // it's fine to truncate

     return result;
}

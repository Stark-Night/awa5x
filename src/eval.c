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

struct EvalResult
eval_prn(struct Abyss abyss, int8_t parameter) {
     struct EvalResult result = { 0 };
     result.code = EVAL_OK;

     struct Bubble top = abyss_top(abyss);
     result.code = prn_bubble(top);
     fflush(stdout);

     return result;
}

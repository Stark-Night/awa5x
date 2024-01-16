#ifndef EVAL_H
#define EVAL_H

#include "abyss.h"

struct EvalResult {
     enum EvalCode {
          EVAL_OK,
          EVAL_ERROR,
          EVAL_NEW_STATE,
     } code;
     struct Abyss state;
};

struct EvalResult
eval_prn(struct Abyss abyss, int8_t parameter);

#endif

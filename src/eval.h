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

struct EvalResult
eval_pr1(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_red(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_r3d(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_blo(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_sbm(struct Abyss abyss, int8_t parameter);

#endif

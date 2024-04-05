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

struct EvalResult
eval_pop(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_dpl(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_srn(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_mrg(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_4dd(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_sub(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_mul(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_div(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_cnt(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_eql(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_lss(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_gr8(struct Abyss abyss, int8_t parameter);

struct EvalResult
eval_eqz(struct Abyss abyss, int8_t parameter);

#endif

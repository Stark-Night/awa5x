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

#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* An enum might appear more type safe, but due to comparisons with
   sized types (e.g. int8_t) things might fall through
   implementation-specific details and I don't want to keep track of
   that. */

#define NOP 0x00
#define PRN 0x01
#define PR1 0x02
#define RED 0x03
#define R3D 0x04
#define BLO 0x05
#define SBM 0x06
#define POP 0x07
#define DPL 0x08
#define SRN 0x09
#define MRG 0x0A
#define DD4 0x0B
#define SUB 0x0C
#define MUL 0x0D
#define DIV 0x0E
#define CNT 0x0F
#define LBL 0x10
#define JMP 0x11
#define EQL 0x12
#define LSS 0x13
#define GR8 0x14
#define EQZ 0x15

// extended jumps, using textual labels
#define TLB 0x16
#define JTL 0x17

// this is the last valid opcode
#define TRM 0x1F

// the valid alphabet (AwaSCII)
#define ALPHABET ("AWawJELYHOSIUMjelyhosiumPCNTpcntBDFGRbdfgr0123456789 .,!'()~_/;\n")

// length of the alphabet; it's always less than 128 so it can be safely casted to int8_t
#define NALNUM ((int8_t)(sizeof(ALPHABET)-1))

const char *
opcode_name(int8_t opcode);

int
opcode_has_parameter(int8_t opcode);

int
opcode_parameter_size(int8_t opcode);

#define opcode_error(opcode, parameter)                 \
     do {                                               \
          if (0 != opcode_has_parameter((opcode))) {    \
               fprintf(stderr,                          \
                       "%s %d\n",                       \
                       opcode_name((opcode)),           \
                       (parameter));                    \
          } else {                                      \
               fprintf(stderr,                          \
                       "%s\n",                          \
                       opcode_name((opcode)));          \
          }                                             \
     } while (0)

#endif

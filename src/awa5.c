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
#include <string.h>
#include <stdint.h>
#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
#include <winsock.h>
#else
#include <arpa/inet.h>
#endif

#include "filemap.h"
#include "opcodes.h"
#include "abyss.h"
#include "eval.h"

struct Header {
     int8_t magic[8];
     uint32_t code_size;
     uint32_t extra_flags;
     uint32_t cursor;
};

struct Program {
     int8_t *code;
     uint32_t counter;
     int8_t opcode;
     int8_t parameter;
     uint32_t extended_parameter;
     struct Abyss abyss;
     struct EvalResult result;
};

// this is defined globally because the large requested size can, in
// some cases, generate runtime errors.
#define ADDRESS_STACK_SIZE 1048576
static uint32_t address_stack[ADDRESS_STACK_SIZE] = { 0 };
static size_t address_stack_top  = 0;

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     struct FileMap input_map = file_map_open(argv[1]);
     if (FILE_MAP_OPEN != input_map.status) {
          fprintf(stderr, "unable to open %s\n", argv[1]);
          return 1;
     }

     // read the file header.
     struct Header file_header = { 0 };

     if (16 > input_map.size) {
          // 8 bytes magic
          // 4 bytes code segment size
          // 4 bytes extra flags
          fprintf(stderr, "file too small to be real (found %ld)\n", input_map.size);
          file_map_close(&input_map);
          return 1;
     }

     memcpy(file_header.magic, input_map.buffer + file_header.cursor, 8);
     file_header.cursor = file_header.cursor + 8;

     file_header.code_size = ntohl(((uint32_t *)(input_map.buffer + file_header.cursor))[0]);
     file_header.extra_flags = ntohl(((uint32_t *)(input_map.buffer + file_header.cursor))[1]);
     file_header.cursor = file_header.cursor + (2 * sizeof(uint32_t));

     // verify header integerity.
     int8_t check_magic[8] = { 0x00, 0x41, 0x57, 0x41, 0x35, 0x30, 0x0D, 0x0A };
     if (0 != memcmp(check_magic, file_header.magic, 8)) {
          fprintf(stderr, "malformed file\n");
          file_map_close(&input_map);
          return 1;
     }

     // verify the program size makes sense
     if (file_header.code_size > input_map.size - file_header.cursor) {
          fprintf(stderr, "malformed file\n");
          file_map_close(&input_map);
          return 1;
     }

     struct Program program = { 0 };
     program.code = input_map.buffer + file_header.cursor;

     while (program.counter < file_header.code_size) {
          program.opcode = ((uint8_t)program.code[program.counter]) % 32;

          if (0 != opcode_has_parameter(program.opcode)) {
               int parasize = opcode_parameter_size(program.opcode);

               program.counter = program.counter + 1;

               if (program.counter >= file_header.code_size) {
                    fprintf(stderr,
                            "%s: not enough arguments\n",
                            opcode_name(program.opcode));
                    continue;
               }

               if (1 == parasize) {
                    program.parameter = program.code[program.counter];
                    program.extended_parameter = 0;
               } else {
                    program.parameter = 0;

                    uint8_t bytes[4] = {
                         program.code[program.counter + 0],
                         program.code[program.counter + 1],
                         program.code[program.counter + 2],
                         program.code[program.counter + 3],
                    };

                    uint32_t sw = 0;
                    memcpy(&sw, bytes, 4);

                    program.extended_parameter = ntohl(sw);
               }
          }

#ifdef OPCODE_TRACING
          if (NULL != program.abyss.head) {
               fprintf(stderr,
                       "{abyss top}[%5d][%1d]     ",
                       program.abyss.head->value,
                       bubble_double(*(program.abyss.head)));
          } else {
               fprintf(stderr,
                       "{abyss top}[empty][0]     ");
          }

          if (0 != opcode_has_parameter(program.opcode)) {
               fprintf(stderr,
                       "[%s %d]\n",
                       opcode_name(program.opcode),
                       program.parameter);
          } else {
               fprintf(stderr,
                       "[%s]\n",
                       opcode_name(program.opcode));
          }
#endif

          switch (program.opcode) {
          case NOP:
               // do nothing
               break;
          case PRN:
               program.result = eval_prn(program.abyss, program.parameter);
               break;
          case PR1:
               program.result = eval_pr1(program.abyss, program.parameter);
               break;
          case RED:
               program.result = eval_red(program.abyss, program.parameter);
               break;
          case R3D:
               program.result = eval_r3d(program.abyss, program.parameter);
               break;
          case BLO:
               program.result = eval_blo(program.abyss, program.parameter);
               break;
          case SBM:
               program.result = eval_sbm(program.abyss, program.parameter);
               break;
          case POP:
               program.result = eval_pop(program.abyss, program.parameter);
               break;
          case DPL:
               program.result = eval_dpl(program.abyss, program.parameter);
               break;
          case SRN:
               program.result = eval_srn(program.abyss, program.parameter);
               break;
          case MRG:
               program.result = eval_mrg(program.abyss, program.parameter);
               break;
          case DD4:
               program.result = eval_4dd(program.abyss, program.parameter);
               break;
          case SUB:
               program.result = eval_sub(program.abyss, program.parameter);
               break;
          case MUL:
               program.result = eval_mul(program.abyss, program.parameter);
               break;
          case DIV:
               program.result = eval_div(program.abyss, program.parameter);
               break;
          case CNT:
               program.result = eval_cnt(program.abyss, program.parameter);
               break;
          case LBL:
               // this opcode should never be found since labels are
               // compiled in the code.
               // let's fail as hard as we can.
               abort();
               break;
          case JMP:
               program.result.code = EVAL_OK;
               if (UINT32_MAX == program.extended_parameter) {
                    program.result.code = EVAL_ERROR;
               } else {
                    // memo: subtract 1 because the counter is
                    // increased at the end of the loop
                    program.counter = program.extended_parameter - 1;
               }
               break;
          case EQL:
               program.result = eval_eql(program.abyss, program.parameter);
               if (EVAL_NO == program.result.code) {
                    // skip the next instruction
                    program.counter = program.counter + 1;
                    program.opcode = ((uint8_t)program.code[program.counter]) % 32;

                    if (0 != opcode_has_parameter(program.opcode)) {
                         int size = opcode_parameter_size(program.opcode);
                         program.counter = program.counter + size;
                    }
               }
               // reset the result to avoid exiting the program
               program.result.code = EVAL_OK;
               break;
          case LSS:
               program.result = eval_lss(program.abyss, program.parameter);
               if (EVAL_NO == program.result.code) {
                    // skip the next instruction
                    program.counter = program.counter + 1;
                    program.opcode = ((uint8_t)program.code[program.counter]) % 32;

                    if (0 != opcode_has_parameter(program.opcode)) {
                         int size = opcode_parameter_size(program.opcode);
                         program.counter = program.counter + size;
                    }
               }
               // reset the result to avoid exiting the program
               program.result.code = EVAL_OK;
               break;
          case GR8:
               program.result = eval_gr8(program.abyss, program.parameter);
               if (EVAL_NO == program.result.code) {
                    // skip the next instruction
                    program.counter = program.counter + 1;
                    program.opcode = ((uint8_t)program.code[program.counter]) % 32;

                    if (0 != opcode_has_parameter(program.opcode)) {
                         int size = opcode_parameter_size(program.opcode);
                         program.counter = program.counter + size;
                    }
               }
               // reset the result to avoid exiting the program
               program.result.code = EVAL_OK;
               break;
          case EQZ:
               program.result = eval_eqz(program.abyss, program.parameter);
               if (EVAL_NO == program.result.code) {
                    // skip the next instruction
                    program.counter = program.counter + 1;
                    program.opcode = ((uint8_t)program.code[program.counter]) % 32;

                    if (0 != opcode_has_parameter(program.opcode)) {
                         int size = opcode_parameter_size(program.opcode);
                         program.counter = program.counter + size;
                    }
               }
               // reset the result to avoid exiting the program
               program.result.code = EVAL_OK;
               break;
          case TLB:
               // this opcode should never be found since labels are
               // compiled in the code.
               // let's fail as hard as we can.
               abort();
               break;
          case JTL:
               program.result.code = EVAL_OK;
               if (UINT32_MAX == program.extended_parameter) {
                    program.result.code = EVAL_ERROR;
               } else {
                    // memo: subtract 1 because the counter is
                    // increased at the end of the loop
                    program.counter = program.extended_parameter - 1;
               }
               break;
          case CLL:
               program.result.code = EVAL_OK;
               if (UINT32_MAX == program.extended_parameter) {
                    program.result.code = EVAL_ERROR;
               } else if (ADDRESS_STACK_SIZE == address_stack_top) {
                    fprintf(stderr, "too much recursion\n");
                    abort();
               } else {
                    // add the size of the parameter too
                    uint32_t address = program.counter +
                         opcode_parameter_size(program.opcode);

                    address_stack[address_stack_top] = address;
                    address_stack_top = address_stack_top + 1;

                    // memo: subtract 1 because the counter is
                    // increased at the end of the loop
                    program.counter = program.extended_parameter - 1;
               }
               break;
          case RET:
               program.result.code = EVAL_OK;
               if (0 == address_stack_top) {
                    program.result.code = EVAL_ERROR;
               } else {
                    uint32_t address = address_stack[address_stack_top - 1];
                    address_stack_top = address_stack_top - 1;

                    // memo: subtract 1 because the counter is
                    // increased at the end of the loop
                    program.counter = address - 1;
               }
               break;
          case TRM:
               program.result.code = EVAL_OK;
               program.counter = file_header.code_size;
               break;
          default:
               opcode_error(program.opcode,
                            (1 == opcode_parameter_size(program.opcode)) ?
                            program.parameter :
                            program.extended_parameter);
               break;
          }

          if (EVAL_NEW_STATE == program.result.code) {
               program.abyss = program.result.state;
          }

          if (EVAL_ERROR == program.result.code) {
               opcode_error(program.opcode,
                            (1 == opcode_parameter_size(program.opcode)) ?
                            program.parameter :
                            program.extended_parameter);
               program.counter = file_header.code_size;
          }

          program.counter = program.counter + 1;
     }

     program.abyss = abyss_drop(program.abyss);
     file_map_close(&input_map);

     return 0;
}

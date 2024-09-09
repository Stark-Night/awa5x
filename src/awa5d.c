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
#include <signal.h>
#include <setjmp.h>

#include "filemap.h"
#include "opcodes.h"
#include "abyss.h"
#include "eval.h"
#include "aline.h"

struct FileMeta {
     struct FileMap file;
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

// opcode evaluation can send SIGABRT
static volatile int received_abort = 0;
static jmp_buf jump_buffer = { 0 };

// commands understood by the debugger
#define QUIT_COMMAND 0x20
#define RUN_COMMAND 0x21
#define STEP_COMMAND 0x22
#define BACKSTEP_COMMAND 0x23
#define FENCE_COMMAND 0x24
#define RESUME_COMMAND 0x25
#define REWIND_COMMAND 0x26
#define GAZE_COMMAND 0x27
#define DISMANTLE_COMMAND 0x28

static struct FileMeta
input_file_open(struct FileMeta state, const char *path) {
     if (FILE_MAP_OPEN == state.file.status) {
          file_map_close(&(state.file));
     }

     state.file = file_map_open(path);
     if (FILE_MAP_OPEN != state.file.status) {
          return state;
     }

     state.cursor = 0;
     state.code_size = 0;
     state.extra_flags = 0;

     // the part that follows is better explained in awa5.c

     if (16 > state.file.size) {
          fprintf(stderr, "file too small to be real (found %ld)\n", state.file.size);
          file_map_close(&(state.file));
          return state;
     }

     memcpy(state.magic, state.file.buffer + state.cursor, 8);
     state.cursor = state.cursor + 8;

     state.code_size = ntohl(((uint32_t *)(state.file.buffer + state.cursor))[0]);
     state.extra_flags = ntohl(((uint32_t *)(state.file.buffer + state.cursor))[1]);
     state.cursor = state.cursor + (2 * sizeof(uint32_t));

     int8_t check_magic[8] = { 0x00, 0x41, 0x57, 0x41, 0x35, 0x30, 0x0D, 0x0A };
     if (0 != memcmp(check_magic, state.magic, 8)) {
          fprintf(stderr, "malformed file\n");
          file_map_close(&(state.file));
          return state;
     }

     // verify the program size makes sense
     if (state.code_size > state.file.size - state.cursor) {
          fprintf(stderr, "malformed file\n");
          file_map_close(&(state.file));
          return state;
     }

     return state;
}

static void
abort_handler(int sig) {
     received_abort = 1;

#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
     signal(SIGABRT, &abort_handler);
#endif

     longjmp(jump_buffer, 1);
}

static int
register_abort_handler(void) {
#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
     signal(SIGABRT, &abort_handler);
#else
     struct sigaction action = { 0 };
     action.sa_handler = &abort_handler;
     sigemptyset(&(action.sa_mask));

     if (-1 == sigaction(SIGABRT, &action, NULL)) {
          fprintf(stderr, "sigaction failed\n");
          return 1;
     }
#endif

     return 0;
}

static uint32_t
find_aline_by_address(struct ALine aline, uint32_t address) {
     // it's slow, can be done better with a more appropriate data
     // structure, but it's a developer tool to catch bugs and I'd
     // rather have simplicity over being too smart.
     for (uint32_t i=0; i<aline.capacity; ++i) {
          if (address != aline.items[i].address) {
               continue;
          }

          return i;
     }

     // thrash the call because something doesn't add up
     abort();
}

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     struct FileMeta input_file = { 0 };
     input_file = input_file_open(input_file, argv[1]);
     if (FILE_MAP_OPEN != input_file.file.status) {
          fprintf(stderr, "unable to open %s\n", argv[1]);
          return 1;
     }

     struct Program program = { 0 };
     program.code = input_file.file.buffer + input_file.cursor;

     int keep_watching = 1;
     int keep_executing = 0;
     struct ALine file_contents = { 0 };

     file_contents = aline_start(file_contents);
     for (uint32_t i=0; i<input_file.code_size; ++i) {
          struct ALineItem item = { 0 };

          item.address = i;
          item.code = ((uint8_t)program.code[i]) % 32;

          if (0 != opcode_has_parameter(item.code)) {
               int parasize = opcode_parameter_size(item.code);

               i = i + 1;

               if (i >= input_file.code_size) {
                    fprintf(stderr,
                            "%s: not enough arguments\n",
                            opcode_name(item.code));
                    continue;
               }

               if (1 == parasize) {
                    item.parameter = program.code[i];
               } else {
                    item.parameter = 0;

                    uint8_t bytes[4] = {
                         program.code[i + 0],
                         program.code[i + 1],
                         program.code[i + 2],
                         program.code[i + 3],
                    };

                    uint32_t sw = 0;
                    memcpy(&sw, bytes, 4);

                    item.parameter = ntohl(sw);

                    i = i + 3;
               }
          }

          file_contents = aline_track(file_contents, item);
     }

     if (0 != register_abort_handler()) {
          keep_watching = 0;
     }

     while (0 != keep_watching) {
          if (0 == keep_executing) {
               int8_t command = EOF;
               uint32_t uparam = 0;
               int32_t sparam = 0;

               if (0 == fread(&command, 1, 1, stdin)) {
                    // trying to get rid of a spurious warning
                    command = EOF;
               }

               switch (command) {
               case EOF:
                    keep_watching = 0;
                    break;
               case QUIT_COMMAND:
                    keep_watching = 0;
                    break;
               case RUN_COMMAND:
                    keep_executing = 1;
                    break;
               case STEP_COMMAND:
                    if (program.counter < input_file.code_size) {
                         program.counter = program.counter + 1;
                    }
                    break;
               case BACKSTEP_COMMAND:
                    if (0 < program.counter) {
                         program.counter = program.counter - 1;
                    }
                    break;
               case FENCE_COMMAND:
                    if (0 == fread(&uparam, sizeof(uint32_t), 1, stdin)) {
                         // not sure how to handle
                         abort();
                    }
                    uparam = ntohl(uparam);
                    file_contents = aline_add_flags_at(file_contents,
                                                       uparam,
                                                       ALINE_FLAG_BREAK);
                    break;
               case RESUME_COMMAND:
                    file_contents = aline_add_flags_at(file_contents,
                                                       program.counter,
                                                       ALINE_FLAG_RESUME);
                    keep_executing = 1;
                    break;
               case REWIND_COMMAND:
                    program.counter = 0;
                    break;
               case GAZE_COMMAND:
                    abyss_visualize(program.abyss, stderr);
                    break;
               case DISMANTLE_COMMAND:
                    if (0 == fread(&uparam, sizeof(uint32_t), 1, stdin)) {
                         // not sure how to handle
                         abort();
                    }
                    uparam = ntohl(uparam);
                    file_contents =
                         aline_remove_flags_at(file_contents,
                                               uparam,
                                               ALINE_FLAG_BREAK|ALINE_FLAG_RESUME);
                    break;
               default:
                    break;
               }
          }

          int jumppoint = setjmp(jump_buffer);
          if (0 != jumppoint && 0 != received_abort) {
               received_abort = 0;

               fprintf(stderr,
                       "Signal at 0x%x\nop: 0x%x\npar: 0x%x\nepar: 0x%x\n",
                       program.counter,
                       program.opcode,
                       program.parameter,
                       program.extended_parameter);

               keep_executing = 0;
          }

          while (program.counter < file_contents.capacity && 0 != keep_executing) {
               struct ALineItem aline = file_contents.items[program.counter];

               program.opcode = aline.code;

               if (0 != opcode_has_parameter(program.opcode)) {
                    int parasize = opcode_parameter_size(program.opcode);

                    if (1 == parasize) {
                         program.parameter = aline.parameter;
                         program.extended_parameter = 0;
                    } else {
                         program.parameter = 0;
                         program.extended_parameter = aline.parameter;
                    }
               }

               if (0 != (ALINE_FLAG_BREAK & aline.flags)) {
                    if (0 == (ALINE_FLAG_RESUME & aline.flags)) {
                         fprintf(stderr, "Stop at 0x%x\n", aline.address);
                         keep_executing = 0;
                         continue;
                    }

                    file_contents = aline_remove_flags_at(file_contents,
                                                          program.counter,
                                                          ALINE_FLAG_RESUME);
               }

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
                    abort();
                    break;
               case JMP:
                    program.result.code = EVAL_OK;
                    if (UINT32_MAX == program.extended_parameter) {
                         program.result.code = EVAL_ERROR;
                    } else {
                         uint32_t where =
                              find_aline_by_address(file_contents,
                                                    program.extended_parameter);

                         // memo: subtract 1 because the counter is
                         // increased at the end of the loop
                         program.counter = where - 1;
                    }
                    break;
               case EQL:
                    program.result = eval_eql(program.abyss, program.parameter);
                    if (EVAL_NO == program.result.code) {
                         // skip the next instruction
                         program.counter = program.counter + 1;
                    }
                    // reset the result to avoid exiting the program
                    program.result.code = EVAL_OK;
                    break;
               case LSS:
                    program.result = eval_lss(program.abyss, program.parameter);
                    if (EVAL_NO == program.result.code) {
                         // skip the next instruction
                         program.counter = program.counter + 1;
                    }
                    // reset the result to avoid exiting the program
                    program.result.code = EVAL_OK;
                    break;
               case GR8:
                    program.result = eval_gr8(program.abyss, program.parameter);
                    if (EVAL_NO == program.result.code) {
                         // skip the next instruction
                         program.counter = program.counter + 1;
                    }
                    // reset the result to avoid exiting the program
                    program.result.code = EVAL_OK;
                    break;
               case EQZ:
                    program.result = eval_eqz(program.abyss, program.parameter);
                    if (EVAL_NO == program.result.code) {
                         // skip the next instruction
                         program.counter = program.counter + 1;
                    }
                    // reset the result to avoid exiting the program
                    program.result.code = EVAL_OK;
                    break;
               case TLB:
                    abort();
                    break;
               case JTL:
                    program.result.code = EVAL_OK;
                    if (UINT32_MAX == program.extended_parameter) {
                         program.result.code = EVAL_ERROR;
                    } else {
                         uint32_t where =
                              find_aline_by_address(file_contents,
                                                    program.extended_parameter);

                         // memo: subtract 1 because the counter is
                         // increased at the end of the loop
                         program.counter = where - 1;
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
                         uint32_t address = program.counter + 1;

                         address_stack[address_stack_top] = address;
                         address_stack_top = address_stack_top + 1;

                         uint32_t where =
                              find_aline_by_address(file_contents,
                                                    program.extended_parameter);
                         // memo: subtract 1 because the counter is
                         // increased at the end of the loop
                         program.counter = where - 1;
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
               case LDO:
                    program.result = eval_ldo(program.abyss, program.parameter);
                    break;
               case CDO:
                    program.result = eval_cdo(program.abyss, program.parameter);
                    break;
               case TRM:
                    program.result.code = EVAL_OK;
                    program.counter = file_contents.capacity;
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
                    abort();
               }

               program.counter = program.counter + 1;
          }

          keep_executing = 0;
          if (program.counter >= file_contents.capacity) {
               program.counter = 0;
          }
     }


     file_contents = aline_end(file_contents);
     program.abyss = abyss_drop(program.abyss);
     file_map_close(&(input_file.file));

     return 0;
}

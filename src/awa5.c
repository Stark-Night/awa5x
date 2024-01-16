#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "opcodes.h"
#include "abyss.h"
#include "eval.h"

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

struct Header {
     int8_t magic[8];
     uint32_t label_num[2];
     uint32_t labels[256];
     uint32_t code_size;
     uint32_t extra_flags;
     uint32_t cursor;
};

struct Program {
     int8_t *code;
     uint32_t counter;
     int8_t opcode;
     int8_t parameter;
     struct Abyss abyss;
     struct EvalResult result;
};

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     int input_fd = open(argv[1], O_RDONLY);
     if (-1 == input_fd) {
          fprintf(stderr, "no file %s\n", argv[1]);
          return 1;
     }

     struct stat input_info = { 0 };
     if (-1 == fstat(input_fd, &input_info)) {
          fprintf(stderr, "stat failed\n");
          close(input_fd);
          return 1;
     }

     if (24 > input_info.st_size) {
          // 8 bytes magic
          // 4 + 4 bytes labels
          // 4 bytes code segment size
          // 4 bytes extra flags
          fprintf(stderr, "malformed file\n");
          close(input_fd);
          return 1;
     }

     void *input_mmap =
          mmap(NULL, input_info.st_size, PROT_READ, MAP_PRIVATE, input_fd, 0);
     if (MAP_FAILED == input_mmap) {
          fprintf(stderr, "mmap failed\n");
          close(input_fd);
          return 1;
     }

     // read the file header.
     struct Header file_header = { 0 };

     memcpy(file_header.magic, input_mmap + file_header.cursor, 8);
     file_header.cursor = file_header.cursor + 8;

     memcpy(file_header.label_num, input_mmap + file_header.cursor, 2 * sizeof(uint32_t));
     file_header.label_num[0] = ntohl(file_header.label_num[0]);
     file_header.label_num[1] = ntohl(file_header.label_num[1]);
     file_header.cursor = file_header.cursor + (2 * sizeof(uint32_t));

     for (uint32_t i=0; i<file_header.label_num[0]; ++i) {
          uint32_t offset = ntohl(((uint32_t *)(input_mmap + file_header.cursor))[0]);
          uint32_t index = ntohl(((uint32_t *)(input_mmap + file_header.cursor))[1]);

          file_header.cursor = file_header.cursor + (2 * sizeof(uint32_t));

          file_header.labels[index] = offset;
     }

     file_header.code_size = ntohl(((uint32_t *)(input_mmap + file_header.cursor))[0]);
     file_header.extra_flags = ntohl(((uint32_t *)(input_mmap + file_header.cursor))[1]);
     file_header.cursor = file_header.cursor + (2 * sizeof(uint32_t));

     // verify header integerity.
     int8_t check_magic[8] = { 0x00, 0x41, 0x57, 0x41, 0x35, 0x30, 0x0D, 0x0A };
     if (0 != memcmp(check_magic, file_header.magic, 8)) {
          fprintf(stderr, "malformed file\n");
          munmap(input_mmap, input_info.st_size);
          close(input_fd);
          return 1;
     }

     if (file_header.label_num[0] != file_header.label_num[1]
          || 256 <= file_header.label_num[0]
          || 256 <= file_header.label_num[1]) {
          fprintf(stderr, "malformed file\n");
          munmap(input_mmap, input_info.st_size);
          close(input_fd);
          return 1;
     }

     for (int i=0; i<256; ++i) {
          if (file_header.labels[i] > file_header.code_size) {
               fprintf(stderr, "malformed file\n");
               munmap(input_mmap, input_info.st_size);
               close(input_fd);
               return 1;
          }
     }

     struct Program program = { 0 };
     program.code = input_mmap + file_header.cursor;

     while (program.counter < file_header.code_size) {
          program.opcode = ((uint8_t)program.code[program.counter]) % 32;

          if (0 != opcode_has_parameter(program.opcode)) {
               program.counter = program.counter + 1;

               if (program.counter >= file_header.code_size) {
                    fprintf(stderr,
                            "%s: not enough arguments\n",
                            opcode_name(program.opcode));
                    continue;
               }

               program.parameter = program.code[program.counter];
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
          case LBL:
               // this opcode should never be found since labels are
               // compiled in the header.
               // let's fail as hard as we can.
               abort();
               break;
          case TRM:
               program.result.code = EVAL_OK;
               program.counter = file_header.code_size;
               break;
          default:
               opcode_error(program.opcode, program.parameter);
               break;
          }

          if (EVAL_NEW_STATE == program.result.code) {
               program.abyss = program.result.state;
          }

          if (EVAL_ERROR == program.result.code) {
               opcode_error(program.opcode, program.parameter);
               program.counter = file_header.code_size;
          }

          program.counter = program.counter + 1;
     }

     program.abyss = abyss_drop(program.abyss);
     munmap(input_mmap, input_info.st_size);
     close(input_fd);

     return 0;
}

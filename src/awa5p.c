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
#include <regex.h>

#include "utf8.h"

struct GrowBuffer {
     int8_t *bytes;
     size_t size;
     size_t capacity;
};

#define OPCODE_PATTERN                                                  \
     "^[[:space:]]*#([[:alnum:]][[:alnum:]][[:alnum:]])([[:space:]]+([[:digit:]]*))?"

#define NOP_AWA "awa awa awa awa awa"
#define PRN_AWA "awa awa awa awawa"
#define PR1_AWA "awa awa awawa awa"
#define RED_AWA "awa awa awawawa"
#define R3D_AWA "awa awawa awa awa"
#define BLO_AWA "awa awawa awawa"
#define SBM_AWA "awa awawawa awa"
#define POP_AWA "awa awawawawa"
#define DPL_AWA "awawa awa awa awa"
#define SRN_AWA "awawa awa awawa"
#define MRG_AWA "awawa awawa awa"
#define DD4_AWA "awawa awawawa"
#define SUB_AWA "awawawa awa awa"
#define MUL_AWA "awawawa awawa"
#define DIV_AWA "awawawawa awa"
#define CNT_AWA "awawawawawa"
#define LBL_AWA "~wa awa awa awa awa"
#define JMP_AWA "~wa awa awa awawa"
#define EQL_AWA "~wa awa awawa awa"
#define LSS_AWA "~wa awawa awa awa"
#define GR8_AWA "~wa awawa awawa"
#define EQZ_AWA "~wa awawawa awa"
#define TRM_AWA "~wawawawawa"

int8_t opcode_bytes[][3] = {
     { 0x4E, 0x4F, 0x50 },
     { 0x50, 0x52, 0x4E },
     { 0x50, 0x52, 0x31 },
     { 0x52, 0x45, 0x44 },
     { 0x52, 0x33, 0x44 },
     { 0x42, 0X4C, 0x4F },
     { 0x53, 0x42, 0x4D },
     { 0x50, 0x4F, 0x50 },
     { 0x44, 0x50, 0x4C },
     { 0x53, 0x52, 0x4E },
     { 0x4D, 0x52, 0x47 },
     { 0x44, 0x44, 0x34 },
     { 0x53, 0x55, 0x42 },
     { 0x4D, 0x55, 0x4C },
     { 0x44, 0x49, 0x56 },
     { 0x43, 0x4E, 0x54 },
     { 0x4C, 0x42, 0x4C },
     { 0x4A, 0x4D, 0x50 },
     { 0x45, 0x51, 0x4C },
     { 0x4C, 0x53, 0x53 },
     { 0x47, 0x52, 0x38 },
     { 0x45, 0x51, 0x5A },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x00, 0x00, 0x00 },
     { 0x54, 0x52, 0x4D },
};

#define NOP_BYTES opcode_bytes[0]
#define PRN_BYTES opcode_bytes[1]
#define PR1_BYTES opcode_bytes[2]
#define RED_BYTES opcode_bytes[3]
#define R3D_BYTES opcode_bytes[4]
#define BLO_BYTES opcode_bytes[5]
#define SBM_BYTES opcode_bytes[6]
#define POP_BYTES opcode_bytes[7]
#define DPL_BYTES opcode_bytes[8]
#define SRN_BYTES opcode_bytes[9]
#define MRG_BYTES opcode_bytes[10]
#define DD4_BYTES opcode_bytes[11]
#define SUB_BYTES opcode_bytes[12]
#define MUL_BYTES opcode_bytes[13]
#define DIV_BYTES opcode_bytes[14]
#define CNT_BYTES opcode_bytes[15]
#define LBL_BYTES opcode_bytes[16]
#define JMP_BYTES opcode_bytes[17]
#define EQL_BYTES opcode_bytes[18]
#define LSS_BYTES opcode_bytes[19]
#define GR8_BYTES opcode_bytes[20]
#define EQZ_BYTES opcode_bytes[21]
#define TRM_BYTES opcode_bytes[31]

regex_t opcode_regex = { 0 };

static struct GrowBuffer *
grow_buffer(struct GrowBuffer *buffer, size_t requested) {
     if (NULL == buffer->bytes) {
          buffer->bytes = malloc(1024);
          buffer->size = 1024;
          buffer->capacity = 0;

          if (NULL == buffer->bytes) {
               abort();
          }
     }

     if (buffer->capacity + requested < buffer->size) {
          return buffer;
     }

     size_t newsize = buffer->size;
     while (buffer->capacity + requested > newsize) {
          newsize = newsize * 2;
     }

     if (newsize <= buffer->size) {
          abort();
     }

     buffer->bytes = realloc(buffer->bytes, newsize);
     buffer->size = newsize;

     if (NULL == buffer->bytes) {
          abort();
     }

     return buffer;
}

static struct GrowBuffer *
shrink_buffer(struct GrowBuffer *buffer) {
     free(buffer->bytes);

     buffer->bytes = NULL;
     buffer->size = 0;
     buffer->capacity = 0;

     return buffer;
}

static struct GrowBuffer *
append_buffer(struct GrowBuffer *buffer, void *data, size_t size) {
     grow_buffer(buffer, size);

     memcpy(buffer->bytes + buffer->capacity, data, size);
     buffer->capacity = buffer->capacity + size;

     return buffer;
}

static struct GrowBuffer *
reset_buffer(struct GrowBuffer *buffer) {
     buffer->capacity = 0;
     return buffer;
}

static int
output_opcode_awa(void *opcode, regoff_t size) {
     if (3 == size) {
          if (0 == memcmp(NOP_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", NOP_AWA);
          }

          if (0 == memcmp(PRN_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", PRN_AWA);
          }

          if (0 == memcmp(PR1_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", PR1_AWA);
          }

          if (0 == memcmp(RED_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", RED_AWA);
          }

          if (0 == memcmp(R3D_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", R3D_AWA);
          }

          if (0 == memcmp(BLO_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", BLO_AWA);
          }

          if (0 == memcmp(SBM_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", SBM_AWA);
          }

          if (0 == memcmp(POP_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", POP_AWA);
          }

          if (0 == memcmp(DPL_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", DPL_AWA);
          }

          if (0 == memcmp(SRN_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", SRN_AWA);
          }

          if (0 == memcmp(MRG_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", MRG_AWA);
          }

          if (0 == memcmp(DD4_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", DD4_AWA);
          }

          if (0 == memcmp(SUB_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", SUB_AWA);
          }

          if (0 == memcmp(MUL_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", MUL_AWA);
          }

          if (0 == memcmp(DIV_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", DIV_AWA);
          }

          if (0 == memcmp(CNT_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", CNT_AWA);
          }

          if (0 == memcmp(LBL_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", LBL_AWA);
          }

          if (0 == memcmp(JMP_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", JMP_AWA);
          }

          if (0 == memcmp(EQL_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", EQL_AWA);
          }

          if (0 == memcmp(LSS_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", LSS_AWA);
          }

          if (0 == memcmp(GR8_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", GR8_AWA);
          }

          if (0 == memcmp(EQZ_BYTES, opcode, size)) {
               return fprintf(stdout, "%s", EQZ_AWA);
          }
     }

     return 0;
}

static int
line_check(struct GrowBuffer *line) {
     regmatch_t grouped[8] = { 0 };

     int opcode_result = regexec(&opcode_regex, line->bytes, 8, grouped, 0);
     if (REG_NOMATCH == opcode_result) {
          // ignore the line
          return 0;
     }

     if (-1 == grouped[1].rm_so) {
          fprintf(stderr, "malformed input\n");
          return 1;
     }

     output_opcode_awa(line->bytes + grouped[1].rm_so,
                       grouped[1].rm_eo - grouped[1].rm_so);

     if (-1 != grouped[2].rm_so
         && -1 != grouped[3].rm_so && 0 < grouped[3].rm_eo - grouped[3].rm_so) {
          line->bytes[grouped[3].rm_eo] = '\0';

          // like in eval.c, strtol is not the best but it does its job I guess
          char *tail = NULL;
          long int cnum = strtol(line->bytes + grouped[3].rm_so, &tail, 10);

          if (NULL != tail && '\0' != tail[0] && (INT8_MIN > cnum || INT8_MAX < cnum)) {
               fprintf(stderr,
                       "invalid parameter; must be a number: %s\n",
                       line->bytes + grouped[3].rm_so);

               return 1;
          }

          uint8_t masks[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
          fprintf(stdout, " %s", (cnum & masks[0]) ? "~wa" : "awa");

          for (int i=1; i<8; ++i) {
               fprintf(stdout, "%s", (cnum & masks[i]) ? "wa" : " awa");
          }
     }

     fprintf(stdout, "\n");

     return 0;
}

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     int fd = open(argv[1], O_RDONLY);
     if (-1 == fd) {
          fprintf(stderr, "no file %s\n", argv[1]);
          return 1;
     }

     struct stat finfo = { 0 };
     if (-1 == fstat(fd, &finfo)) {
          fprintf(stderr, "stat failed\n");
          close(fd);
          return 1;
     }

     if (0 == finfo.st_size) {
          // empty file, nothing to do?
          close(fd);
          return 0;
     }

     void *fmap = mmap(NULL, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
     if (MAP_FAILED == fmap) {
          fprintf(stderr, "mmap failed\n");
          close(fd);
          return 1;
     }

     struct GrowBuffer buffer = { 0 };

     if (0 != regcomp(&opcode_regex, OPCODE_PATTERN, REG_EXTENDED)) {
          fprintf(stderr, "regex compilation failed\n");
          munmap(fmap, finfo.st_size);
          close(fd);
          return 1;
     }

     // print the first 'awa' necessary to make this a valid output
     fprintf(stdout, "awa\n");

     off_t cursor = 0;
     while (cursor < finfo.st_size) {
          struct UTF8Result decoded = utf8_decode(fmap + cursor);

          append_buffer(&buffer, &(decoded.point), decoded.bytes);

          if (1 == buffer.capacity && IS_N(decoded)) {
               reset_buffer(&buffer);
               cursor = cursor + decoded.bytes;
               continue;
          }

          if (IS_N(decoded)) {
               append_buffer(&buffer, "\0", 1);

               if (0 != line_check(&buffer)) {
                    cursor = finfo.st_size;
               }

               reset_buffer(&buffer);
          }

          cursor = cursor + decoded.bytes;
     }

     if (buffer.capacity > 0) {
          append_buffer(&buffer, "\0", 1);
          line_check(&buffer);
     }

     shrink_buffer(&buffer);
     regfree(&opcode_regex);

     munmap(fmap, finfo.st_size);
     close(fd);

     return 0;
}

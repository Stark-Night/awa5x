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

#include "filemap.h"
#include "utf8.h"
#include "grow.h"
#include "opcodes.h"
#include "strtoawa.h"

struct CurrentFile {
     struct FileMap map;
     off_t cursor;
};

struct MatchState {
     int opcode;
     int parameter;
     int include;
     int plain;
     int txtlbl;
};

#define NOT_MATCH(state)                                                 \
     ((0 == (state).opcode) &&                                          \
      (0 == (state).parameter) &&                                       \
      (0 == (state).include) &&                                         \
      (0 == (state).plain) &&                                           \
      (0 == (state).txtlbl))

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
#define LSS_AWA "~wa awa awawawa"
#define GR8_AWA "~wa awawa awa awa"
#define EQZ_AWA "~wa awawa awawa"
#define TLB_AWA "~wa awawawa awa"
#define JTL_AWA "~wa awawawawa"
#define CLL_AWA "~wawa awa awa awa"
#define RET_AWA "~wawa awa awawa"
#define TRM_AWA "~wawawawawa"

static int8_t opcode_bytes[][3] = {
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
     { 0x54, 0x4C, 0x42 },
     { 0x4A, 0x54, 0x4C },
     { 0x43, 0x4C, 0x4C },
     { 0x52, 0x45, 0x54 },
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
#define TLB_BYTES opcode_bytes[22]
#define JTL_BYTES opcode_bytes[23]
#define CLL_BYTES opcode_bytes[24]
#define RET_BYTES opcode_bytes[25]
#define TRM_BYTES opcode_bytes[31]

// this is defined globally because the large requested size can, in
// some cases, generate runtime errors.
static struct CurrentFile file_stack[1048576] = { 0 };
static size_t file_stack_top = 0;

static int
opcode_line_check(struct GrowBuffer *line) {
     void *opcode = line->bytes;

     if (3 == line->capacity - 1) {
          if (0 == memcmp(NOP_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", NOP_AWA);
          }

          if (0 == memcmp(PRN_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", PRN_AWA);
          }

          if (0 == memcmp(PR1_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", PR1_AWA);
          }

          if (0 == memcmp(RED_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", RED_AWA);
          }

          if (0 == memcmp(R3D_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", R3D_AWA);
          }

          if (0 == memcmp(BLO_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", BLO_AWA);
          }

          if (0 == memcmp(SBM_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", SBM_AWA);
          }

          if (0 == memcmp(POP_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", POP_AWA);
          }

          if (0 == memcmp(DPL_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", DPL_AWA);
          }

          if (0 == memcmp(SRN_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", SRN_AWA);
          }

          if (0 == memcmp(MRG_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", MRG_AWA);
          }

          if (0 == memcmp(DD4_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", DD4_AWA);
          }

          if (0 == memcmp(SUB_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", SUB_AWA);
          }

          if (0 == memcmp(MUL_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", MUL_AWA);
          }

          if (0 == memcmp(DIV_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", DIV_AWA);
          }

          if (0 == memcmp(CNT_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", CNT_AWA);
          }

          if (0 == memcmp(LBL_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", LBL_AWA);
          }

          if (0 == memcmp(JMP_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", JMP_AWA);
          }

          if (0 == memcmp(EQL_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", EQL_AWA);
          }

          if (0 == memcmp(LSS_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", LSS_AWA);
          }

          if (0 == memcmp(GR8_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", GR8_AWA);
          }

          if (0 == memcmp(EQZ_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", EQZ_AWA);
          }

          if (0 == memcmp(TLB_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", TLB_AWA);
          }

          if (0 == memcmp(JTL_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", JTL_AWA);
          }

          if (0 == memcmp(CLL_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", CLL_AWA);
          }

          if (0 == memcmp(RET_BYTES, opcode, 4)) {
               return fprintf(stdout, "%s", RET_AWA);
          }

          if (0 == memcmp(TRM_BYTES, opcode, 3)) {
               return fprintf(stdout, "%s", TRM_AWA);
          }
     }

     fprintf(stderr, "invalid opcode: {%s}", line->bytes);

     return 0;
}

static int
parameter_line_check(struct GrowBuffer *line) {
     if (0 == line->capacity || '\0' == line->bytes[0]) {
          fprintf(stdout, "\n");
          return 0;
     }

     // like in eval.c, strtol/strtoawa is not the best but it does its job I guess
     char *tail = NULL;
     long int cnum = strtoawa(line->bytes, &tail);

     if (NULL != tail && '\0' != tail[0] && (INT8_MIN > cnum || INT8_MAX < cnum)) {
          fprintf(stderr,
                  "invalid parameter; must be a number: %s\n",
                  line->bytes);
          fprintf(stdout, "\n");

          return 1;
     }

     uint8_t masks[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };
     fprintf(stdout, " %s", (cnum & masks[0]) ? "~wa" : "awa");

     for (int i=1; i<8; ++i) {
          fprintf(stdout, "%s", (cnum & masks[i]) ? "wa" : " awa");
     }

     fprintf(stdout, "\n");

     return 0;
}

static struct CurrentFile *
open_file(const char *name) {
     if (file_stack_top + 1 >= 1048576) {
          abort();
     }

     size_t index = file_stack_top;
     struct CurrentFile *file = file_stack + index;

     file->map = file_map_open(name);
     file->cursor = 0;

     file_stack_top = file_stack_top + 1;

     return file;
}

static struct CurrentFile *
close_file(struct CurrentFile *file) {
     struct CurrentFile *previous =
          &(file_stack[(file_stack_top < 2) ? 0 : (file_stack_top - 2)]);

     file_map_close(&(file->map));
     file->cursor = 0;

     if (file_stack_top > 0) {
          file_stack_top = file_stack_top - 1;
     }

     return (0 == file_stack_top) ? NULL : previous;
}

static struct CurrentFile *
include_line_check(struct GrowBuffer *line) {
     struct CurrentFile *file = open_file(line->bytes);
     return file;
}

static int
txtlbl_line_check(struct GrowBuffer *line) {
     if (0 == line->capacity || '\0' == line->bytes[0]) {
          fprintf(stdout, "\n");
          return 0;
     }

     uint8_t bytes[] = { 0x20, 0x5B, 0x20, 0x5D };
     uint8_t masks[8] = { 128, 64, 32, 16, 8, 4, 2, 1 };

     fwrite(bytes + 0, 1, 2, stdout);

     for (size_t i=0; i<line->capacity-1; ++i) {
          uint8_t letter = line->bytes[i];
          uint32_t character = INT8_MAX;

          // iterate over the alphabet to find the letter
          // there is probably a better way, but I can't think of it right now
          for (uint8_t i=0; i<NALNUM; ++i) {
               if (ALPHABET[i] != letter) {
                    continue;
               }

               character = i;
               break;
          }

          if (character > NALNUM) {
               fprintf(stderr,
                       "invalid parameter; must be within the alphabet: %c\n",
                       letter);
               fprintf(stdout, "\n");

               return 1;
          }

          fprintf(stdout, " %s", (character & masks[0]) ? "~wa" : "awa");

          for (int i=1; i<8; ++i) {
               fprintf(stdout, "%s", (character & masks[i]) ? "wa" : " awa");
          }
     }

     fwrite(bytes + 2, 1, 2, stdout);

     fprintf(stdout, "\n");

     return 0;
}

int
main(int argc, char *argv[]) {
     if (argc < 2) {
          fprintf(stderr, "%s %s\nUsage: %s FILE\n", argv[0], PACKAGE_VERSION, argv[0]);
          return 1;
     }

     struct CurrentFile *file = open_file(argv[1]);
     if (NULL == file) {
          return 1;
     }

     if (0 == file->map.size) {
          // empty file, nothing to do?
          close_file(file);
          return 0;
     }

     struct GrowBuffer buffer = { 0 };

     // print the first 'awa' necessary to make this a valid output
     fprintf(stdout, "awa\n");

     struct MatchState match_state = { 0 };

     while (NULL != file) {
          while (file->cursor < file->map.size) {
               struct UTF8Result decoded = utf8_decode(file->map.buffer + file->cursor);
               file->cursor = file->cursor + decoded.bytes;

               // start of opcode check
               if (IS_B(decoded)) {
                    // only if not already in a match state
                    if (NOT_MATCH(match_state)) {
                         // start opcode match
                         match_state.opcode = 1;

                         // go to next symbol
                         continue;
                    }
               }

               // start of include check
               if (IS_G(decoded)) {
                    // only if not already in a match state
                    if (NOT_MATCH(match_state)) {
                         // start include match
                         match_state.include = 1;

                         // go to next symbol
                         continue;
                    }
               }

               // start of text label check
               if (IS_OB(decoded)) {
                    // only if not already in a match state or as a parameter
                    if (NOT_MATCH(match_state) || 1 == match_state.parameter) {
                         match_state.txtlbl = 1;
                    }

                    // go to next symbol
                    continue;
               }

               // end of text label check
               if (IS_CB(decoded)) {
                    // only if we are matching a label
                    if (1 == match_state.txtlbl) {
                         // stop matching
                         match_state.txtlbl = 2;
                    }

                    // go to next symbol
                    continue;
               }

               // start of keeping the line as-is
               if (!IS_S(decoded)) {
                    // only if not already in a match state
                    if (NOT_MATCH(match_state)) {
                         // start plain text match
                         match_state.plain = 1;

                         // do not skip this symbol
                    }
               }

               // check opcode match
               if (0 != match_state.opcode) {
                    // add to line buffer if not space
                    if (!IS_S(decoded)) {
                         buffer = append_buffer(buffer, &decoded.point, decoded.bytes);
                    }

                    // try to match it
                    if (IS_S(decoded)) {
                         buffer = append_buffer(buffer, "\0", 1);
                         opcode_line_check(&buffer);

                         buffer = reset_buffer(buffer);
                         match_state.opcode = 0;
                    }

                    // if not end of line seek a parameter
                    if (IS_P(decoded)) {
                         match_state.parameter = 1;
                    }

                    // at end of line add a newline in the output too
                    if (IS_N(decoded)) {
                         fprintf(stdout, "\n");
                    }

                    // go to next symbol
                    continue;
               }

               // check text label match
               // must go before parameter since it can clash
               if (0 != match_state.txtlbl) {
                    // add to line buffer if not space
                    if (!IS_S(decoded)) {
                         buffer = append_buffer(buffer, &decoded.point, decoded.bytes);
                    }

                    // try to match it
                    if (2 == match_state.txtlbl) {
                         buffer = append_buffer(buffer, "\0", 1);
                         txtlbl_line_check(&buffer);

                         buffer = reset_buffer(buffer);
                         match_state.txtlbl = 0;

                         if (0 != match_state.parameter) {
                              // reset parameter state if expected to
                              match_state.parameter = 0;
                         }
                    }

                    // go to next symbol
                    continue;
               }

               // check parameter match
               if (0 != match_state.parameter) {
                    // add to line buffer if not space
                    if (!IS_S(decoded)) {
                         buffer = append_buffer(buffer, &decoded.point, decoded.bytes);
                    }

                    // try to match it
                    if (IS_S(decoded)) {
                         buffer = append_buffer(buffer, "\0", 1);
                         parameter_line_check(&buffer);

                         buffer = reset_buffer(buffer);
                         match_state.parameter = 0;
                    }

                    // go to next symbol
                    continue;
               }

               // check include match
               if (0 != match_state.include) {
                    // add to line buffer if not space
                    if (!IS_N(decoded)) {
                         buffer = append_buffer(buffer, &decoded.point, decoded.bytes);
                    }

                    // try to match it at end of line
                    if (IS_N(decoded)) {
                         buffer = append_buffer(buffer, "\0", 1);

                         struct CurrentFile *newfile = include_line_check(&buffer);
                         buffer = reset_buffer(buffer);

                         if (NULL != newfile) {
                              file = newfile;
                         }

                         match_state.include = 0;
                    }

                    // go to next symbol
                    continue;
               }

               // check plain match
               if (0 != match_state.plain) {
                    // always add to line buffer
                    buffer = append_buffer(buffer, decoded.codes, decoded.bytes);

                    // at end of line output it
                    if (IS_N(decoded)) {
                         buffer = append_buffer(buffer, "\0", 1);

                         fwrite(buffer.bytes, buffer.capacity - 1, 1, stdout);

                         buffer = reset_buffer(buffer);
                         match_state.plain = 0;
                    }

                    // go to next symbol
                    continue;
               }
          }

          file = close_file(file);

          // do the same as above, but at the end of file
          if (buffer.capacity > 0) {
               buffer = append_buffer(buffer, "\0", 1);

               if (0 != match_state.opcode) {
                    opcode_line_check(&buffer);
               } else if (0 != match_state.parameter) {
                    parameter_line_check(&buffer);
               } else if (0 != match_state.include) {
                    struct CurrentFile *newfile = include_line_check(&buffer);

                    if (NULL != newfile) {
                         file = newfile;
                    }
               }

               buffer = reset_buffer(buffer);
          }

          match_state.opcode = 0;
          match_state.parameter = 0;
          match_state.include = 0;
     }

     buffer = shrink_buffer(buffer);

     return 0;
}

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

     return state;
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

     program.abyss = abyss_drop(program.abyss);
     file_map_close(&(input_file.file));

     return 0;
}

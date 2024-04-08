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

#ifndef FILEMAP_H
#define FILEMAP_H

#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#endif

struct FileMap {
     void *buffer;
     off_t size;
     enum {
          FILE_MAP_INVALID,
          FILE_MAP_OPEN,
          FILE_MAP_CLOSE,
     } status;

#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
     HANDLE descriptor;
     HANDLE object;
#else
     int descriptor;
#endif
};

struct FileMap
file_map_open(const char *filename);

int
file_map_close(struct FileMap *map);

#endif

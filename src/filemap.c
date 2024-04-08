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
#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#include "filemap.h"

#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
static int
map_file_windows(const char *filename, struct FileMap *map) {
     map->descriptor = CreateFile(filename,
                                  GENERIC_READ,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  0);
     if (INVALID_HANDLE_VALUE == map->descriptor) {
          fprintf(stderr, "no file %s\n", filename);
          return 1;
     }

     LARGE_INTEGER size = { 0 };
     if (!GetFileSizeEx(map->descriptor, &size)) {
          fprintf(stderr, "no file size\n");
          CloseHandle(map->descriptor);
          return 1;
     }

     if (0 == size.QuadPart) {
          fprintf(stderr, "empty file\n");
          CloseHandle(map->descriptor);
          return 1;
     }

     map->size = size.QuadPart;

     map->object = CreateFileMapping(map->descriptor,
                                     NULL,
                                     PAGE_READONLY,
                                     0,
                                     0,
                                     NULL);
     if (INVALID_HANDLE_VALUE == map->object) {
          fprintf(stderr, "no map object\n");
          CloseHandle(map->descriptor);
          return 1;
     }

     map->buffer = MapViewOfFile(map->object,
                                 FILE_MAP_READ,
                                 0,
                                 0,
                                 0);
     if (NULL == map->buffer) {
          fprintf(stderr, "unavailable file view\n");
          CloseHandle(map->object);
          CloseHandle(map->descriptor);
          return 1;
     }

     return 0;
}

static int
unmap_file_windows(struct FileMap *map) {
     UnmapViewOfFile(map->buffer);
     CloseHandle(map->object);
     CloseHandle(map->descriptor);

     return 0;
}
#else
static int
map_file_unix(const char *filename, struct FileMap *map) {
     int fd = open(filename, O_RDONLY);
     if (-1 == fd) {
          fprintf(stderr, "no file %s\n", filename);
          return 1;
     }

     struct stat finfo = { 0 };
     if (-1 == fstat(fd, &finfo)) {
          fprintf(stderr, "stat failed\n");
          close(fd);
          return 1;
     }

     if (3 > finfo.st_size) {
          fprintf(stderr, "unrealistic file size %ld\n", (long int)finfo.st_size);
          close(fd);
          return 1;
     }

     void *fmap = mmap(NULL, finfo.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
     if (MAP_FAILED == fmap) {
          fprintf(stderr, "mmap failed\n");
          close(fd);
          return 1;
     }

     map->size = finfo.st_size;
     map->descriptor = fd;
     map->buffer = fmap;

     return 0;
}

static int
unmap_file_unix(struct FileMap *map) {
     munmap(map->buffer, map->size);
     close(map->descriptor);

     return 0;
}
#endif

struct FileMap
file_map_open(const char *filename) {
     struct FileMap value = { 0 };

#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
     int result = map_file_windows(filename, &value);
#else
     int result = map_file_unix(filename, &value);
#endif

     if (0 != result) {
          value.status = FILE_MAP_INVALID;
          return value;
     }

     value.status = FILE_MAP_OPEN;

     return value;
}

int
file_map_close(struct FileMap *map) {
     if (FILE_MAP_OPEN != map->status) {
          return 0;
     }

#if defined(_WIN64) || defined(_WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
     int result = unmap_file_windows(map);
#else
     int result = unmap_file_unix(map);
#endif

     if (0 == result) {
          map->status = FILE_MAP_CLOSE;
     }

     return result;
}

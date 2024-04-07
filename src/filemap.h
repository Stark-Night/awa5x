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

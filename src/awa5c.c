#include "config.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

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

     munmap(fmap, finfo.st_size);
     close(fd);

     return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int fd;
  char *file;
  uint32_t bl, bh, al, ah;
  uint64_t before, after;
  struct stat sb;
  char a;

  fd = open("FILE", 0);

  if (fd == -1) {
    perror("Openning FILE");
    return 1;
  }

  if(fstat(fd, &sb)) {
    perror("Fstat FILE");
    return 1;
  }
  // printf("Size: %lu\n", (uint64_t)sb.st_size);

  file = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (file == MAP_FAILED) {
    perror("Mmap FILE");
    return 1;
  }

  // Perform the first I/O !
  a = file[0];

  while (1) {
    // printf("Flushes the caches\n");
    __asm__ __volatile__("clflush (%0)" : : "r"(&file[0]));
    // printf("Waiting %u seconds\n", WAIT_TIME);
    // usleep(WAIT_TIME);
    int i;
    for (i = 0; i < 1000; i++) {
      i++, i--;
    }
    // printf("Waking up, let's go!\n");
    __asm__ __volatile__("mfence");
    __asm__ __volatile__("lfence");
    __asm__ __volatile__("rdtscp" : "=a" (bl), "=d"(bh));
    a = file[0], a++;
    __asm__ __volatile__("rdtscp" : "=a" (al), "=d"(ah));
    // printf("Read done!\n");
    before = (((uint64_t)bh) << 32) | bl;
    after = (((uint64_t)ah) << 32) | al;
    // printf("Before %lu, after %lu\n", after, before);
    // printf("Reading time %lu for character %c\n", after - before, a);
    printf("%lu\n", after - before);
    fflush(stdout);
  }

  return 0;
}

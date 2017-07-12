#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

int probe(char **set, int ss, char *candidate);
void randomize_lines(char **ls, int s);
void minus(char **s1, int ss1, char **s2, int ss2, char **so, int *sso);
void print_line(char *l, int cr);
void fill_buf_list(char ***buf_list, char **set, int s);
void print_buf_list(char **buf_list, int s);

#define L3_PD_OFFSET_WIDTH 21
#define MAP_HUGE_2MB (L3_PD_OFFSET_WIDTH << MAP_HUGE_SHIFT)

/**
 * See
 * /sys/devices/system/cpu/cpu0/cache/index3/
 */

#define L3_LINE_WIDTH 6 // 64 octets
#define L3_SETS_WIDTH 12 // 4096 sets
#define L3_ASSOC 16 // 16 lines per set
#define L3_RTAG_WIDTH (L3_PD_OFFSET_WIDTH - L3_SETS_WIDTH - L3_LINE_WIDTH)
#define L3_CACHE_SIZE (((1 << L3_LINE_WIDTH) << L3_SETS_WIDTH) \
    * L3_ASSOC) // 4 MB total size
#define L3_2MB_TAG_NB (1 << \
    (L3_PD_OFFSET_WIDTH - L3_LINE_WIDTH - L3_SETS_WIDTH))
#define L3_SLICE_WIDTH 2 // Number of L3 cache slices
#define L3_TOTAL_SET_LINES (L3_ASSOC << L3_SLICE_WIDTH)

/**
 * Algorithm parameters
 */

#define L3_FACTOR_WIDTH 3 // times slices number times associativity
#define L3_LINES_NB ((L3_ASSOC << L3_SLICE_WIDTH) << L3_FACTOR_WIDTH)
#define L3_BUF_SIZE ((L3_LINES_NB / L3_2MB_TAG_NB) << 21)
#define L3_TARGET_SET (0x65e & ((1 << L3_SETS_WIDTH) - 1))
#define L3_CACHE_MISS_THRESHOLD 100
#define L3_PROBE_PASSES 0x10


/**
 * To activate huge pages
 * # echo 512 > /proc/sys/vm/nr_hugepages
 */

int main(int argc, char *argv[]) {
  char *buf;
  char **buf_list;
  int p, t, l, lc, i;
  uintptr_t pa, ta;
  char *lines[L3_LINES_NB];
  char *cset[L3_LINES_NB];
  char *eset[L3_LINES_NB];
  char *tset[L3_LINES_NB];
  char *mset[L3_LINES_NB];
  int css = 0, ess = 0, tss = 0, mss = 0;
  time_t now;

  printf("Processor specification:\n"
      "  Line width 0x%x\n"
      "  Sets 0x%x\n"
      "  Associativity 0x%x\n"
      "  Total L3 cache size 0x%x\n",
      L3_LINE_WIDTH, 1 << L3_SETS_WIDTH, L3_ASSOC, L3_CACHE_SIZE);

  printf("Algorithm parameters:\n"
      "  Associativity factor 0x%x\n"
      "  Searching eviction set in 0x%x lines\n"
      "  Target set 0x%x of 0x%x\n",
      1 << L3_FACTOR_WIDTH, L3_LINES_NB, L3_TARGET_SET, 1 << L3_SETS_WIDTH);

  // Allocation of buffer
  buf = mmap(NULL, L3_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE |
      MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

  if (buf == MAP_FAILED) {
    perror("Mmap buf");
    return 1;
  }

  printf("Allocated pages @0x%016lx of size 0x%08x\n", (uintptr_t)buf,
      L3_BUF_SIZE);

  /**
   * First creating the cache lines
   */

  // Iterate over the 2 MB pages
  for (p = 0; p < (L3_BUF_SIZE >> L3_PD_OFFSET_WIDTH); p++) {
    pa = (uintptr_t)(buf + (p << L3_PD_OFFSET_WIDTH));
   printf("Page %d, @0x%016lx\n", p, pa);
    // Iterating over available tags in the same 2MB page
    for (t = 0; t < L3_2MB_TAG_NB; t++) {
      ta = pa | (t << (L3_LINE_WIDTH + L3_SETS_WIDTH));
     printf("Tag (Lines) %d, @0x%016lx\n", t, ta);
      // We add the line for the targeted set
      lines[p * L3_2MB_TAG_NB + t] =
          (char *)(ta | (L3_TARGET_SET << L3_LINE_WIDTH));
    }
  }

  /**
   * Displays the generated lines
   */
  for (l = 0; l < L3_LINES_NB; l++) {
    printf("Line %d @0x%016lx\n", l, (uintptr_t)lines[l]);
  }

  /**
   * Creating the eviction sets
   */

  // Set random number generator seed
  now = time(NULL);
  srand48(now);

//  randomize_lines(&lines[0], L3_LINES_NB);

  // XXX test buf list
//  fill_buf_list(&buf_list, &lines[0], L3_LINES_NB);
//  print_buf_list(buf_list, L3_LINES_NB);

  // Step 1 : conflict set
  for (l = 0; l < L3_LINES_NB; l++) {
    printf("Line #0x%x\n", l);
    fill_buf_list(&buf_list, &cset[0], css);
    // print_buf_list(buf_list, css);
    if (!probe(buf_list, css, lines[l])) {
      printf("Add : non conflicting\n");
      cset[css] = lines[l];
      css++;
    } else {
      printf("Leave : conflicting\n");
    }
    if (css == L3_TOTAL_SET_LINES) {
      printf("NOTE : conflict set has reached required size : 0x%x\n", css);
//      break;
    }
  }

  /**
   * Displays the conflict set
   */
  printf("Size of conflict set 0x%x\n", css);
  for (l = 0; l < css; l++) {
//    printf("Line @0x%016lx\n", (uintptr_t)cset[l]);
  }
  return 0;

  /**
   * Compute lines = lines - conflict set
   */
  minus(lines, L3_LINES_NB, cset, css, tset, &tss);
  memcpy(&mset[0], &tset[0], sizeof(char *) * tss);
  mss = tss;
  printf("Size of lines - conflict set 0x%x\n", mss);
  for (l = 0; l < mss; l++) {
    printf("Line @0x%016lx\n", (uintptr_t)mset[l]);
  }

  /**
   * Step 2 : creating eviction sets
   */
  for (l = 0; l < mss; l++) {
    fill_buf_list(&buf_list, &cset[0], css);
    if (probe(buf_list, css, mset[l])) {
      ess = 0;
      for (lc = 0; lc < css; lc++) {
        // Creating cset - {lc}
        tss = 0;
        for (i = 0; i < css; i++) {
          if (cset[i] != cset[lc]) {
            tset[tss] = cset[i];
            tss++;
          }
        }
        fill_buf_list(&buf_list, &tset[0], tss);
        if (!probe(buf_list, tss, mset[l])) {
          eset[ess] = cset[lc];
          ess++;
        }
      }
      // Print eviction set
      if (ess > 0) {
        printf("New eviction set of size 0x%x:\n", ess);
        for (lc = 0; lc < ess; lc++) {
          printf("  Line @0x%016lx\n", (uintptr_t)eset[lc]);
        }
        // Remove eviction set from conflict set
        minus(cset, css, eset, ess, tset, &tss);
        memcpy(&cset[0], &tset[0], sizeof(char *) * tss);
        css = tss;
      } else {
        printf("Eviction set is null\n");
      }
    }
  }

  return 0;
}

/**
 * TODO Write it in assembly
 *
 * If conflict ? i.e. cache miss for candidate
 *
 */
int probe(char **set, int ss, char *candidate) {
  uint64_t time;
  char **llp;
  int count = 0, at = 0;
  int i;
  uintptr_t rip;
  if (ss > 0) {
    print_line(candidate, 0);
    printf(", ss 0x%x\n", ss);
  } else {
    return 0;
  }
  for (i = 0; i < L3_PROBE_PASSES; i++) {
    if (ss > 0) {
      __asm__ __volatile__("call asm_probe"
          : "=a"(time), "=c"(llp), "=d"(rip) : "a"(set), "b"(ss), "c"(candidate));
      printf("Candidate access time %lu ", time);
    if (time > L3_CACHE_MISS_THRESHOLD) {
        printf("M ");
    } else {
        printf("H ");
      count++;
    }
    at += time;
      printf("\n");
      //    return time > L3_CACHE_MISS_THRESHOLD;
    }
  }
  printf("hit count 0x%x, at %u average time, rip @0x%018lx\n", count, at /
      L3_PROBE_PASSES, rip);
  printf("Last line pointer accessed @0x%016lx of list @0x%016lx\n",
      (uintptr_t)llp, (uintptr_t)set);
  return count == 0;
  // return count > (L3_PROBE_PASSES >> 1);
}

void randomize_lines(char **ls, int s) {
  int i;
  long int r;
  char *l;
  // Shuffle array
  for (i = s - 1; i > 0; i--) {
    r = lrand48() % (i + 1);
    // printf("RANDOM %lx\n", r);
    l = ls[i];
    ls[i] = ls[r];
    ls[r] = l;
  }
}

int in(char **s, int ss, char *e) {
  int i;
  for (i = 0; i < ss; i++) {
    if (s[i] == e) {
      return 1;
    }
  }
  return 0;
}

void minus(char **s1, int ss1, char **s2, int ss2, char **so, int *sso) {
  int i;
  *sso = 0;
  for (i = 0; i < ss1; i++) {
    // Search s1[i] in s2, if not, then add it to so
    if (!in(s2, ss2, s1[i])) {
      so[*sso] = s1[i];
      (*sso)++;
    }
  }
}

void print_line(char *l, int cr) {
  printf("Line @0x%016lx : rtag 0x%lx, set 0x%lx, line 0x%lx", (uintptr_t)l,
      (((uintptr_t)l) >> (L3_SETS_WIDTH + L3_LINE_WIDTH)) &
          ((1 << L3_RTAG_WIDTH) - 1),
      (((uintptr_t)l) >> L3_LINE_WIDTH) & ((1 << L3_SETS_WIDTH) - 1),
      ((uintptr_t)l) & ((1 << L3_LINE_WIDTH) - 1));
  if (cr) {
    printf("\n");
  }
}

void fill_buf_list(char ***buf_list, char **set, int s) {
  int i;
  char **tbf;
  if (s > 0 ){
    *buf_list = (char **)set[0];
//    printf("Buf list initial pointer @0x%016lx\n", (uintptr_t)*buf_list);
    tbf = (char **)*buf_list;
//    printf("Buf list first pointer value @0x%016lx\n", (uintptr_t)tbf);
    for (i = 1; i < s; i++) {
      *tbf = (char *)set[i];
//      printf("Set @0x%016lx to @0x%016lx\n", (uintptr_t)tbf,
//          (uintptr_t)set[i]);
      tbf = (char **)*tbf;
    }
    // We cycle the end !
    *tbf = (char *)set[s - 1];
  }
}

void print_buf_list(char **buf_list, int s) {
  int i;
  printf("Print buf list of size 0x%x\n", s);
  for (i = 0; i < s; i++) {
    print_line((char *)buf_list, 1);
    buf_list = (char **)*buf_list;
  }
}

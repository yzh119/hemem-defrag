/*
 * =====================================================================================
 *
 *       Filename:  gups.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  02/21/2018 02:36:27 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "../src/timer.h"
#include "../src/hemem.h"


#include "gups.h"

#define MAX_THREADS     64

#define PAGE_NUM            3
#define PAGES               2048
#define GUPS_PAGE_SIZE      (4 * 1024)
#define HUGE_PAGE_SIZE (2L * 1024L * 1024)

#ifdef HOTSPOT
extern uint64_t hotset_start;
extern double hotset_fraction;
#endif

int threads;

uint64_t hot_start = 0;
uint64_t hotsize = 0;
bool move_hotset = false;

struct gups_args {
  int tid;                      // thread id
  uint64_t *indices;       // array of indices to access
  void* field;                  // pointer to start of thread's region
  uint64_t iters;          // iterations to perform
  uint64_t size;           // size of region
  uint64_t elt_size;       // size of elements
  uint64_t hot_start;            // start of hot set
  uint64_t hotsize;        // size of hot set
  uint64_t sparse_frac;    // Sparsity index
};


static inline uint64_t rdtscp(void)
{
    uint32_t eax, edx;
    // why is "ecx" in clobber list here, anyway? -SG&MH,2017-10-05
    __asm volatile ("rdtscp" : "=a" (eax), "=d" (edx) :: "ecx", "memory");
    return ((uint64_t)edx << 32) | eax;
}

uint64_t thread_gups[MAX_THREADS];

static unsigned long updates, nelems;

uint64_t tot_updates = 0;

static void *print_instantaneous_gups(void *arg)
{
  char *log_filename = (char *)(arg);
  FILE *tot;
  uint64_t tot_gups, tot_last_second_gups = 0;
  fprintf(stderr, "Opening inst gups at %s\n", log_filename);
  tot = fopen(log_filename, "w");
  if (tot == NULL) {
    perror("fopen");
  }

  for (;;) {
    tot_gups = 0;
    for (int i = 0; i < threads; i++) {
      tot_gups += thread_gups[i];
    }
    fprintf(tot, "%.10f\n", (1.0 * (abs(tot_gups - tot_last_second_gups))) / (1.0e9));
    fflush(tot);
    tot_updates += abs(tot_gups - tot_last_second_gups);
    tot_last_second_gups = tot_gups;
    sleep(1);
  }

  return NULL;
}

static uint64_t lfsr_fast(uint64_t lfsr)
{
  lfsr ^= lfsr >> 7;
  lfsr ^= lfsr << 9;
  lfsr ^= lfsr >> 13;
  return lfsr;
}

char *filename = "indices1.txt";

FILE *hotsetfile = NULL;

bool hotset_only = true;

static void *prefill_hotset(void* arguments)
{
  struct gups_args *args = (struct gups_args*)arguments;
  uint64_t *field = (uint64_t*)(args->field);
  uint64_t location;
  uint64_t index1;
  uint64_t elt_size = args->elt_size;
  char data[elt_size];
  uint64_t sparse_frac = args->sparse_frac;

  index1 = 0;
  uint64_t shift = 9;
  uint64_t shift2 = shift + sparse_frac;
  for(int iters = 0; iters < 100000; ++iters) {
  	  uint64_t page;
	  for (page = 0; page < args->hotsize / (GUPS_PAGE_SIZE / elt_size); page++) {
		  index1 = args->hot_start + (page << shift2);
      	  //assert(index1 < args->size);
		  //fprintf(stderr, "Index: %lld for %d\n", index1, location);
			if (elt_size == 8) {
				uint64_t  tmp = field[index1];
				tmp = tmp + page;
				field[index1] = tmp;
			}
			else {
				memcpy(data, &field[index1 * elt_size], elt_size);
				memset(data, data[0] + page, elt_size);
				memcpy(&field[index1 * elt_size], data, elt_size);
			}
	  }
  }
  return 0;
}

static void *do_gups_dense(void *arguments)
{
  //printf("do_gups entered\n");
  struct gups_args *args = (struct gups_args*)arguments;
  uint64_t *field = (uint64_t*)(args->field);
  uint64_t i;
  uint64_t index1, index2;
  uint64_t elt_size = args->elt_size;
  char data[elt_size];
  uint64_t lfsr;
  uint64_t hot_num;
  uint64_t sparse_frac = args->sparse_frac;

  srand(args->tid);
  lfsr = rand();

  index1 = 0;
  index2 = 0;

  fprintf(hotsetfile, "Thread %d region: %p - %p\thot set: %p - %p\n", args->tid, field, field + (args->size * elt_size), field + args->hot_start, field + args->hot_start + (args->hotsize * elt_size));   

  // Page: 12 bits, ELT_Size = 8 -> 3 bits. 12 - 3 = 9 bit shift
  uint64_t shift = 9;
  uint64_t shift2 = shift + sparse_frac;
  for (i = 0; i < args->iters; i++) {
    // dense hot set
    hot_num = lfsr_fast(lfsr) % 100;
    if (hot_num < 90) {
      lfsr = lfsr_fast(lfsr);
      index1 = args->hot_start + (lfsr % args->hotsize);
      if (elt_size == 8) {
        uint64_t  tmp = field[index1];
        tmp = tmp + i;
        field[index1] = tmp;
      }
      else {
        memcpy(data, &field[index1 * elt_size], elt_size);
        memset(data, data[0] + i, elt_size);
        memcpy(&field[index1 * elt_size], data, elt_size);
      }
    } else {
      lfsr = lfsr_fast(lfsr);
      index2 = lfsr % (args->size);
      if (elt_size == 8) {
        uint64_t tmp = field[index2];
        tmp = tmp + i;
        field[index2] = tmp;
      }
      else {
        memcpy(data, &field[index2 * elt_size], elt_size);
        memset(data, data[0] + i, elt_size);
        memcpy(&field[index2 * elt_size], data, elt_size);
      }
    }

    if (i % 1000 == 0) {
      thread_gups[args->tid] += 1000;
    }
  }

  return 0;
}

static void *do_gups_sparse(void *arguments)
{
  //printf("do_gups entered\n");
  struct gups_args *args = (struct gups_args*)arguments;
  uint64_t *field = (uint64_t*)(args->field);
  uint64_t i;
  uint64_t index1, index2;
  uint64_t elt_size = args->elt_size;
  char data[elt_size];
  uint64_t lfsr;
  uint64_t hot_num;
  uint64_t sparse_frac = args->sparse_frac;

  srand(args->tid);
  lfsr = rand();

  index1 = 0;
  index2 = 0;

  fprintf(hotsetfile, "Thread %d region: %p - %p\thot set: %p - %p\n", args->tid, field, field + (args->size * elt_size), field + args->hot_start, field + args->hot_start + (args->hotsize * elt_size));   

  // Page: 12 bits, ELT_Size = 8 -> 3 bits. 12 - 3 = 9 bit shift
  uint64_t shift = 9;
  uint64_t shift2 = shift + sparse_frac;
  for (i = 0; i < args->iters; i++) {
    // sparse hot set
    hot_num = lfsr_fast(lfsr) % 100;
    if (hot_num < 90) {
      lfsr = lfsr_fast(lfsr);
      uint64_t location = (lfsr % args->hotsize);
      uint64_t page = location >> shift;
      uint64_t offset = location & ((1 << shift) - 1);
      index1 = args->hot_start + (page << shift2) + offset;
      if (elt_size == 8) {
        uint64_t  tmp = field[index1];
        tmp = tmp + i;
        field[index1] = tmp;
      }
      else {
        memcpy(data, &field[index1 * elt_size], elt_size);
        memset(data, data[0] + i, elt_size);
        memcpy(&field[index1 * elt_size], data, elt_size);
      }
    }
    else {
      lfsr = lfsr_fast(lfsr);
      index2 = lfsr % (args->size);
      if (elt_size == 8) {
        uint64_t tmp = field[index2];
        tmp = tmp + i;
        field[index2] = tmp;
      }
      else {
        memcpy(data, &field[index2 * elt_size], elt_size);
        memset(data, data[0] + i, elt_size);
        memcpy(&field[index2 * elt_size], data, elt_size);
      }
    }

    if (i % 1000 == 0) {
      thread_gups[args->tid] += 1000;
    }
  }

  return 0;
}

int main(int argc, char **argv)
{
  unsigned long expt;
  unsigned long size, elt_size;
  unsigned long tot_hot_size;
  unsigned long sparse_frac;
  int log_hot_size;
  struct timeval starttime, stoptime;
  double secs, gups;
  int i;
  void *p;
  struct gups_args** ga;
  char *log_filename;
  pthread_t t[MAX_THREADS];

  if (argc != 8) {
    fprintf(stderr, "Usage: %s [threads] [updates per thread] [exponent] [data size (bytes)] [noremap/remap] [sparse_frac]\n", argv[0]);
    fprintf(stderr, "  threads\t\t\tnumber of threads to launch\n");
    fprintf(stderr, "  updates per thread\t\tnumber of updates per thread\n");
    fprintf(stderr, "  exponent\t\t\tlog size of region\n");
    fprintf(stderr, "  data size\t\t\tsize of data in array (in bytes)\n");
    fprintf(stderr, "  hot size\t\t\tlog size of hot set\n");
    fprintf(stderr, "  sparse fraction\t\t\tLog distribution of sparse hot data. Argument should be X, indicating every (1 << X) pages is hot.\n");
    fprintf(stderr, "  log filename\t\t\tthe filename of instantaneous gups.\n");
    return 0;
  }

  gettimeofday(&starttime, NULL);

  threads = atoi(argv[1]);
  assert(threads <= MAX_THREADS);
  ga = (struct gups_args**)malloc(threads * sizeof(struct gups_args*));

  updates = atol(argv[2]);
  updates -= updates % 256;
  expt = atoi(argv[3]);
  assert(expt > 8);
  assert(updates > 0 && (updates % 256 == 0));
  size = (unsigned long)(1) << expt;
  size -= (size % 256);
  assert(size > 0 && (size % 256 == 0));
  elt_size = atoi(argv[4]);
  log_hot_size = atof(argv[5]);
  sparse_frac = atoi(argv[6]);
  tot_hot_size = ((unsigned long)(1) << log_hot_size);
  log_filename = argv[7];
  fprintf(stderr, "%s\n", log_filename);

  fprintf(stderr, "%lu updates per thread (%d threads)\n", updates, threads);
  fprintf(stderr, "field of 2^%lu (%lu) bytes\n", expt, size);
  fprintf(stderr, "%ld byte element size (%ld elements total)\n", elt_size, size / elt_size);

  p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_HUGETLB | MAP_POPULATE, -1, 0);
  if (p == MAP_FAILED) {
    perror("mmap");
    assert(0);
  }

  gettimeofday(&stoptime, NULL);
  fprintf(stderr, "Init took %.4f seconds\n", elapsed(&starttime, &stoptime));
  fprintf(stderr, "Region address: %p - %p\t size: %ld\n", p, (p + size), size);
  
  nelems = (size / threads) / elt_size; // number of elements per thread
  fprintf(stderr, "Elements per thread: %lu\n", nelems);

  memset(thread_gups, 0, sizeof(thread_gups));

  hotsetfile = fopen("hotsets.txt", "w");
  if (hotsetfile == NULL) {
    perror("fopen");
    assert(0);
  }

  gettimeofday(&stoptime, NULL);
  secs = elapsed(&starttime, &stoptime);
  fprintf(stderr, "Initialization time: %.4f seconds.\n", secs);

  //hemem_start_timing();

  hot_start = 0;
  hotsize = ((tot_hot_size / threads) / elt_size);
  //printf("hot_start: %p\thot_end: %p\thot_size: %lu\n", p + hot_start, p + hot_start + (hotsize * elt_size), hotsize);

  for (i = 0; i < threads; i++) {
    //printf("starting thread [%d]\n", i);
    ga[i] = (struct gups_args*)malloc(sizeof(struct gups_args));
    ga[i]->tid = i;
    ga[i]->field = p + (i * nelems * elt_size);
    ga[i]->iters = updates / 3;
    ga[i]->size = nelems;
    ga[i]->elt_size = elt_size;
    ga[i]->hot_start = 0;        // hot set at start of thread's region
    ga[i]->hotsize = hotsize;
    ga[i]->sparse_frac = sparse_frac;
  }

  if (hotset_only) {
    for (i = 0; i < threads; i++) {
      int r = pthread_create(&t[i], NULL, prefill_hotset, (void*)ga[i]);
      assert(r == 0);
    }
    // wait for worker threads
    for (i = 0; i < threads; i++) {
      int r = pthread_join(t[i], NULL);
      assert(r == 0);
    }
    sleep(5);
    fprintf(stderr, "Finished prefetching\n");
  }

  gettimeofday(&starttime, NULL);
  // run through gups once to touch all memory
  // spawn gups worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, do_gups_dense, (void*)ga[i]);
    assert(r == 0);
  }

  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }
  //hemem_print_stats();

  gettimeofday(&stoptime, NULL);

  secs = elapsed(&starttime, &stoptime);
  //printf("Elapsed time:\t%.4f\tseconds.\n", secs);
  gups = threads * ((double)updates) / (secs * 1.0e9);
  //printf("GUPS =\t%.10f\n", gups);
  memset(thread_gups, 0, sizeof(thread_gups));

  filename = "indices2.txt";

  pthread_t print_thread;
  int pt = pthread_create(&print_thread, NULL, print_instantaneous_gups, log_filename);
  assert(pt == 0);
  
  fprintf(stderr, "Timing.\n");
  gettimeofday(&starttime, NULL);

  hemem_clear_stats2();
  // spawn gups worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, do_gups_dense, (void*)ga[i]);
    assert(r == 0);
  }

  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }
  
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, do_gups_sparse, (void*)ga[i]);
    assert(r == 0);
  }

  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }
  
  
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, do_gups_dense, (void*)ga[i]);
    assert(r == 0);
  }

  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }
  
  memset(thread_gups, 0, sizeof(thread_gups));
  
  gettimeofday(&stoptime, NULL);
  hemem_print_stats(stdout);

  secs = elapsed(&starttime, &stoptime);
  printf("Elapsed time:\t%.4f\tseconds.\t", secs);
  gups = threads * ((double)(updates / 3 * 3)) / (secs * 1.0e9);
  printf("GUPS =\t%.10f\n", gups);


#if 0
  FILE* pebsfile = fopen("pebs.txt", "w+");
  assert(pebsfile != NULL);
  for (uint64_t addr = (uint64_t)p; addr < (uint64_t)p + size; addr += (2*1024*1024)) {
    struct hemem_page *pg = get_hemem_page(addr);
    assert(pg != NULL);
    if (pg != NULL) {
      fprintf(pebsfile, "0x%lx:\t%lu\t%lu\t%lu\n", pg->va, pg->tot_accesses[DRAMREAD], pg->tot_accesses[NVMREAD], pg->tot_accesses[WRITE]);
    }
  }
#endif
  //hemem_stop_timing();

  for (i = 0; i < threads; i++) {
    //free(ga[i]->indices);
    free(ga[i]);
  }
  free(ga);

  //getchar();

  munmap(p, size);

  return 0;
}

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

int dramfd, nvmfd, uffd;

#include "gups.h"

#define MAX_THREADS     64

#define PAGE_NUM            3
#define PAGES               2048
#define BASE_PAGE_SIZE      (4 * 1024)
#define HUGE_PAGE_SIZE (2L * 1024L * 1024)

#define DRAM_SIZE (4L * (1024L * 1024L * 1024L))
#define NVM_SIZE (64L * (1024L * 1024L * 1024L))
//#define PAGE_SIZE BASE_PAGE_SIZE

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

//uint64_t thread_gups[MAX_THREADS];

static unsigned long updates, nelems;


#if 0
static void *print_instantaneous_gups()
{
  uint64_t last_second_gups[threads];
  FILE *f[threads];
  char fname[20];

  for (int i = 0; i < threads; i++) {
    last_second_gups[i] = 0;
    snprintf(fname, 20, "gups_%d.txt", i);
    //printf("file name: %s\n", fname);
    f[i] = fopen(fname, "w");
    if (f[i] == NULL) {
      perror("fopen");
      assert(0);
    }
  }

  for (;;) {
    for (int i = 0; i < threads; i++) {
      //fprintf(f[i], "%.10f\n", (1.0 * (abs(thread_gups[i] - last_second_gups[i]))) / (1.0e9));
      //last_second_gups[i] = thread_gups[i];
    }
    sleep(1);
    //printf("GUPS: %.10f\n", (1.0 * (abs(thread_gups[0]- last_second_gups))) / (1.0e9));
    //last_second_gups = thread_gups[0];
    //sleep(1);
  }

  return NULL;
}
#endif

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
  uint64_t index1;
  uint64_t elt_size = args->elt_size;
  char data[elt_size];
  uint64_t sparse_frac = args->sparse_frac;

  index1 = 0;
  uint64_t shift = 9;
  uint64_t shift2 = shift + sparse_frac;
  for(int iters = 0; iters < 100000; ++iters) {
  	  uint64_t page;
	  for (page = 0; page < args->hotsize / (BASE_PAGE_SIZE / elt_size); page++) {
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

static void *do_gups(void *arguments)
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
    hot_num = lfsr_fast(lfsr) % 100;
    if (hot_num < 90) {
      lfsr = lfsr_fast(lfsr);
      uint64_t location = (lfsr % args->hotsize);
      uint64_t page = location >> shift;
      uint64_t offset = location & ((1 << shift) - 1);
      index1 = args->hot_start + (page << shift2) + offset;
      //assert(index1 < args->size);
      //index1 = args->hot_start + (lfsr % args->hotsize);
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
  }

  return 0;
}

static void *map_memory(void *arguments) {
	
  struct gups_args *args = (struct gups_args*)arguments;
  uint64_t *field = (uint64_t*)(args->field);
  uint64_t sparse_frac = args->sparse_frac;
  uint64_t size = args->size * args->elt_size;
  uint64_t hot_pages = args->hotsize / (BASE_PAGE_SIZE / args->elt_size);
  
  size_t nvm_size = (NVM_SIZE / threads);
  size_t dram_size = (DRAM_SIZE / threads);
  
  off_t nvm_offset = nvm_size * args->tid;
  off_t dram_offset = dram_size * args->tid;
  uint64_t elt_size = args->elt_size;
  char data[elt_size];
  
  uint64_t sparsity = 1 << sparse_frac;
  
  uint64_t stride = PAGE_SIZE / sizeof(uint64_t);
  // Map each page to appropriate location
  for(int page = 0; page < size / PAGE_SIZE; ++page) {
  	void *newptr;
  	if(page % sparsity == 0 && page / sparsity < hot_pages && dram_size >= PAGE_SIZE) {
	    newptr = mmap((void*)&field[page * stride], PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE | MAP_FIXED, dramfd, dram_offset);
	    dram_size -= PAGE_SIZE;
	    dram_offset += PAGE_SIZE;
		if (newptr == MAP_FAILED) {
			  fprintf(stderr, "line %d: %d %ld %ld\n", __LINE__, page, page * stride, dram_offset);
		  perror("newptr mmap  ");
		  assert(0);
		}
		if (newptr != (void*)&field[page * stride]) {
		  fprintf(stderr, "mapped address is not same as faulting address\n");
		  assert(false);
		}
    }
    else {
    	newptr = mmap((void*)&field[page * stride], PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE | MAP_FIXED, nvmfd, nvm_offset);
    	nvm_size -= PAGE_SIZE;
    	nvm_offset += PAGE_SIZE;
    	if(nvm_size <= 0) {
		  fprintf(stderr, "Out of memory line %d\n", __LINE__);
		  assert(false);
		}
		if (newptr == MAP_FAILED) {
			  fprintf(stderr, "line %d: %d %ld %ld\n", __LINE__, page, page * stride, nvm_offset);
		  perror("newptr mmap  ");
		  assert(0);
		}
		if (newptr != (void*)&field[page * stride]) {
		  fprintf(stderr, "mapped address is not same as faulting address\n");
		  assert(false);
		}
    }
    
    memcpy(data, &field[page * stride], elt_size);
    memset(data, data[0] + page, elt_size);
    memcpy(&field[page * stride], data, elt_size);
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
  pthread_t t[MAX_THREADS];

  if (argc != 7) {
    fprintf(stderr, "Usage: %s [threads] [updates per thread] [exponent] [data size (bytes)] [noremap/remap] [sparse_frac]\n", argv[0]);
    fprintf(stderr, "  threads\t\t\tnumber of threads to launch\n");
    fprintf(stderr, "  updates per thread\t\tnumber of updates per thread\n");
    fprintf(stderr, "  exponent\t\t\tlog size of region\n");
    fprintf(stderr, "  data size\t\t\tsize of data in array (in bytes)\n");
    fprintf(stderr, "  hot size\t\t\tlog size of hot set\n");
    fprintf(stderr, "  sparse fraction\t\t\tLog distribution of sparse hot data. Argument should be X, indicating every (1 << X) pages is hot.\n");
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

  fprintf(stderr, "%lu updates per thread (%d threads)\n", updates, threads);
  fprintf(stderr, "field of 2^%lu (%lu) bytes\n", expt, size);
  fprintf(stderr, "%ld byte element size (%ld elements total)\n", elt_size, size / elt_size);

  p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
  if (p == MAP_FAILED) {
    perror("mmap");
    assert(0);
  }

  gettimeofday(&stoptime, NULL);
  fprintf(stderr, "Init took %.4f seconds\n", elapsed(&starttime, &stoptime));
  fprintf(stderr, "Region address: %p - %p\t size: %ld\n", p, (p + size), size);
  
  nelems = (size / threads) / elt_size; // number of elements per thread
  fprintf(stderr, "Elements per thread: %lu\n", nelems);

  //memset(thread_gups, 0, sizeof(thread_gups));

  hotsetfile = fopen("hotsets.txt", "w");
  if (hotsetfile == NULL) {
    perror("fopen");
    assert(0);
  }

  gettimeofday(&stoptime, NULL);
  secs = elapsed(&starttime, &stoptime);
  fprintf(stderr, "Initialization time: %.4f seconds.\n", secs);

  //pthread_t print_thread;
  //int pt = pthread_create(&print_thread, NULL, print_instantaneous_gups, NULL);
  //assert(pt == 0);


  hot_start = 0;
  hotsize = ((tot_hot_size / threads) / elt_size);
  //printf("hot_start: %p\thot_end: %p\thot_size: %lu\n", p + hot_start, p + hot_start + (hotsize * elt_size), hotsize);

  for (i = 0; i < threads; i++) {
    //printf("starting thread [%d]\n", i);
    ga[i] = (struct gups_args*)malloc(sizeof(struct gups_args));
    ga[i]->tid = i;
    ga[i]->field = p + (i * nelems * elt_size);
    ga[i]->iters = updates;
    ga[i]->size = nelems;
    ga[i]->elt_size = elt_size;
    ga[i]->hot_start = 0;        // hot set at start of thread's region
    ga[i]->hotsize = hotsize;
    ga[i]->sparse_frac = sparse_frac;
    
  }

  dramfd = open("/dev/dax0.0", O_RDWR);
  if (dramfd < 0) {
    perror("dram open");
  }
  assert(dramfd >= 0);

  nvmfd = open("/dev/dax1.0", O_RDWR);
  if (nvmfd < 0) {
    perror("nvm open");
  }
  assert(nvmfd >= 0);
  
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, map_memory, (void*)ga[i]);
	assert(r == 0);
  }
  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }
/*
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
*/
  gettimeofday(&starttime, NULL);
  // run through gups once to touch all memory
  // spawn gups worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, do_gups, (void*)ga[i]);
    assert(r == 0);
  }

  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }

  gettimeofday(&stoptime, NULL);

  secs = elapsed(&starttime, &stoptime);
  //printf("Elapsed time:\t%.4f\tseconds.\n", secs);
  gups = threads * ((double)updates) / (secs * 1.0e9);
  //printf("GUPS =\t%.10f\n", gups);
  //memset(thread_gups, 0, sizeof(thread_gups));

  filename = "indices2.txt";

  fprintf(stderr, "Timing.\n");
  gettimeofday(&starttime, NULL);

  // spawn gups worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_create(&t[i], NULL, do_gups, (void*)ga[i]);
    assert(r == 0);
  }

  // wait for worker threads
  for (i = 0; i < threads; i++) {
    int r = pthread_join(t[i], NULL);
    assert(r == 0);
  }
  gettimeofday(&stoptime, NULL);

  secs = elapsed(&starttime, &stoptime);
  printf("Elapsed time:\t%.4f\tseconds.\t", secs);
  gups = threads * ((double)updates) / (secs * 1.0e9);
  printf("GUPS =\t%.10f\n", gups);

  //memset(thread_gups, 0, sizeof(thread_gups));


  for (i = 0; i < threads; i++) {
    //free(ga[i]->indices);
    free(ga[i]);
  }
  free(ga);

  //getchar();

  munmap(p, size);

  return 0;
}

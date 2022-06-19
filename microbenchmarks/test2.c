#include <sys/mman.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sched.h>
#include <assert.h>
#include <chrono>
#define TIME_NOW std::chrono::high_resolution_clock::now()
#define time_diff(a, b) std::chrono::duration_cast<std::chrono::nanoseconds>(a - b).count()
#include <stdio.h>

/*
static __attribute__((constructor)) void init(void) {
	fprintf(stderr, "INITED\n");
}*/

#define NVMSIZE   (60L * (1024L * 1024L * 1024L))
#define DRAMSIZE  (6L * (1024L * 1024L * 1024L))
#define DRAMPATH  "/dev/dax0.0"
#define NVMPATH   "/dev/dax1.0"


void *dram_devdax_mmap;
void *nvm_devdax_mmap;
int dramfd = -1;
int nvmfd = -1;

#define NUMBER 100000000

void visit_thread(int *data, size_t size) {
	size_t x = 0;
	for(size_t i = 0; i < NUMBER; ++i) {
		size_t j = (rand() + x) % size;
		x = data[j];
		++x;
		data[j] = x;
	}
}

int main() {
  
  dramfd = open(DRAMPATH, O_RDWR);
  if (dramfd < 0) {
    perror("dram open");
  }
  assert(dramfd >= 0);

  nvmfd = open(NVMPATH, O_RDWR);
  if (nvmfd < 0) {
    perror("nvm open");
  }
  assert(nvmfd >= 0);
  
  dram_devdax_mmap = mmap(NULL, DRAMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, dramfd, 0);
  if (dram_devdax_mmap == MAP_FAILED) {
    perror("dram devdax mmap");
    assert(0);
  }
  
  nvm_devdax_mmap  = mmap(NULL, NVMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, nvmfd, 0);
  if (nvm_devdax_mmap == MAP_FAILED) {
    perror("nvm devdax mmap");
    assert(0);
  }
  
  
  //dram_devdax_mmap = malloc(DRAMSIZE);
  //nvm_devdax_mmap = malloc(NVMSIZE);
  for(int i = 0; i < 5; ++i) {
  	auto start = TIME_NOW;
  	visit_thread((int*)dram_devdax_mmap, DRAMSIZE / sizeof(int)); 
  	auto end = TIME_NOW;
  	printf("DRAM:\t%ld\tns\tTime per:\t%ld\n", time_diff(end, start), time_diff(end, start) / NUMBER);
  	auto start2 = TIME_NOW;
  	visit_thread((int*)nvm_devdax_mmap, NVMSIZE / sizeof(int)); 
  	auto end2 = TIME_NOW;
  	printf("NVM:\t%ld\tns\tTime per:\t%ld\n", time_diff(end2, start2), time_diff(end2, start2) / NUMBER);
  }
  
  
}

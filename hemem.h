#ifndef HEMEM_H

#define HEMEM_H

#include <pthread.h>

//#define EXAMINE_PGTABLES

#define NVMSIZE		(128L * (1024L * 1024L * 1024L))
#define DRAMSIZE	(16L * (1024L * 1024L * 1024L))

#define DRAMPATH "/dev/dax0.0"
#define NVMPATH "/dev/dax1.0"

//#define PAGE_SIZE (1024 * 1024 * 1024)
//#define PAGE_SIZE (2 * (1024 * 1024))
#define PAGE_SIZE (4 * 1024)

extern pthread_t fault_thread;

extern int dramfd;
extern int nvmfd;
extern long uffd;
extern int init;
extern unsigned long mem_allocated;
extern int alloc_nvm;
extern int wp_faults_handled;
extern int missing_faults_handled;

void hemem_init();
void* hemem_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int hemem_munmap(void* addr, size_t length);
void *handle_fault(void* arg);

#ifdef EXAMINE_PGTABLES

struct pagemapEntry {
  unsigned long long pfn : 54;
  unsigned int soft_dirty : 1;
  unsigned int exclusive : 1;
  unsigned int file_page : 1;
  unsigned int swapped : 1;
  unsigned int present : 1;
};

void *examine_pagetables();

#endif /*EXAMINE_PGTABLES*/

#endif /* HEMEM_H */

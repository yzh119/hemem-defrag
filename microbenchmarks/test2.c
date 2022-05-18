#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
/*
static __attribute__((constructor)) void init(void) {
	fprintf(stderr, "INITED\n");
}*/

int main() {
	fprintf(stderr, "START EXECUTION\n");
	int *data = NULL;
//	mmap(data, 2 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_HUGETLB | MAP_POPULATE | MAP_ANONYMOUS, -1, 0);
	return 0;
}

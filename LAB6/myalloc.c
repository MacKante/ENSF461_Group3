#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <sys/mman.h>
#include "myalloc.h"

void* _arena_start = NULL;
size_t _arena_size;

extern int myinit(size_t size) {
    printf("Initializing arena:\n");
    printf("...requested size %d bytes", size);

    int page_size = getpagesize();
    printf("...pagesize is %d bytes\n", page_size);

    printf("...adjusting size with page boundaries\n");
    
    int page_count = floor((size - 1) / 4096) + 1;
    int adjusted_size = page_count * page_size;
    
    printf("...adjusted size is %d bytes", adjusted_size);

    printf("...mapping arena with mmap()\n");
    _arena_start = mmap(NULL, adjusted_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (_arena_start == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    printf("...arena starts at %p\n", _arena_start);
    _arena_size = _arena_start + adjusted_size;
    printf("...arena ends at : %p\n", _arena_size);
    
    return 0;
}

int mydestroy(){
    printf("Destroying Arena:\n");
    printf("...unmapping arena with munmap()\n");
    if(munmap(_arena_start, _arena_size)==-1){
        perror("munmap");
        return -1;
    }
    _arena_start = NULL;
    _arena_size = 0;
   }

void* myalloc(size_t size){

}

void myfree(void *ptr){

}

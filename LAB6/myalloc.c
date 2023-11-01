#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

#include <math.h>
#include <sys/mman.h>
#include <unistd.h>
#include "myalloc.h"

#define PRINTF_GREEN(...) fprintf(stderr, "\033[32m"); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\033[0m");

void* _arena_start = NULL;
size_t _arena_size;
void* _arena_end = NULL;
int statusno = 0;

extern int myinit(size_t size) {
    PRINTF_GREEN("Initializing arena:\n");
    PRINTF_GREEN("...requested size %zu bytes\n", size);

    int page_size = getpagesize();
    PRINTF_GREEN("...pagesize is %u bytes\n", page_size);

    PRINTF_GREEN("...adjusting size with page boundaries\n");
    int page_count = floor((size - 1) / 4096) + 1;
    _arena_size = page_count * page_size;
    PRINTF_GREEN("...adjusted size is %lu bytes\n", _arena_size);

    PRINTF_GREEN("...mapping arena with mmap()\n");
    _arena_start = mmap(NULL, _arena_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (_arena_start == MAP_FAILED) {
        _arena_start = NULL;
        _arena_size = 0;
        _arena_end = NULL;
        return ERR_BAD_ARGUMENTS;
    }

    PRINTF_GREEN("...arena starts at %p\n", _arena_start);
    void* _arena_end = (char*)_arena_start + _arena_size;
    PRINTF_GREEN("...arena ends at : %p\n", _arena_end);

    // Creating Free Chunk 0
    node_t* header = (node_t*)_arena_start;
    header->size = _arena_size - sizeof(node_t);
    header->is_free = 1;
    header->fwd = NULL;
    header->bwd = NULL;

    return _arena_size;
}

extern int mydestroy() {
    if( _arena_start == NULL){
        statusno = ERR_UNINITIALIZED;
        return statusno;
    }
    PRINTF_GREEN("Destroying Arena:\n");

    PRINTF_GREEN("...unmapping arena with munmap()\n");
    if(munmap(_arena_start, _arena_size) == -1){
        return -1;
    }

    _arena_start = NULL;
    _arena_size = 0;
    _arena_end = NULL;

    return 0;
}

extern void* myalloc(size_t size) {
    if( _arena_start == NULL){
        statusno = ERR_UNINITIALIZED;
        return NULL;
    } 
    PRINTF_GREEN("Allocating Memory:\n");
    
    PRINTF_GREEN("...looking for free chunk of >= %ld bytes\n", size);
    node_t* current = (node_t*)_arena_start;

    while(current != NULL) {
        size_t chunk_size = current->size;
        
        if(chunk_size >= size && current->is_free == 1 && (chunk_size - size - sizeof(node_t)) >= sizeof(node_t)){
            PRINTF_GREEN("...found free chunk of %ld bytes with header at %p\n", chunk_size, (void*)current);
            PRINTF_GREEN("...free chunk->fwd currently points to %p\n", (void*)(current->fwd));
            PRINTF_GREEN("...free chunk->bwd currently points to %p\n", (void*)(current->bwd));
            
            // fix allocated chunk
            node_t* to_alloc = current;
            to_alloc->fwd = current->fwd;

            PRINTF_GREEN("...checking if splitting is required\n");
            // split the free chunk into an allocated chunk and a new free chunk
            if (chunk_size >= (size + sizeof(node_t))){
                PRINTF_GREEN("...splitting is required\n");
                node_t* new_free = (node_t*)((char*)current + size + sizeof(node_t));
                new_free->size = chunk_size - size - sizeof(node_t);
                new_free->bwd = current;
                new_free->fwd = current->fwd;
                new_free->is_free = 1;

                to_alloc->fwd = new_free;
                to_alloc->size = size;

            } else {
                PRINTF_GREEN("...splitting is not required\n");
                to_alloc->size = chunk_size;
            }

            PRINTF_GREEN("updating chunk header at %p\n", (void*)to_alloc);
            to_alloc->is_free = 0;
            to_alloc->bwd = current->bwd;

            PRINTF_GREEN("...being careful with my pointer arthimetic and void pointer casting\n");
            void* alloc_data = (void*)((char*)current + sizeof(node_t));
            PRINTF_GREEN("...allocation starts at %p\n", alloc_data);

            return alloc_data;
            
        } else {
            current = current->fwd;
        }
    }
    statusno = ERR_OUT_OF_MEMORY;
    return NULL;
}

extern void myfree(void *ptr){
    PRINTF_GREEN("Freeing allocated memory:\n");
    PRINTF_GREEN("...supplied pointer %p:\n", ptr);

    PRINTF_GREEN("...being careful with my pointer arthimetic and void pointer casting\n");
    node_t* current = (node_t*)((char*)ptr - sizeof(node_t)); // points to the ptr's header

    PRINTF_GREEN("...accessing chunk header at %p\n", current);
    current->is_free = 1; // the ptr's header is now free

    PRINTF_GREEN("...chunk of size %ld\n", current->size);

    PRINTF_GREEN("...checking if coalescing is needed\n");
    int coal_fwd = 0;
    int coal_bwd = 0;
    
    if (current->fwd != NULL && current->fwd->is_free == 1) {
        coal_fwd = 1;
    }

    if (current->bwd != NULL && current->bwd->is_free == 1) {
        coal_bwd = 1;
    }

    if(coal_fwd || coal_bwd) {
        PRINTF_GREEN("...coalescing is needed.\n");
    } else {
        PRINTF_GREEN("...coalescing is not needed.\n");
    }

    if(coal_fwd){
        // Setting new size
        current->size += sizeof(node_t) + current->fwd->size; // current size = the size of a header object for the next because it is freed+ the size of next chunks size

        node_t* free_chunk = current->fwd; // the free chunk is the current header's next fwd 
        node_t* free_fwd = free_chunk->fwd; // the enxt chunk after this is the current->fwd->fwd

        // pop free chunk
        if(free_fwd != NULL){
            free_fwd->bwd = current;
        }
        current->fwd = free_fwd;

        // kill free
        free_chunk->bwd = NULL;
        free_chunk->fwd = NULL;
        free_chunk->size = 0;
        free_chunk->is_free = 0;
    }

    // if bwd of to be freed chunk is also free
    if(coal_bwd){
        node_t* free_chunk = current->bwd;
        node_t* current_fwd = current->fwd;

        // modify size of the free chunk
        free_chunk->size += sizeof(node_t) + current->size;

        // pop current chunk
        if(current_fwd != NULL){
            current_fwd->bwd = free_chunk;
        }
        free_chunk->fwd = current_fwd;

        // kill current
        current->bwd = NULL;
        current->fwd = NULL;
        current->size = 0;
        current->is_free = 0;        
    }
}

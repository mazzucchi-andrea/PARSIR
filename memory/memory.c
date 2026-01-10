#include <memory.h>
#include <numaif.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "setup.h"

#define MIN_CHUNK_SIZE (32)
#define MAX_CHUNK_SIZE (MIN_CHUNK_SIZE << 7) // 4KB is the currently set max chunk size
#define EPSILON 0.000001

unsigned long MAX_MEMORY = (1 << 21); // maximum amount of memory manageable per object

#ifdef GRID_CKPT
#define BITMAP_SIZE (MAX_MEMORY / MOD) / 8 + 1
#endif

unsigned long base_address;

unsigned long SEGMENTS;
unsigned long SEGMENT_PAGES;

typedef struct _area {
    void **addresses;
    int top_elem;
#ifdef SPECULATION
    void **addresses_ckpt;
    int top_elem_ckpt;
#ifdef CHUNK_BASED
    void *bitmap;
#endif
#endif
    int size;
} area;

area *allocators[OBJECTS];
void *base[OBJECTS];

void allocators_base_init(void) {
    int i;

    SEGMENTS = MAX_MEMORY >> 3; // each segment is configured to be a set of 64 pages
    SEGMENT_PAGES = SEGMENTS >> 12;
    AUDIT printf("base allocators init - max memory is %ld - segment size is %ld - segment pages are %ld - memory "
                 "areas are %d\n",
                 MAX_MEMORY, SEGMENTS, SEGMENT_PAGES, (int)((double)(MAX_MEMORY >> 12) / (double)(SEGMENT_PAGES)));
    for (i = 0; i < OBJECTS; i++) {
        allocators[i] = malloc((int)((double)(MAX_MEMORY >> 12) / (double)(SEGMENT_PAGES)) * sizeof(area));
        if (!allocators[i]) {
            printf("allocators base init error\n");
            exit(EXIT_FAILURE);
        }
        base[i] = NULL;
    }
}

void object_allocator_setup(void) {
    int i;
    int j;
    int current;
    int NUMAnode;
    unsigned long target_address;
    void *mapping;
    void *addr;
    int ret;
    unsigned long mask;
    int chunk_size;
    void *limit;
    void *aux;

    base_address = 8 * (1024 * MAX_MEMORY); // this can be setup in a different manner if needed
    AUDIT printf("base_address: 0x%lx\n", base_address);
    current = get_current();
    NUMAnode = get_NUMAnode();

#if GRID_CKPT
    target_address = base_address + current * (3 * MAX_MEMORY * MEM_NODES);
#elif CHUNK_BASED
    target_address = base_address + current * (2 * MAX_MEMORY * MEM_NODES);
#else
    target_address = base_address + current * (MAX_MEMORY * MEM_NODES);
#endif

    AUDIT printf("allocator setup for object %d\n", current);

    mask = 0x1 << NUMAnode; // this is for NUMA binding of memory zones
    AUDIT printf("thread running on NUMA node %d - mask for setting up object %d is %lu\n", NUMAnode, current, mask);
    for (i = 0; i < 1; i++) {
        // the above line is left just to let the developer restart from here for NUMA ubiquitousness
#if GRID_CKPT
        addr = mmap((void *)target_address, MAX_MEMORY * 2 + BITMAP_SIZE, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, 0, 0);
#elif CHUNK_BASED
        addr = mmap((void *)target_address, MAX_MEMORY * 2, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, 0, 0);
#else
        addr = mmap((void *)target_address, MAX_MEMORY, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                    0, 0);
#endif
        if (addr != (void *)target_address) {
            printf("mmap failure for object %d\n", current);
            exit(EXIT_FAILURE);
        };
#if GRID_CKPT
        ret = mbind(addr, MAX_MEMORY * 2 + BITMAP_SIZE, MPOL_BIND, &mask, sizeof(unsigned long), 0);
#elif CHUNK_BASED
        ret = mbind(addr, MAX_MEMORY * 2, MPOL_BIND, &mask, sizeof(unsigned long), 0);
#else
        ret = mbind(addr, MAX_MEMORY, MPOL_BIND, &mask, sizeof(unsigned long), 0);
#endif
        if (ret == -1) {
            printf("mbibd failure for object %d\n", current);
            exit(EXIT_FAILURE);
        };
        *(char *)addr = 'f'; // materialize the 3-rd level page table entry
        AUDIT printf("mapped zone at address %p for object %d\n", (void *)target_address, current);
#if GRID_CKPT
        target_address += MAX_MEMORY * 3;
#elif CHUNK_BASED
        target_address += MAX_MEMORY * 2;
#else
        target_address += MAX_MEMORY;
#endif
        mask = mask < 1;
    }

#if GRID_CKPT
    target_address = base_address + current * (3 * MAX_MEMORY * MEM_NODES);
#elif CHUNK_BASED
    target_address = base_address + current * (2 * MAX_MEMORY * MEM_NODES);
#else
    target_address = base_address + current * (MAX_MEMORY * MEM_NODES);
#endif
    base[current] = (void *)target_address;

    // the allocator is based on a stack of free-chunk addresses
    // now we initialize the free stacks of the per-object allocator
    // the minimal chunk size managed is 32 bytes

    AUDIT printf("running the allocator setup before iteration for object %d - "
                 "cycles are %d\n",
                 current, (int)((double)(MAX_MEMORY >> 12) / (double)(SEGMENT_PAGES)));
    limit = base[current]; //(void*)target_address;
    for (i = 0, chunk_size = MIN_CHUNK_SIZE; i < (int)((double)(MAX_MEMORY >> 12) / (double)(SEGMENT_PAGES));
         chunk_size *= 2, i++) {

        AUDIT printf("running the allocator setup for object %d\n", current);

        (allocators[current])[i].size = ((SEGMENT_PAGES << 12) / chunk_size);
        (allocators[current])[i].addresses = malloc(sizeof(void *) * ((allocators[current])[i].size));
        if (!(allocators[current])[i].addresses) {
            printf("area allocation error\n");
            exit(EXIT_FAILURE);
        }
        AUDIT printf("current is %d - allocated area with %ld elements (chunk size is %d)\n", current,
                     (SEGMENT_PAGES << 12) / chunk_size, chunk_size);
        for (j = 0; j < ((allocators[current])[i].size); j++) {
            (allocators[current])[i].addresses[j] = limit;
            limit += chunk_size;
        }
        (allocators[current])[i].top_elem = 0;
#ifdef SPECULATION
        (allocators[current])[i].addresses_ckpt = malloc(sizeof(void *) * ((allocators[current])[i].size));
        if (!(allocators[current])[i].addresses_ckpt) {
            printf("(ckpt) area allocation error\n");
            exit(EXIT_FAILURE);
        }
        AUDIT printf("current is %d - allocated (ckpt) area with %ld elements (chunk size is %d)\n", current,
                     (SEGMENT_PAGES << 12) / chunk_size, chunk_size);
        (allocators[current])[i].top_elem_ckpt = 0;
#ifdef CHUNK_BASED
        (allocators[current])[i].bitmap = malloc((sizeof(void *) * ((allocators[current])[i].size)) >> 3);
        if (!(allocators[current])[i].bitmap) {
            printf("(ckpt) area-bitmap allocation error\n");
            exit(EXIT_FAILURE);
        }
        AUDIT printf("current is %d - allocated (ckpt) area-nitmap with %ld elements\n", current,
                     (sizeof(void *) * ((SEGMENT_PAGES << 12) / chunk_size)));
#endif
#endif
    }
}

#ifdef SPECULATION
void set_allocator_ckpt(int current) {
    AUDIT printf("Set Allocator Checkpoint - current is %d\n", current);
    int chunk_size = MIN_CHUNK_SIZE;
    for (int i = 0; chunk_size <= MAX_CHUNK_SIZE; i++) {
        allocators[current][i].top_elem_ckpt = allocators[current][i].top_elem;
        memset(allocators[current][i].addresses_ckpt, 0, sizeof(void *) * allocators[current][i].size);
        for (int j = allocators[current][i].top_elem; j < allocators[current][i].size; j++) {
            allocators[current][i].addresses_ckpt[j] = allocators[current][i].addresses[j];
        }
        chunk_size = chunk_size << 1;
#ifdef CHUNK_BASED
        memset(allocators[current][i].bitmap, 0, (sizeof(void *) * ((allocators[current])[i].size)) >> 3);
#endif
    }
}

void restore_allocator(int current) {
    AUDIT printf("Restore Allocator - current is %d\n", current);
    int chunk_size = MIN_CHUNK_SIZE;
    for (int i = 0; chunk_size <= MAX_CHUNK_SIZE; i++) {
        allocators[current][i].top_elem = allocators[current][i].top_elem_ckpt;
        for (int j = allocators[current][i].top_elem_ckpt; j < allocators[current][i].size; j++) {
            allocators[current][i].addresses[j] = allocators[current][i].addresses_ckpt[j];
        }
        chunk_size = chunk_size << 1;
    }
}

#ifdef CHUNK_BASED
void ckpt_chunk(void *ptr) {
    int bitmap_offset, chunk, chunk_size, current, index;
    uint8_t bitmask, bit_index;
    current = get_current();
    index = (int)((double)((ptr - base[current]) >> 12) / (double)(SEGMENT_PAGES));
    chunk_size = MIN_CHUNK_SIZE << index;

    for (int i = 0; i < allocators[current][index].size; i++) {
        if (ptr < allocators[current][index].addresses[i] + chunk_size &&
            ptr >= allocators[current][index].addresses[i]) {
            chunk = i;
            break;
        }
    }
    bit_index = chunk % 8;
    bitmap_offset = chunk >> 3;
    bitmask = 1 << bit_index;
    if (!(*(uint8_t *)(allocators[current][index].bitmap + bitmap_offset) & bitmask)) {
        memcpy(allocators[current][index].addresses[chunk] + MAX_MEMORY, allocators[current][index].addresses[chunk],
               chunk_size);
        *(uint8_t *)(allocators[current][index].bitmap + bitmap_offset) |= bitmask;
    }
    AUDIT printf("Saving chunk - current is %d - address is %p - index is %d - chunk is %d - chunk size is %d - bit "
                 "index is %d - "
                 "bitmap offset is %d\n",
                 current, ptr, index, chunk, chunk_size, bit_index, bitmap_offset);
}

void restore_chunks(int current) {
    int chunk_size;
    uint8_t current_byte;
    size_t bitmap_size;
    chunk_size = MIN_CHUNK_SIZE;
    for (int i = 0; chunk_size <= MAX_CHUNK_SIZE; i++) {
        bitmap_size = (sizeof(void *) * ((allocators[current])[i].size)) >> 3;
        for (int j = 0; j < bitmap_size; j++) {
            current_byte = *(uint8_t *)(allocators[current][i].bitmap + j);
            if (current_byte == 0) {
                continue;
            }
            for (int k = 0; k < 8; k++) {
                if (((current_byte >> k) & 1) == 1) {
                    memcpy(allocators[current][i].addresses[j * 8 + k],
                           allocators[current][i].addresses[j * 8 + k] + MAX_MEMORY, chunk_size);
                }
            }
        }
        memset(allocators[current][i].bitmap, 0, bitmap_size);
        chunk_size = chunk_size << 1;
    }
}
#endif
#endif
void *__wrap_malloc(size_t size) {
    int current;
    int index;
    void *chunk_address;
    int min_chunk_size = MIN_CHUNK_SIZE;
    int max_chunk_size = MAX_CHUNK_SIZE;
    int i;

    current = get_current();
    if (size == 0) {
        return NULL;
    }
    for (i = 0; min_chunk_size <= max_chunk_size; i++) {
        if (size <= min_chunk_size) {
            index = i;
            break;
        }
        min_chunk_size = min_chunk_size << 1;
        index = i;
    }
    // index = (int)( ((double)size / (double)MIN_CHUNK_SIZE) - EPSILON);
    AUDIT printf("wrapping malloc for object %d - size is %ld - index is %d\n", current, size, index);
redo:
    if (index >= (int)((double)(MAX_MEMORY >> 12) / (double)(SEGMENT_PAGES))) {
        printf("unavailable memory for object %d\n", current);
        exit(EXIT_FAILURE);
        return NULL;
    }
    if ((allocators[current])[index].top_elem == (allocators[current])[index].size) {
        index++;
        goto redo;
    }
    chunk_address = (allocators[current])[index].addresses[(allocators[current])[index].top_elem];
    (allocators[current])[index].top_elem++;
    AUDIT printf("returning address %p to object %d\n", chunk_address, current);
    return chunk_address;
}

void __wrap_free(void *ptr) {
    int current;
    int index;
    current = get_current();
    AUDIT printf("freeing address %p for object %d\n", ptr, current);
    if (ptr < base[current] || ptr >= (base[current] + MAX_MEMORY)) {
        printf("bad address for free by object %d\n", current);
        exit(EXIT_FAILURE);
    }
    index = (int)((double)((ptr - base[current]) >> 12) / (double)(SEGMENT_PAGES));
    AUDIT printf("wrapping free for object %d - index is %d\n", current, index);
    (allocators[current])[index].addresses[--(allocators[current])[index].top_elem] = ptr;
    if ((allocators[current])[index].top_elem < 0) {
        printf("allocator corruption on free by object %d\n", current);
        exit(EXIT_FAILURE);
    }
}

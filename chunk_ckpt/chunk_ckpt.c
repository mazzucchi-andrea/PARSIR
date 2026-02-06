#include <asm/prctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "memory.h"
#include "setup.h"

// #define TEST

#ifdef TEST
uint8_t *shadow_area[OBJECTS] = {NULL};
#endif

void restore_object(int object) {
    restore_allocator(object);
    restore_chunks(object);
#ifdef TEST
    uint8_t *area = (uint8_t *)(8 * (1024 * MAX_MEMORY) + object * (2 * MAX_MEMORY * MEM_NODES));
    if (memcmp(shadow_area[object], area, MAX_MEMORY)) {
        printf("ERROR: object %d restore failed\n", object);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
#endif
}

void set_ckpt(int object) {
    set_allocator_ckpt(object);
#ifdef TEST
    uint8_t *area = (uint8_t *)(8 * (1024 * MAX_MEMORY) + object * (2 * MAX_MEMORY * MEM_NODES));
    if (shadow_area[object] == NULL) {
        shadow_area[object] = mmap(NULL, MAX_MEMORY, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        if (shadow_area[object] == MAP_FAILED) {
            printf("object %d - shadow_area mmap failed\n", object);
            exit(EXIT_FAILURE);
        }
    }
    memcpy(shadow_area[object], area, MAX_MEMORY);
    if (memcmp(shadow_area[object], area, MAX_MEMORY)) {
        printf("object %d - set ckpt failed\n", object);
        exit(EXIT_FAILURE);
    }
#endif
#ifdef CHUNK_FULL
    set_used_chunks_ckpt(object);
#endif
}
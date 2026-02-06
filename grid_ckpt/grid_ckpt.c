#include <asm/prctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "ckpt_setup.h"
#include "engine.h"
#include "memory.h"
#include "setup.h"

typedef struct _seeds {
    uint32_t seed1; // seed passed in input to randomization functions
    uint32_t seed2; // seed passed in input to randomization functions
} seeds;

extern uint32_t *seeds1[OBJECTS];
extern uint32_t *seeds2[OBJECTS];

seeds object_seeds[OBJECTS];

// #define TEST

#ifdef TEST
uint8_t *shadow_area[OBJECTS] = {NULL};
#endif

void tls_setup() { _tls_setup(); }

void save_seeds(int object) {
    object_seeds[object].seed1 = *seeds1[object];
    AUDIT printf("object %d - saving seed1 %d\n", object, object_seeds[object].seed1);
    object_seeds[object].seed2 = *seeds2[object];
    AUDIT printf("object %d - saving seed2 %d\n", object, object_seeds[object].seed2);
}

void restore_seeds(int object) {
    *seeds1[object] = object_seeds[object].seed1;
    AUDIT printf("object %d - restore seed1 %d\n", object, *seeds1[object]);
    *seeds2[object] = object_seeds[object].seed2;
    AUDIT printf("object %d - restore seed2 %d\n", object, *seeds2[object]);
}

void restore_object(int object) {
    restore_seeds(object);
    uint8_t *area = (uint8_t *)(8 * (1024 * MAX_MEMORY) + object * (3 * MAX_MEMORY * MEM_NODES));
    restore_allocator(object);
    _restore_area(area);
#ifdef TEST
    if (memcmp(shadow_area[object], area, MAX_MEMORY)) {
        printf("ERROR: object %d restore failed\n", object);
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
#endif
}

void set_ckpt(int object) {
    save_seeds(object);
    set_allocator_ckpt(object);
    uint8_t *area = (uint8_t *)(8 * (1024 * MAX_MEMORY) + object * (3 * MAX_MEMORY * MEM_NODES));
#ifdef TEST
    if (shadow_area[object] == NULL) {
        shadow_area[object] = mmap(NULL, MAX_MEMORY, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    }
    memcpy(shadow_area[object], area, MAX_MEMORY);
    if (memcmp(shadow_area[object], area, MAX_MEMORY)) {
        printf("set ckpt failed\n");
        exit(EXIT_FAILURE);
    }
#endif
    _set_ckpt(area);
}
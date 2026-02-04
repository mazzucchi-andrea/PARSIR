#include <asm/prctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "ckpt_setup.h"
#include "memory.h"
#include "setup.h"

// #define TEST

#ifdef TEST
uint8_t *shadow_area[OBJECTS] = {NULL};
#endif

void tls_setup() { _tls_setup(); }

void restore_object(int object) {
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + object * (3 * MAX_MEMORY * MEM_NODES));
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
    set_allocator_ckpt(object);
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + object * (3 * MAX_MEMORY * MEM_NODES));
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
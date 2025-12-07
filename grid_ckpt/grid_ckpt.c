#include <asm/prctl.h>
#include <immintrin.h> // AVX
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "setup.h"
#include "ckpt_setup.h"

#ifndef MOD
#define MOD 64
#endif

#ifndef MAX_MEMORY
#define MAX_MEMORY 0x200000UL
#endif

#if MOD == 64
#define BITARRAY_SIZE (MAX_MEMORY / 8) / 8
#elif MOD == 128
#define BITARRAY_SIZE (MAX_MEMORY / 16) / 8
#elif MOD == 256
#define BITARRAY_SIZE (MAX_MEMORY / 32) / 8
#elif MOD == 512
#define BITARRAY_SIZE (MAX_MEMORY / 64) / 8
#else
#error "Valid MODs are 64, 128, 256, and 512."
#endif

extern int arch_prctl(int code, unsigned long addr);

void tls_setup() {
    _tls_setup();
}

void restore_obj(int obj_index) {
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + obj_index * (3 * MAX_MEMORY * MEM_NODES));
    _restore_area(area);
}

void set_ckpt(int obj_index) {
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + obj_index * (3 * MAX_MEMORY * MEM_NODES));
    _set_ckpt(area);
}
#include <asm/prctl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "setup.h"
#include "memory.h"
#include "ckpt_setup.h"

void tls_setup() { _tls_setup(); }

void restore_object(int object) {
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + object * (3 * MAX_MEMORY * MEM_NODES));
    _restore_area(area);
    restore_allocator(object);
}

void set_ckpt(int object) {
    set_allocator_ckpt(object);
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + object * (3 * MAX_MEMORY * MEM_NODES));
    _set_ckpt(area);
}
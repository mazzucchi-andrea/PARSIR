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

void restore_obj(int obj_index) {
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + obj_index * (3 * MAX_MEMORY * MEM_NODES));
    _restore_area(area);
    restore_allocator(obj_index);
}

void set_ckpt(int obj_index) {
    AUDIT printf("Set Checkpoint - current is %d\n", obj_index);
    set_allocator_ckpt(obj_index);
    u_int8_t *area = (u_int8_t *)(8 * (1024 * MAX_MEMORY) + obj_index * (3 * MAX_MEMORY * MEM_NODES));
    _set_ckpt(area);
}
#include <asm/prctl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "setup.h"
#include "memory.h"

void set_ckpt(int obj_index) {
    set_allocator_ckpt(obj_index);
}

void restore_obj(int obj_index) {
    restore_chunks(obj_index);
    restore_allocator(obj_index);
}
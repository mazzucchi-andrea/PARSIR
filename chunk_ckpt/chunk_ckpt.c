#include <asm/prctl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "memory.h"
#include "setup.h"

void set_ckpt(int object) {
    set_allocator_ckpt(object);
#ifdef CHUNK_FULL
    set_used_chunks_ckpt(object);
#endif
}

void restore_object(int object) {
    restore_chunks(object);
    restore_allocator(object);
}
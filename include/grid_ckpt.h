#ifndef GRID_CKPT_H
#define GRID_CKPT_H

#include <stdint.h>

void *tls_setup();

void restore_area(int8_t *);

#endif
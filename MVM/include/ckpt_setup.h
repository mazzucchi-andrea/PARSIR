#ifndef _CKPT_SETUP_
#define _CKPT_SETUP_

#include <stdint.h>
#include <sys/types.h>

void _tls_setup();

void _restore_area(u_int8_t *);

void _set_ckpt(u_int8_t *);

#endif
/*
 * SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
 * SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _CKPT_SETUP_
#define _CKPT_SETUP_

#include <stdint.h>

#ifndef MOD
#define MOD 8
#endif

#ifndef ALLOCATOR_AREA_SIZE
#define ALLOCATOR_AREA_SIZE 0x200000UL
#endif

#if MOD == 8 || MOD == 16 || MOD == 32 || MOD == 64
#define BITMAP_SIZE (ALLOCATOR_AREA_SIZE / MOD) / 8
#else
#error "Valid MODs are 8, 16, 32, and 64."
#endif

void _tls_setup();

void _restore_area(uint8_t *);

void _set_ckpt(uint8_t *);

#endif
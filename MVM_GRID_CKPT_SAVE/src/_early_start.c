/*
 * SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
 * SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#include "head.h"

void setup_memory_access_rules() {

    void (*address)(void);
    long int ret;
    int i = 0;

    address = _instructions;
    ret = syscall(10, ((unsigned long)address) & mask, SIZE, PROT_READ | PROT_WRITE);
    AUDIT printf("(%d) protection command at address %p returned %ld (errno is %d)\n", i++, address, ret, errno);

    address = _patches;
    ret = syscall(10, ((unsigned long)address) & mask, SIZE, PROT_READ | PROT_EXEC | PROT_WRITE);
    AUDIT
    printf("(%d) protection command at address %p returned %ld (errno is %d)\n", i++, address, ret, errno);

    address = _codemap;
    ret = syscall(10, ((unsigned long)address) & mask, SIZE, PROT_READ | PROT_EXEC | PROT_WRITE);
    AUDIT
    printf("(%d) protection command at address %p returned %ld (errno is %d)\n", i++, address, ret, errno);
}

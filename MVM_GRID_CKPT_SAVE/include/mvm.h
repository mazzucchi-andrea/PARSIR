/*
 * SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
 * SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _MVM_
#define _MVM_

#define INSTRUMENT                                                                                                     \
    do {                                                                                                               \
        asm volatile("jmp 1f; "                                                                                        \
                     "cli;nop;nop;nop;nop;cli;nop;nop;nop;nop;cli;nop;nop;"                                            \
                     "nop;nop; 1:" ::);                                                                                \
    } while (0)

#endif

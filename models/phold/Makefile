# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

CC = gcc 
CFLAGS = -c -Wall -Wextra -Wpedantic -I../../include/ -I./
GRID_CKPT_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT/include -DSPECULATION
CHUNK_CKPT_FLAGS = -pie -fpie -I../../MVM_CHUNK/include -DSPECULATION
FULL_CKPT_FLAGS = -DSPECULATION
MMAP_MV_FLAGS = -pie -fpie -I../../MVM_MMAP_MV/include -DSPECULATION -DMMAP_MV
TARGET = intermediate-phold.o 

SRCS = application.c 

M=1
OBJECTS=1024
LOOKAHEAD=1.0
P_SHIFT=5

RUN_FLAGS = -DOBJECTS=$(OBJECTS) -DLOOKAHEAD=$(LOOKAHEAD) -DM=$(M) -DP_SHIFT=$(P_SHIFT)

ifeq ($(BENCHMARK),1)
RUN_FLAGS += -DBENCHMARKING
endif

all: 
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET) $(RUN_FLAGS)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

grid_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET) 
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

chunk_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(CHUNK_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

full_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(FULL_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

mmap_mv:
	$(CC) $(SRCS) $(CFLAGS) $(MMAP_MV_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

clean:
	$(RM) *.o $(TARGET)

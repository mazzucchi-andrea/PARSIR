# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

CC = gcc 
CFLAGS = -c -Wall -Wextra -Wpedantic -I../../include/ -I./ -lm -Wimplicit-fallthrough
CHUNK_CKPT_FLAGS = -pie -fpie -I../../MVM_CHUNK/include -DSPECULATION
FULL_CKPT_FLAGS = -DSPECULATION
GRID_CKPT_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT/include -DSPECULATION
TARGET1 = intermediate-pcs1.o 
TARGET2 =  
TARGET3 = intermediate-pcs3.o 

SRCS1 = application.c 
#SRCS2 = functions-app.c 
SRCS2 =  
SRCS3 = topology.c

TA=0.4
OBJECTS=1024
LOOKAHEAD=1.0

RUN_FLAGS = -DOBJECTS=$(OBJECTS) -DLOOKAHEAD=$(LOOKAHEAD) -DTA=$(TA)

ifeq ($(BENCHMARK),1)
RUN_FLAGS += -DBENCHMARKING
endif

all: 
	$(CC) $(SRCS1) $(CFLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

chunk_ckpt:
	$(CC) $(SRCS1) $(CFLAGS) $(CHUNK_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) $(CHUNK_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

full_ckpt:
	$(CC) $(SRCS1) $(CFLAGS) $(FULL_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) $(FULL_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

grid_ckpt:
	$(CC) $(SRCS1) $(CFLAGS) $(GRID_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) $(GRID_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

clean:
	$(RM) *.o $(TARGET)

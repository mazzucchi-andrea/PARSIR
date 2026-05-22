CC = gcc 
CFLAGS = -c -Wall -Wextra -Wpedantic -I../../include/ -I./
GRID_CKPT_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT/include -DSPECULATION
GRID_CKPT_SAVE_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT_SAVE/include -DSPECULATION
CHUNK_CKPT_FLAGS = -pie -fpie -I../../MVM_CHUNK/include -DSPECULATION
FULL_CKPT_FLAGS = -DSPECULATION
MMAP_MV_FLAGS = -pie -fpie -I../../MVM_MMAP_MV/include -DSPECULATION -DMMAP_MV
TARGET = intermediate-highway.o 

SRCS = application.c

OBJECTS=1024
LOOKAHEAD=1.0

RUN_FLAGS = -DOBJECTS=$(OBJECTS) -DLOOKAHEAD=$(LOOKAHEAD)

ifeq ($(BENCHMARK),1)
RUN_FLAGS += -DBENCHMARKING
endif

all: 
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET) $(RUN_FLAGS)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o highway.o

grid_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET) 
	ld -r $(TARGET) --wrap=malloc --wrap=free -o highway.o

grid_ckpt_save:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_SAVE_FLAGS) $(RUN_FLAGS) -o $(TARGET) 
	ld -r $(TARGET) --wrap=malloc --wrap=free -o highway.o

chunk_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(CHUNK_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o highway.o

full_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(FULL_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o highway.o

mmap_mv:
	$(CC) $(SRCS) $(CFLAGS) $(MMAP_MV_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o highway.o

clean:
	$(RM) *.o $(TARGET)


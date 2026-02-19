CC = gcc 
CFLAGS = -c -Wall -Wextra -Wpedantic -I../../include/ -I./ 
CHUNK_CKPT_FLAGS = -pie -fpie -I../../MVM_CHUNK/include -DSPECULATION
FULL_CKPT_FLAGS = -DSPECULATION
GRID_CKPT_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT/include -DSPECULATION
GRID_CKPT_BS_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT_BS/include -DSPECULATION
TARGET = intermediate-phold.o 

SRCS = application.c 

M=1
OBJECTS=1024
LOOKAHEAD=1.0

RUN_FLAGS = -DOBJECTS=$(OBJECTS) -DLOOKAHEAD=$(LOOKAHEAD) -DM=$(M)

ifeq ($(BENCHMARK),1)
RUN_FLAGS += -DBENCHMARKING
endif

all: 
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET) $(RUN_FLAGS)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

chunk_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(CHUNK_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

full_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(FULL_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

grid_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_FLAGS) $(RUN_FLAGS) -o $(TARGET) 
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

grid_ckpt_bs:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_BS_FLAGS) $(RUN_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

clean:
	$(RM) *.o $(TARGET)

CC = gcc 
CFLAGS = -c -Wall -Wextra -Wpedantic -I../../include/ -I./ 
GRID_CKPT_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT/include
GRID_CKPT_BS_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT_BS/include
TARGET = intermediate-phold.o 

SRCS = application.c 

all: 
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

grid_ckpt:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

grid_ckpt_bs:
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_BS_FLAGS) -o $(TARGET)
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

clean:
	$(RM) *.o $(TARGET)

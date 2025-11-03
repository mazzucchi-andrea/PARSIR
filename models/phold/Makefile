GRID_CKPT = 0
GRID_CKPT_FLAGS = -pie -fpie -I ../../MVM/include
CC = gcc 
CFLAGS = -Wall -Wextra -I ../../include/ -I ./ -c
LINKER = -Xlinker --wrap=malloc 
TARGET = intermediate-phold.o 

SRCS = application.c 

all:
ifeq ($(GRID_CKPT), 1)
	$(CC) $(SRCS) $(CFLAGS) $(GRID_CKPT_FLAGS) -o $(TARGET) $(LINKER)
else
	$(CC) $(SRCS) $(CFLAGS) -o $(TARGET) $(LINKER)
endif
	ld -r $(TARGET) --wrap=malloc --wrap=free -o phold.o

clean:
	$(RM) *.o $(TARGET)

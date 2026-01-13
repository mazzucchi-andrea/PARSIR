CC = gcc 
CFLAGS = -c -Wall -Wextra -Wpedantic -I../../include/ -I./ -lm -Wimplicit-fallthrough
CHUNK_CKPT_FLAGS = -pie -fpie -I../../MVM_CHUNK/include
GRID_CKPT_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT/include
GRID_CKPT_BS_FLAGS = -pie -fpie -I../../MVM_GRID_CKPT_BS/include
TARGET1 = intermediate-pcs1.o 
TARGET2 =  
TARGET3 = intermediate-pcs3.o 

SRCS1 = application.c 
#SRCS2 = functions-app.c 
SRCS2 =  
SRCS3 = topology.c 

all: 
	$(CC) $(SRCS1) $(CFLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

chunk_ckpt:
	$(CC) $(SRCS1) $(CFLAGS) $(CHUNK_CKPT_FLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) $(CHUNK_CKPT_FLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

grid_ckpt:
	$(CC) $(SRCS1) $(CFLAGS) $(GRID_CKPT_FLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) $(GRID_CKPT_FLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

grid_ckpt_bs:
	$(CC) $(SRCS1) $(CFLAGS) $(GRID_CKPT_BS_FLAGS) -o $(TARGET1)
	$(CC) $(SRCS3) $(CFLAGS) $(GRID_CKPT_BS_FLAGS) -o $(TARGET3)
	ld -r $(TARGET1) $(TARGET2) $(TARGET3) --wrap=malloc --wrap=free -o pcs.o

clean:
	$(RM) *.o $(TARGET)

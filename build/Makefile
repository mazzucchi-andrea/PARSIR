MAX_MEMORY = 0x200000UL
GRID_CKPT = 0
GRID_CKPT_FLAGS = -pie -fPIE -fstack-protector-all
MOD = 64

all: runtime
	@ echo "The runtime environment of PARSIR has been compiled - object files are available in the lib directory"

runtime:
	cd ../scripts ; make get-HW-info
	
ifeq ($(GRID_CKPT), 1)
	gcc -c -O3 ../queue/queue.c $(GRID_CKPT_FLAGS) -o ../lib/queue.o -I ../include
	gcc -c ../queue/barrier-synchronization.c $(GRID_CKPT_FLAGS) -o ../lib/barrier-synchronization.o -I ../include
	gcc -c -O3 ../stubs/stubs.c $(GRID_CKPT_FLAGS) -o ../lib/stubs.o -I../include 
	gcc -c -O3 ../random/random.c $(GRID_CKPT_FLAGS) -o ../lib/random.o -I../include
	gcc -c -O3 ../memory/memory.c $(GRID_CKPT_FLAGS) -o ../lib/memory.o -I ../include -DGRID_CKPT
	gcc -c -O3 ../engine/engine.c $(GRID_CKPT_FLAGS) -o ../lib/engine.o -I ../include -DGRID_CKPT
	gcc -c -O3 ../grid_ckpt/grid_ckpt.c $(GRID_CKPT_FLAGS) -mavx -mavx2 -mavx512f -o ../lib/grid_ckpt.o -I ../include -DMOD=$(MOD)
else
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I ../include
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I ../include
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include 
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I ../include
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include
endif

ifeq ($(GRID_CKPT), 1)
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/grid_ckpt.o -I../include -o ../lib/PARSIR-runtime.o
else
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o -I../include -o ../lib/PARSIR-runtime.o
endif 

phold: runtime
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
ifeq ($(GRID_CKPT), 1)
	cd ../MVM ; make checks backup-files file-rewriting ALLOCATOR_AREA_SIZE=$(MAX_MEMORY)
	cd ../models/phold ; make GRID_CKPT=$(GRID_CKPT)		
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I ../MVM/include -I ../include -o ../bin/PARSIR-simulator.o
	cd ../MVM ; make compile-and-link restore-files ALLOCATOR_AREA_SIZE=$(MAX_MEMORY)
	@echo MVM CKPT applied to PARSIR
else
	cd ../models/phold ; make
	gcc -o ../bin/PARSIR-simulator ../lib/PARSIR-runtime.o -I ../include -lpthread -lnuma ../models/phold/phold.o
endif

clean:
	find .. -name "*.o" -type f -delete ; rm -f ../bin/PARSIR-simulator
MAX_MEMORY = 0x200000UL
MOD = 64
GRID_CKPT_FLAGS = -pie -fPIE -fstack-protector-all -DGRID_CKPT -DMOD=$(MOD) -DMAX_MEMORY=$(MAX_MEMORY)

all: runtime
	@ echo "The runtime environment of PARSIR has been compiled - object files are available in the lib directory"

runtime:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include 
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include 
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include 
	ld  -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o -I../include -o ../lib/PARSIR-runtime.o

phold: runtime
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../models/phold ; make
	gcc -o ../bin/PARSIR-simulator ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -lpthread -lnuma

runtime_grid_ckpt:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include -DGRID_CKPT -DMOD=$(MOD)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 -Wextra -Wall -Wpedantic -I../include $(GRID_CKPT_FLAGS) -mavx -mavx2 -mavx512f ../grid_ckpt/grid_ckpt.c -o ../lib/grid_ckpt.o
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/grid_ckpt.o -I../include -o ../lib/PARSIR-runtime.o

phold_grid_ckpt: runtime_grid_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM ; make checks backup-files file-rewriting
	cd ../models/phold ; make grid_ckpt
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM/include -o ../bin/PARSIR-simulator.o
	cd ../MVM ; make compile-and-link restore-files MOD=$(MOD) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY)
	@echo MVM CKPT applied to PARSIR

restore-files:
	cd ../MVM ; make restore-files

clean:
	find .. -name "*.o" -type f -delete ; rm -f ../bin/PARSIR-simulator ; cd ../MVM ; make clean
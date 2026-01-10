MAX_MEMORY = 0x200000UL
MOD = 8
GRID_CKPT_FLAGS = -pie -fPIE -fstack-protector-all -DGRID_CKPT -DMOD=$(MOD) -DSPECULATION
CHUNK_CKPT_FLAGS = -pie -fPIE -fstack-protector-all -DSPECULATION -DCHUNK_BASED

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

runtime_grid_ckpt:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include  $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../grid_ckpt/grid_ckpt.c -o ../lib/grid_ckpt.o -I../include -I../MVM_GRID_CKPT/include -DMAX_MEMORY=$(MAX_MEMORY) $(GRID_CKPT_FLAGS)
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/grid_ckpt.o -I../include -o ../lib/PARSIR-runtime.o

runtime_grid_ckpt_bs:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(GRID_CKPT_FLAGS)
	gcc -c -O3 ../grid_ckpt/grid_ckpt.c -o ../lib/grid_ckpt.o -I../include -I../MVM_GRID_CKPT_BS/include -DMAX_MEMORY=$(MAX_MEMORY) $(GRID_CKPT_FLAGS)
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/grid_ckpt.o -I../include -I../MVM_GRID_CKPT_BS/include -o ../lib/PARSIR-runtime.o

runtime_chunk_ckpt:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(CHUNK_CKPT_FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(CHUNK_CKPT_FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include $(CHUNK_CKPT_FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(CHUNK_CKPT_FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(CHUNK_CKPT_FLAGS)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(CHUNK_CKPT_FLAGS)
	gcc -c -O3 ../chunk_ckpt/chunk_ckpt.c -o ../lib/chunk_ckpt.o -I../include $(CHUNK_CKPT_FLAGS)
	ld  -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/chunk_ckpt.o -I../include -o ../lib/PARSIR-runtime.o

phold: runtime
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../models/phold ; make
	gcc -o ../bin/PARSIR-simulator ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -lpthread -lnuma

phold_grid_ckpt: runtime_grid_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_GRID_CKPT ; make checks backup-files file-rewriting
	cd ../models/phold ; make grid_ckpt
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM_GRID_CKPT/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_GRID_CKPT ; make compile-and-link restore-files MOD=$(MOD) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY)
	@echo MVM_GRID_CKPT CKPT applied to PARSIR

phold_grid_ckpt_bs: runtime_grid_ckpt_bs
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_GRID_CKPT_BS ; make checks backup-files file-rewriting
	cd ../models/phold ; make grid_ckpt_bs
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM_GRID_CKPT_BS/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_GRID_CKPT_BS ; make compile-and-link restore-files MOD=$(MOD) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY)
	@echo MVM_GRID_CKPT_BS applied to PARSIR

phold_chunk_ckpt: runtime_chunk_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_CHUNK ; make checks backup-files file-rewriting
	cd ../models/phold ; make chunk_ckpt
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM_CHUNK/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_CHUNK ; make compile-and-link restore-files
	@echo MVM_CHUNK applied to PARSIR

restore-files:
	cd ../MVM_GRID_CKPT_BS ; make restore-files ; cd ../MVM_GRID_CKPT ; make restore-files ; cd ../MVM_CHUNK ; make restore-files

clean:
	find .. -name "*.o" -type f -delete ; rm -f ../bin/PARSIR-simulator ; cd ../MVM_GRID_CKPT ; make clean ; cd ../MVM_GRID_CKPT_BS ; make clean ; cd ../MVM_CHUNK ; make clean
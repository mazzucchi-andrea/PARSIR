# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

MAX_MEMORY = 0x200000UL
MOD = 8
GRID_CKPT_FLAGS = -pie -fPIE -fstack-protector-all -DGRID_CKPT -DMOD=$(MOD) -DSPECULATION

CHUNK_CKPT_FLAGS = -pie -fPIE -fstack-protector-all -DSPECULATION -DCHUNK_BASED -DCHUNK_BASED_SAVE

CHUNK_CKPT_FULL_FLAGS = -DSPECULATION -DCHUNK_BASED -DFULL_CKPT

MMAP_MV_RT_FLAGS = -pie -fPIE -fstack-protector-all -DSPECULATION -DMMAP_MV
MMAP_MV_MODEL_FLAGS = -pie -fpie -I../MVM_MMAP_MV/include -DSPECULATION -DMMAP_MV

MMAP_MV_PAGE_SIZE ?= 4096
MMAP_MV_BACKEND_FLAGS = -DMVMM_PAGE_SIZE=$(MMAP_MV_PAGE_SIZE)

PHOLD_TARGET_FUNCTIONS = \""<add1>:","<dell1>:","<add2>:","<dell2>:","<read1>:","<read2>:","<update1>:","<update2>:","<ProcessEvent>:"\"
PHOLD_TARGET_MODULES = ../models/phold/application.c

PCS_TARGET_FUNCTIONS = \""<ProcessEvent>:","<allocation>:","<deallocation>:","<fading_recheck>:"\"
PCS_TARGET_MODULES = ../models/pcs/application.c
PCS_LIBS = "-lm -lpthread -lnuma"

#run
THREADS=8
OBJECTS=1024
LOOKAHEAD=1.0

#Benchmarking
WARMUP=10
DURATION=60

#phold
M=1
P_SHIFT=5 

#pcs
MIT=0.4

FLAGS = -DTHREADS=$(THREADS) -DOBJECTS=$(OBJECTS) -DLOOKAHEAD=$(LOOKAHEAD)
PHOLD_FLAGS = OBJECTS=$(OBJECTS) LOOKAHEAD=$(LOOKAHEAD) M=$(M) P_SHIFT=$(P_SHIFT)
PCS_FLAGS = OBJECTS=$(OBJECTS) LOOKAHEAD=$(LOOKAHEAD) TA=$(MIT)

ifeq ($(DEBUG),1)
FLAGS += -g -DDEBUG
PHOLD_FLAGS += DEBUG=1
PCS_FLAGS += DEBUG=1
endif

ifeq ($(BENCHMARK),1)
FLAGS += -DBENCHMARKING -DWARMUP=$(WARMUP) -DDURATION=$(DURATION)
PHOLD_FLAGS += BENCHMARK=1
PCS_FLAGS += BENCHMARK=1
endif

ifeq ($(AVOID_THROTTLING),1)
FLAGS += -DAVOID_THROTTLING
endif

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
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include  $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../grid_ckpt/grid_ckpt.c -o ../lib/grid_ckpt.o -I../include -I../MVM_GRID_CKPT/include -DMAX_MEMORY=$(MAX_MEMORY) $(GRID_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../speculation/speculation.c -o ../lib/speculation.o -I../include -pthread $(GRID_CKPT_FLAGS) $(FLAGS)
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/grid_ckpt.o ../lib/speculation.o -I../include -o ../lib/PARSIR-runtime.o

runtime_chunk_ckpt:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(CHUNK_CKPT_FLAGS) $(FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(CHUNK_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include $(CHUNK_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(CHUNK_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(CHUNK_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(CHUNK_CKPT_FLAGS) $(FLAGS)
	gcc -c -O3 ../chunk_ckpt/chunk_ckpt.c -o ../lib/chunk_ckpt.o -I../include $(CHUNK_CKPT_FLAGS) -DMAX_MEMORY=$(MAX_MEMORY) $(FLAGS)
	gcc -c -O3 ../speculation/speculation.c -o ../lib/speculation.o -I../include -pthread $(CHUNK_CKPT_FLAGS) $(FLAGS)
	ld  -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/chunk_ckpt.o ../lib/speculation.o -I../include -o ../lib/PARSIR-runtime.o

runtime_full_ckpt:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS) 
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS) 
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS) 
	gcc -c -O3 ../chunk_ckpt/chunk_ckpt.c -o ../lib/chunk_ckpt.o -I../include $(CHUNK_CKPT_FULL_FLAGS) -DMAX_MEMORY=$(MAX_MEMORY) $(FLAGS)
	gcc -c -O3 ../speculation/speculation.c -o ../lib/speculation.o -I../include -pthread $(CHUNK_CKPT_FULL_FLAGS) $(FLAGS)
	ld  -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/chunk_ckpt.o ../lib/speculation.o -I../include -o ../lib/PARSIR-runtime.o

runtime_mmap_mv:
	cd ../scripts ; make get-HW-info
	@if [ ! -d ../lib ]; then mkdir -p ../lib; fi
	gcc -c -O3 ../queue/queue.c -o ../lib/queue.o -I../include $(MMAP_MV_RT_FLAGS) $(FLAGS)
	gcc -c ../queue/barrier-synchronization.c -o ../lib/barrier-synchronization.o -I../include $(MMAP_MV_RT_FLAGS) $(FLAGS)
	gcc -c -O3 ../memory/memory.c -o ../lib/memory.o -I../include $(MMAP_MV_RT_FLAGS) $(FLAGS)
	gcc -c -O3 ../stubs/stubs.c -o ../lib/stubs.o -I../include $(MMAP_MV_RT_FLAGS) $(FLAGS)
	gcc -c -O3 ../random/random.c -o ../lib/random.o -I../include $(MMAP_MV_RT_FLAGS) $(FLAGS)
	gcc -c -O3 ../engine/engine.c -o ../lib/engine.o -I../include $(MMAP_MV_RT_FLAGS) $(FLAGS)
	gcc -c -O3 ../speculation/speculation.c -o ../lib/speculation.o -I../include -pthread $(MMAP_MV_RT_FLAGS) $(FLAGS)
	ld -r ../lib/queue.o ../lib/memory.o ../lib/barrier-synchronization.o ../lib/stubs.o ../lib/random.o ../lib/engine.o ../lib/speculation.o -I../include -o ../lib/PARSIR-runtime.o


phold: runtime
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../models/phold ; make $(PHOLD_FLAGS)
	gcc -o ../bin/PARSIR-simulator ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -lpthread -lnuma

pcs: runtime
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../models/pcs ; make $(PCS_FLAGS)
	gcc -o ../bin/PARSIR-simulator ../lib/PARSIR-runtime.o -I../include -lpthread -lnuma ../models/pcs/pcs.o -lm

phold_grid_ckpt: runtime_grid_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_GRID_CKPT ; make checks backup-files file-rewriting TARGET_MODULES=$(PHOLD_TARGET_MODULES) TARGET_FUNCTIONS=$(PHOLD_TARGET_FUNCTIONS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	cd ../models/phold ; make grid_ckpt $(PHOLD_FLAGS)
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM_GRID_CKPT/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_GRID_CKPT ; make compile-and-link restore-files MOD=$(MOD) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) TARGET_MODULES=$(PHOLD_TARGET_MODULES) TARGET_FUNCTIONS=$(PHOLD_TARGET_FUNCTIONS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	@echo MVM_GRID_CKPT applied to PARSIR

pcs_grid_ckpt: runtime_grid_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_GRID_CKPT ; make checks backup-files file-rewriting TARGET_MODULES=$(PCS_TARGET_MODULES) TARGET_FUNCTIONS=$(PCS_TARGET_FUNCTIONS) LIBS=$(PCS_LIBS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	cd ../models/pcs ; make grid_ckpt $(PCS_FLAGS)
	ld -r ../models/pcs/pcs.o ../lib/PARSIR-runtime.o -I../include -I../MVM_GRID_CKPT/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_GRID_CKPT ; make compile-and-link restore-files MOD=$(MOD) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) TARGET_MODULES=$(PCS_TARGET_MODULES) TARGET_FUNCTIONS=$(PCS_TARGET_FUNCTIONS) LIBS=$(PCS_LIBS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	@echo MVM_GRID_CKPT applied to PARSIR


phold_chunk_ckpt: runtime_chunk_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_CHUNK ; make checks backup-files file-rewriting TARGET_MODULES=$(PHOLD_TARGET_MODULES) TARGET_FUNCTIONS=$(PHOLD_TARGET_FUNCTIONS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	cd ../models/phold ; make chunk_ckpt $(PHOLD_FLAGS)
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM_CHUNK/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_CHUNK ; make compile-and-link restore-files TARGET_MODULES=$(PHOLD_TARGET_MODULES) TARGET_FUNCTIONS=$(PHOLD_TARGET_FUNCTIONS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	@echo MVM_CHUNK applied to PARSIR

pcs_chunk_ckpt: runtime_chunk_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_CHUNK ; make checks backup-files file-rewriting TARGET_MODULES=$(PCS_TARGET_MODULES) TARGET_FUNCTIONS=$(PCS_TARGET_FUNCTIONS) LIBS=$(PCS_LIBS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	cd ../models/pcs ; make chunk_ckpt $(PCS_FLAGS)
	ld -r ../models/pcs/pcs.o ../lib/PARSIR-runtime.o -I../include -I../MVM_CHUNK/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_CHUNK ; make compile-and-link restore-files TARGET_MODULES=$(PCS_TARGET_MODULES) TARGET_FUNCTIONS=$(PCS_TARGET_FUNCTIONS) LIBS=$(PCS_LIBS) ALLOCATOR_AREA_SIZE=$(MAX_MEMORY) MOD=$(MOD)
	@echo MVM_CHUNK applied to PARSIR

phold_full_ckpt: runtime_full_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../models/phold ; make full_ckpt $(PHOLD_FLAGS)
	gcc -o ../bin/PARSIR-simulator ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -lpthread -lnuma

pcs_full_ckpt: runtime_full_ckpt
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../models/pcs ; make full_ckpt $(PCS_FLAGS)
	gcc -o ../bin/PARSIR-simulator ../models/pcs/pcs.o ../lib/PARSIR-runtime.o -I../include -lm -lpthread -lnuma

phold_mmap_mv: runtime_mmap_mv
	@if [ ! -d ../bin ]; then mkdir -p ../bin; fi
	cd ../MVM_MMAP_MV ; make checks backup-files file-rewriting TARGET_MODULES=$(PHOLD_TARGET_MODULES) TARGET_FUNCTIONS=$(PHOLD_TARGET_FUNCTIONS) OBJECTS=$(OBJECTS) EXTRA_FLAGS="$(MMAP_MV_BACKEND_FLAGS)"
	cd ../models/phold ; make mmap_mv $(PHOLD_FLAGS)
	ld -r ../models/phold/phold.o ../lib/PARSIR-runtime.o -I../include -I../MVM_MMAP_MV/include -o ../bin/PARSIR-simulator.o
	cd ../MVM_MMAP_MV ; make compile-and-link restore-files TARGET_MODULES=$(PHOLD_TARGET_MODULES) TARGET_FUNCTIONS=$(PHOLD_TARGET_FUNCTIONS) OBJECTS=$(OBJECTS) EXTRA_FLAGS="$(MMAP_MV_BACKEND_FLAGS)"
	@echo MVM_MMAP_MV base applied to PARSIR 

clean:
	find .. -name "*.o" -type f -delete ; rm -f ../bin/PARSIR-simulator ; cd ../MVM_GRID_CKPT ; make clean ; cd ../MVM_CHUNK ; make clean

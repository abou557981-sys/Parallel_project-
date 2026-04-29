CC ?= gcc
MPICC ?= mpicc
NVCC ?= nvcc
CFLAGS ?= -O3 -std=c11 -Wall -Wextra -Iinclude
LDFLAGS ?= -lm

SRC_COMMON = src/mandelbrot_common.c

.PHONY: all clean dirs cpu mpi cuda

all: dirs cpu mpi cuda

dirs:
	mkdir -p bin results

cpu: dirs bin/mandelbrot_seq bin/mandelbrot_omp bin/mandelbrot_pthreads

mpi: dirs bin/mandelbrot_mpi

cuda: dirs bin/mandelbrot_cuda

bin/mandelbrot_seq: src/mandelbrot_seq.c $(SRC_COMMON) include/mandelbrot_common.h
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

bin/mandelbrot_omp: src/mandelbrot_omp.c $(SRC_COMMON) include/mandelbrot_common.h
	$(CC) $(CFLAGS) -fopenmp $^ -o $@ $(LDFLAGS)

bin/mandelbrot_pthreads: src/mandelbrot_pthreads.c $(SRC_COMMON) include/mandelbrot_common.h
	$(CC) $(CFLAGS) $^ -o $@ -lpthread $(LDFLAGS)

bin/mandelbrot_mpi: src/mandelbrot_mpi.c $(SRC_COMMON) include/mandelbrot_common.h
	$(MPICC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

bin/mandelbrot_cuda: src/mandelbrot_cuda.cu src/mandelbrot_common.c include/mandelbrot_common.h
	$(NVCC) -O3 -Iinclude src/mandelbrot_cuda.cu src/mandelbrot_common.c -o $@

clean:
	rm -rf bin results/*.ppm

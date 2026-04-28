# Parallel Mandelbrot Project

This repository now includes **full working implementations** of Mandelbrot generation using:

- Sequential C (baseline)
- OpenMP
- POSIX Threads (pthreads)
- MPI
- CUDA

Each variant computes Mandelbrot iteration depth per pixel and writes a colorized **PPM image**.

## Project Layout

- `include/mandelbrot_common.h`: shared config/types/util API
- `src/mandelbrot_common.c`: shared Mandelbrot math + color mapping + PPM output
- `src/mandelbrot_seq.c`: sequential baseline
- `src/mandelbrot_omp.c`: OpenMP parallel loops
- `src/mandelbrot_pthreads.c`: pthread worker partitioning
- `src/mandelbrot_mpi.c`: MPI row distribution + gather
- `src/mandelbrot_cuda.cu`: CUDA kernel implementation
- `scripts/benchmark.sh`: one-shot CPU+MPI benchmark runner
- `Makefile`: builds all targets into `bin/`

## Build

```bash
make            # builds cpu + mpi + cuda targets
```

If CUDA is not installed on your machine, build only CPU/MPI:

```bash
make cpu mpi
```

## Run

### Sequential

```bash
./bin/mandelbrot_seq --width 1920 --height 1080 --max-iter 1000 --output results/mandelbrot_seq.ppm
```

### OpenMP

```bash
OMP_NUM_THREADS=8 ./bin/mandelbrot_omp --width 1920 --height 1080 --max-iter 1000 --output results/mandelbrot_omp.ppm
```

### Pthreads

```bash
./bin/mandelbrot_pthreads --threads 8 --width 1920 --height 1080 --max-iter 1000 --output results/mandelbrot_pthreads.ppm
```

### MPI

```bash
mpirun -np 4 ./bin/mandelbrot_mpi --width 1920 --height 1080 --max-iter 1000 --output results/mandelbrot_mpi.ppm
```

### CUDA

```bash
./bin/mandelbrot_cuda --width 1920 --height 1080 --max-iter 1000 --output results/mandelbrot_cuda.ppm
```

## Benchmark Helper

```bash
scripts/benchmark.sh
```

Environment overrides:

- `WIDTH` (default `1920`)
- `HEIGHT` (default `1080`)
- `MAX_ITER` (default `1000`)
- `THREADS` (default `8`)
- `MPI_RANKS` (default `4`)

## Notes

- Output images are PPM (`P6`) for minimal dependencies and fast writing.
- Use ImageMagick to convert to PNG if desired:

```bash
convert results/mandelbrot_seq.ppm results/mandelbrot_seq.png
```


## Google Colab (CUDA)

A ready-to-run Colab setup is included in `colab/`:

- `colab/run_in_colab.sh`
- `colab/display_ppm.py`
- `colab/mandelbrot_colab.ipynb`

See `colab/README.md` for step-by-step usage.

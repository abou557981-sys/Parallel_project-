#!/usr/bin/env bash
set -euo pipefail

mkdir -p results

WIDTH="${WIDTH:-1920}"
HEIGHT="${HEIGHT:-1080}"
MAX_ITER="${MAX_ITER:-1000}"
THREADS="${THREADS:-8}"
MPI_RANKS="${MPI_RANKS:-4}"

echo "Building targets..."
make cpu mpi

echo "Running sequential"
./bin/mandelbrot_seq --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" --output results/mandelbrot_seq.ppm

echo "Running OpenMP"
OMP_NUM_THREADS="$THREADS" ./bin/mandelbrot_omp --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" --output results/mandelbrot_omp.ppm

echo "Running pthreads"
./bin/mandelbrot_pthreads --threads "$THREADS" --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" --output results/mandelbrot_pthreads.ppm

echo "Running MPI"
mpirun -np "$MPI_RANKS" ./bin/mandelbrot_mpi --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" --output results/mandelbrot_mpi.ppm

echo "Done. Outputs written to results/*.ppm"

#!/usr/bin/env bash
set -euo pipefail

WIDTH="${WIDTH:-1280}"
HEIGHT="${HEIGHT:-720}"
MAX_ITER="${MAX_ITER:-700}"
THREADS="${THREADS:-2,4}"
MPI_RANKS="${MPI_RANKS:-2}"
TRIALS="${TRIALS:-2}"

mkdir -p results

echo "[INFO] Ensuring Python plotting dependency"
python3 -m pip -q install -r analysis/requirements.txt

echo "[INFO] Building CPU binaries"
make cpu

echo "[INFO] Running chart pipeline for seq/openmp/pthreads"
python3 analysis/generate_advanced_charts.py \
  --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" \
  --trials "$TRIALS" --threads "$THREADS" --output-dir results/charts_cpu

if command -v mpicc >/dev/null 2>&1 && command -v mpirun >/dev/null 2>&1; then
  echo "[INFO] MPI detected: building and running MPI-inclusive charts"
  make mpi
  python3 analysis/generate_advanced_charts.py \
    --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" \
    --trials "$TRIALS" --threads "$THREADS" \
    --include-mpi --mpi-ranks "$MPI_RANKS" --output-dir results/charts_cpu_mpi
else
  echo "[WARN] MPI toolchain not found. Skipping MPI runs."
fi

if command -v nvidia-smi >/dev/null 2>&1 && command -v nvcc >/dev/null 2>&1; then
  echo "[INFO] GPU + nvcc detected"
  nvidia-smi || true
  nvcc --version || true

  echo "[INFO] Building and running CUDA"
  make cuda
  ./bin/mandelbrot_cuda --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" --output results/mandelbrot_cuda_colab.ppm

  echo "[INFO] Converting CUDA output to PNG"
  python3 colab/display_ppm.py --input results/mandelbrot_cuda_colab.ppm --output results/mandelbrot_cuda_colab.png
else
  echo "[WARN] CUDA toolchain or GPU runtime not found. Skipping CUDA run."
fi

echo "[INFO] Completed all available backends for this Colab runtime."
echo "[INFO] Download bundles: results/charts_cpu.zip and (if MPI run) results/charts_cpu_mpi.zip"

#!/usr/bin/env bash
set -euo pipefail

# Tunable parameters (can be overridden in Colab cell)
WIDTH="${WIDTH:-1920}"
HEIGHT="${HEIGHT:-1080}"
MAX_ITER="${MAX_ITER:-1000}"
OUTPUT="${OUTPUT:-results/mandelbrot_cuda_colab.ppm}"

if ! command -v nvidia-smi >/dev/null 2>&1; then
  echo "[ERROR] No NVIDIA GPU runtime detected."
  echo "In Colab: Runtime -> Change runtime type -> Hardware accelerator -> GPU"
  exit 1
fi

if ! command -v nvcc >/dev/null 2>&1; then
  echo "[ERROR] nvcc not found."
  echo "In Colab: switch to a CUDA-enabled runtime image and restart runtime."
  exit 1
fi

echo "[INFO] GPU"
nvidia-smi

echo "[INFO] CUDA compiler"
nvcc --version

echo "[INFO] Building CUDA target"
make cuda

echo "[INFO] Running CUDA Mandelbrot"
./bin/mandelbrot_cuda --width "$WIDTH" --height "$HEIGHT" --max-iter "$MAX_ITER" --output "$OUTPUT"

echo "[INFO] Done -> $OUTPUT"

#!/usr/bin/env python3
from pathlib import Path
from PIL import Image
import argparse

parser = argparse.ArgumentParser(description="Convert and preview PPM output in Colab")
parser.add_argument("--input", default="results/mandelbrot_cuda_colab.ppm")
parser.add_argument("--output", default="results/mandelbrot_cuda_colab.png")
args = parser.parse_args()

src = Path(args.input)
dst = Path(args.output)

if not src.exists():
    raise SystemExit(f"Input does not exist: {src}")

img = Image.open(src)
img.save(dst)
print(f"Saved: {dst}")

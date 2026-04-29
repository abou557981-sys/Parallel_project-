# Google Colab CUDA Folder

This folder is designed so you can run the CUDA Mandelbrot implementation directly in **Google Colab**.

## 1) In Colab: enable GPU runtime

- Go to **Runtime → Change runtime type**
- Set **Hardware accelerator = GPU**

## 2) Clone/upload this repo in Colab

Example in a Colab code cell:

```bash
!git clone <your-repo-url>
%cd Parallel_project-
```

## 3) Run the CUDA pipeline

```bash
!bash colab/run_in_colab.sh
```

Optional custom resolution/iterations:

```bash
!WIDTH=2560 HEIGHT=1440 MAX_ITER=1500 OUTPUT=results/mandelbrot_2k.ppm bash colab/run_in_colab.sh
```

## 4) Convert output to PNG for inline display

```bash
!pip -q install pillow
!python colab/display_ppm.py --input results/mandelbrot_cuda_colab.ppm --output results/mandelbrot_cuda_colab.png
```

Then display in Colab:

```python
from IPython.display import Image, display
display(Image('results/mandelbrot_cuda_colab.png'))
```

## Files in this folder

- `run_in_colab.sh` — checks GPU/CUDA toolchain, builds, and runs CUDA executable.
- `display_ppm.py` — converts PPM output to PNG for easy preview.

## Troubleshooting

If you see linker errors like:

- `undefined reference to default_config()`
- `undefined reference to write_ppm(...)`

make sure you are on the latest commit. That error came from missing C/C++ linkage guards in the shared header when CUDA (`.cu`) code linked against C utilities. The current version includes the fix (`extern "C"` guards in `include/mandelbrot_common.h`).


## Run all backends in Colab (recommended)

Use this one command to run everything available in your Colab runtime:

```bash
!bash colab/run_all_in_colab.sh
```

What it does:
- builds and runs CPU backends (sequential, OpenMP, pthreads)
- generates advanced charts + ZIP bundle (`results/charts_cpu.zip`)
- runs MPI too if `mpicc`/`mpirun` exist
- builds/runs CUDA if GPU + `nvcc` are present
- writes CUDA image (`results/mandelbrot_cuda_colab.png`)

- If OpenMPI warns about running as root in Colab, this workflow already handles it automatically (`--allow-run-as-root` + `OMPI_ALLOW_RUN_AS_ROOT*` env vars).
- If OpenMPI reports insufficient slots, the benchmark runner already uses `--oversubscribe` automatically.

Optional tuning:

```bash
!WIDTH=1920 HEIGHT=1080 MAX_ITER=1000 THREADS=2,4,8 MPI_RANKS=2,4 TRIALS=3 bash colab/run_all_in_colab.sh
```

Download outputs in Colab:

```python
from google.colab import files
files.download('results/charts_cpu.zip')
# Optional:
# files.download('results/charts_cpu_mpi.zip')
# files.download('results/mandelbrot_cuda_colab.png')
```

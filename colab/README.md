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

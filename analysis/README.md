# Advanced Performance Charts

This folder generates **high-detail, publication-ready benchmark charts** and bundles them for download.

## What it creates

Running `analysis/generate_advanced_charts.py` produces:

- `results/charts/benchmark_results.csv` (raw statistics)
- `results/charts/mandelbrot_advanced_dashboard.png` (quick view)
- `results/charts/mandelbrot_advanced_dashboard.pdf` (high-quality report asset)
- `results/charts/mandelbrot_advanced_dashboard.svg` (editable vector)
- `results/charts/REPORT.md` (text summary)
- `results/charts.zip` (**download this**)

## Usage

```bash
make cpu
python3 analysis/generate_advanced_charts.py --width 1920 --height 1080 --max-iter 1000 --trials 5 --threads 1,2,4,8
```

Optional MPI comparison:

```bash
make cpu mpi
python3 analysis/generate_advanced_charts.py --include-mpi --mpi-ranks 2,4,8
```

Use existing CSV only:

```bash
python3 analysis/generate_advanced_charts.py --input-csv results/charts/benchmark_results.csv
```

## Google Colab download tip

After generation in Colab:

```python
from google.colab import files
files.download('results/charts.zip')
```

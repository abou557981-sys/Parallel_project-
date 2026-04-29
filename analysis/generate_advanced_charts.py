#!/usr/bin/env python3
import argparse
import csv
import math
import os
import re
import statistics
import subprocess
import sys
import zipfile
from dataclasses import dataclass
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    from matplotlib.gridspec import GridSpec
except ModuleNotFoundError as exc:
    raise SystemExit("matplotlib is required. Install with: pip install matplotlib") from exc

TIME_RE = re.compile(r"time=([0-9]*\.?[0-9]+) s")


@dataclass
class RunConfig:
    width: int
    height: int
    max_iter: int
    trials: int
    threads: list[int]
    include_mpi: bool
    mpi_ranks: list[int]


def run_command(cmd: list[str], env: dict | None = None) -> float:
    proc = subprocess.run(cmd, capture_output=True, text=True, env=env)
    output = (proc.stdout or "") + "\n" + (proc.stderr or "")
    if proc.returncode != 0:
        raise RuntimeError(f"Command failed: {' '.join(cmd)}\n{output}")

    match = TIME_RE.search(output)
    if not match:
        raise RuntimeError(f"Could not parse timing from output:\n{output}")
    return float(match.group(1))


def summarize(values: list[float]) -> tuple[float, float, float, float]:
    mean = statistics.mean(values)
    median = statistics.median(values)
    stdev = statistics.pstdev(values) if len(values) > 1 else 0.0
    ci95 = 1.96 * stdev / math.sqrt(len(values)) if len(values) > 1 else 0.0
    return mean, median, stdev, ci95


def benchmark(cfg: RunConfig, out_csv: Path) -> list[dict]:
    rows = []
    binaries = [
        ("seq", ["./bin/mandelbrot_seq"]),
        ("openmp", ["./bin/mandelbrot_omp"]),
        ("pthreads", ["./bin/mandelbrot_pthreads"]),
    ]

    if cfg.include_mpi:
        binaries.append(("mpi", ["mpirun", "-np", "{ranks}", "./bin/mandelbrot_mpi"]))

    seq_baseline = None

    for name, base_cmd in binaries:
        param_values = [1] if name == "seq" else (cfg.mpi_ranks if name == "mpi" else cfg.threads)
        param_label = "ranks" if name == "mpi" else "threads"

        for pval in param_values:
            times = []
            for t in range(cfg.trials):
                output = f"results/{name}_{pval}_trial{t+1}.ppm"
                cmd = []
                for c in base_cmd:
                    if c == "{ranks}":
                        cmd.append(str(pval))
                    else:
                        cmd.append(c)
                cmd += [
                    "--width", str(cfg.width),
                    "--height", str(cfg.height),
                    "--max-iter", str(cfg.max_iter),
                    "--output", output,
                ]
                env = os.environ.copy()
                if name == "openmp":
                    env["OMP_NUM_THREADS"] = str(pval)
                if name == "pthreads":
                    cmd.extend(["--threads", str(pval)])

                times.append(run_command(cmd, env=env))

            mean, median, stdev, ci95 = summarize(times)
            if name == "seq":
                seq_baseline = mean
            speedup = seq_baseline / mean if seq_baseline else 1.0
            efficiency = speedup / pval if pval else 1.0

            row = {
                "impl": name,
                param_label: pval,
                "width": cfg.width,
                "height": cfg.height,
                "max_iter": cfg.max_iter,
                "trials": cfg.trials,
                "mean_time_s": mean,
                "median_time_s": median,
                "stdev_time_s": stdev,
                "ci95_time_s": ci95,
                "min_time_s": min(times),
                "max_time_s": max(times),
                "speedup_vs_seq": speedup,
                "efficiency": efficiency,
            }
            rows.append(row)

    out_csv.parent.mkdir(parents=True, exist_ok=True)
    fields = sorted(set().union(*[r.keys() for r in rows]))
    with out_csv.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)

    return rows


def load_rows(path: Path) -> list[dict]:
    with path.open() as f:
        return list(csv.DictReader(f))


def to_float(row: dict, key: str, default: float = 0.0) -> float:
    v = row.get(key, "")
    return float(v) if v not in ("", None) else default


def plot(rows: list[dict], out_dir: Path, title: str) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    plt.style.use("seaborn-v0_8-whitegrid")

    seq_time = next(to_float(r, "mean_time_s") for r in rows if r["impl"] == "seq")
    cpu_rows = [r for r in rows if r["impl"] in {"openmp", "pthreads", "seq"}]

    fig = plt.figure(figsize=(18, 11), constrained_layout=True)
    gs = GridSpec(2, 3, figure=fig)

    ax1 = fig.add_subplot(gs[0, 0])
    impls = [r["impl"] for r in cpu_rows]
    means = [to_float(r, "mean_time_s") for r in cpu_rows]
    ci95 = [to_float(r, "ci95_time_s") for r in cpu_rows]
    ax1.bar(impls, means, yerr=ci95, capsize=5, color=["#3b82f6", "#22c55e", "#a855f7"])
    ax1.set_title("Mean Runtime with 95% CI")
    ax1.set_ylabel("Seconds")

    ax2 = fig.add_subplot(gs[0, 1])
    for impl, marker, color in [("openmp", "o", "#16a34a"), ("pthreads", "s", "#9333ea")]:
        subset = sorted([r for r in rows if r["impl"] == impl], key=lambda x: int(x.get("threads", 1)))
        xs = [int(r.get("threads", 1)) for r in subset]
        ys = [to_float(r, "speedup_vs_seq") for r in subset]
        ax2.plot(xs, ys, marker=marker, label=impl, color=color, linewidth=2)
    max_threads = max([int(r.get("threads", 1) or 1) for r in rows])
    ax2.plot([1, max_threads], [1, max_threads], "k--", alpha=0.6, label="Ideal linear")
    ax2.set_title("Speedup vs Sequential")
    ax2.set_xlabel("Threads")
    ax2.set_ylabel("Speedup")
    ax2.legend()

    ax3 = fig.add_subplot(gs[0, 2])
    for impl, marker, color in [("openmp", "o", "#16a34a"), ("pthreads", "s", "#9333ea")]:
        subset = sorted([r for r in rows if r["impl"] == impl], key=lambda x: int(x.get("threads", 1)))
        xs = [int(r.get("threads", 1)) for r in subset]
        ys = [to_float(r, "efficiency") * 100 for r in subset]
        ax3.plot(xs, ys, marker=marker, label=impl, color=color, linewidth=2)
    ax3.axhline(100, linestyle="--", color="black", alpha=0.5)
    ax3.set_title("Parallel Efficiency")
    ax3.set_xlabel("Threads")
    ax3.set_ylabel("Efficiency (%)")
    ax3.legend()

    ax4 = fig.add_subplot(gs[1, :2])
    table_data = []
    for r in rows:
        p = r.get("threads") or r.get("ranks") or "1"
        table_data.append([
            r["impl"], p,
            f"{to_float(r, 'mean_time_s'):.4f}",
            f"±{to_float(r, 'ci95_time_s'):.4f}",
            f"{to_float(r, 'speedup_vs_seq'):.2f}",
            f"{to_float(r, 'efficiency')*100:.1f}%",
        ])
    table = ax4.table(
        cellText=table_data,
        colLabels=["Implementation", "Workers", "Mean (s)", "95% CI", "Speedup", "Efficiency"],
        loc="center",
    )
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.6)
    ax4.axis("off")
    ax4.set_title("Detailed Statistical Summary", pad=18)

    ax5 = fig.add_subplot(gs[1, 2])
    throughput = []
    labels = []
    pixels = int(rows[0]["width"]) * int(rows[0]["height"])
    for r in cpu_rows:
        labels.append(f"{r['impl']}:{r.get('threads', '1')}")
        throughput.append(pixels / to_float(r, "mean_time_s") / 1e6)
    ax5.barh(labels, throughput, color="#0ea5e9")
    ax5.set_title("Throughput")
    ax5.set_xlabel("Megapixels / second")

    fig.suptitle(f"{title}\nSeq baseline: {seq_time:.4f}s | Resolution: {rows[0]['width']}x{rows[0]['height']} | Iter: {rows[0]['max_iter']}", fontsize=14)

    for ext in ["png", "pdf", "svg"]:
        fig.savefig(out_dir / f"mandelbrot_advanced_dashboard.{ext}", dpi=220)
    plt.close(fig)


def write_report(rows: list[dict], out_md: Path) -> None:
    seq = [r for r in rows if r["impl"] == "seq"][0]
    best = min(rows, key=lambda r: to_float(r, "mean_time_s"))
    lines = [
        "# Mandelbrot Performance Report",
        "",
        f"- Resolution: **{seq['width']}x{seq['height']}**",
        f"- Max iterations: **{seq['max_iter']}**",
        f"- Trials per configuration: **{seq['trials']}**",
        f"- Sequential baseline: **{to_float(seq, 'mean_time_s'):.4f}s**",
        f"- Best configuration: **{best['impl']}** workers={best.get('threads') or best.get('ranks') or '1'} @ **{to_float(best, 'mean_time_s'):.4f}s**",
        "",
        "## Notes",
        "- Use `mandelbrot_advanced_dashboard.pdf` for publication quality export.",
        "- Use `mandelbrot_advanced_dashboard.svg` for vector editing in Illustrator/Inkscape.",
        "- Raw benchmark data is stored in `benchmark_results.csv`.",
    ]
    out_md.write_text("\n".join(lines))


def make_zip(folder: Path, zip_path: Path) -> None:
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for f in folder.rglob("*"):
            if f.is_file():
                zf.write(f, f.relative_to(folder.parent))


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate advanced Mandelbrot benchmark charts.")
    parser.add_argument("--width", type=int, default=1280)
    parser.add_argument("--height", type=int, default=720)
    parser.add_argument("--max-iter", type=int, default=800)
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--threads", default="1,2,4,8")
    parser.add_argument("--include-mpi", action="store_true")
    parser.add_argument("--mpi-ranks", default="2,4")
    parser.add_argument("--input-csv", type=Path)
    parser.add_argument("--output-dir", type=Path, default=Path("results/charts"))
    parser.add_argument("--title", default="Mandelbrot Multi-backend Benchmark Dashboard")
    args = parser.parse_args()

    csv_path = args.output_dir / "benchmark_results.csv"

    if args.input_csv:
        rows = load_rows(args.input_csv)
    else:
        cfg = RunConfig(
            width=args.width,
            height=args.height,
            max_iter=args.max_iter,
            trials=args.trials,
            threads=[int(x) for x in args.threads.split(",") if x],
            include_mpi=args.include_mpi,
            mpi_ranks=[int(x) for x in args.mpi_ranks.split(",") if x],
        )
        rows = benchmark(cfg, csv_path)

    plot(rows, args.output_dir, args.title)
    write_report(rows, args.output_dir / "REPORT.md")
    make_zip(args.output_dir, args.output_dir.with_suffix(".zip"))

    print(f"Charts generated in: {args.output_dir}")
    print(f"Download bundle: {args.output_dir.with_suffix('.zip')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

# Parallel Computation of the Mandelbrot Set with Color-Encoded Iteration Depth

## Selected Dataset

This project uses a 2D grid of complex numbers over the plane, where each point is defined as:

\[
c = x + iy
\]

For each point \((x, y)\), we compute a third value: the number of iterations required before divergence under the Mandelbrot recurrence. This produces a dataset of the form:

\[
(x, y, \text{iterations})
\]

The third component is represented through color mapping rather than physical depth.

## Problem Definition

The Mandelbrot iteration is:

\[
z_{n+1} = z_n^2 + c, \quad z_0 = 0
\]

A point is considered to diverge when:

\[
|z_n| > 2
\]

The iteration count at divergence (or a maximum iteration cap if no divergence occurs) is stored per pixel and later mapped to a color scale.

## Why This Project

- Computationally expensive and highly parallelizable.
- Well-suited to compare multiple parallel programming models on the same workload.
- Direct mapping from independent pixel computations to thread/process-based execution.
- Compatible with both CPU and GPU acceleration.
- Produces a visually interpretable result (fractal image).

## Planned Implementations

### 1) Sequential (Baseline)

- Standard nested-loop traversal over image rows and columns.
- Computes iteration counts pixel by pixel on a single core.
- Serves as the timing baseline for speedup analysis.

### 2) OpenMP

- Parallelizes outer loops (typically rows) using shared-memory threads.
- Uses scheduling strategies (e.g., static/dynamic) for load balancing.
- Measures thread-level scalability on multicore CPUs.

### 3) Pthreads

- Manual thread creation and explicit workload partitioning.
- Assigns row or block ranges per thread.
- Provides low-level control for synchronization and scheduling experiments.

### 4) MPI

- Distributes image regions across multiple processes (single node or cluster).
- Uses scatter/gather-style coordination to collect final image data.
- Evaluates distributed-memory scaling and communication overhead.

### 5) CUDA

- Maps each pixel to a GPU thread.
- Executes Mandelbrot iterations in parallel on CUDA cores.
- Compares GPU throughput against CPU-based implementations.

## Expected Outputs

- High-resolution Mandelbrot image.
- Color-encoded visualization of iteration depth.
- Execution-time measurements for each implementation.
- Comparative charts/tables of performance.

## Performance Evaluation Criteria

- **Execution time** (absolute runtime per implementation).
- **Speedup** relative to sequential baseline:
  \[
  S_p = \frac{T_1}{T_p}
  \]
- **Scalability** with increasing threads/processes/problem size.
- **CPU vs GPU comparison** for throughput and efficiency.

## Conclusion

This project demonstrates how a mathematically simple but computationally intensive task can be accelerated using multiple parallel paradigms. The Mandelbrot set naturally forms a data-parallel workload, and iteration-depth color encoding provides both quantitative and visual insight into performance and correctness.

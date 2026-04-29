#include "mandelbrot_common.h"

#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__device__ int mandelbrot_iterations_device(double cx, double cy, int max_iter) {
    double zx = 0.0;
    double zy = 0.0;

    for (int i = 0; i < max_iter; i++) {
        double zx2 = zx * zx - zy * zy + cx;
        double zy2 = 2.0 * zx * zy + cy;
        zx = zx2;
        zy = zy2;
        if ((zx * zx + zy * zy) > 4.0) return i;
    }
    return max_iter;
}

__device__ RGB colorize_device(int iter, int max_iter) {
    RGB c;
    if (iter >= max_iter) {
        c.r = 0; c.g = 0; c.b = 0;
        return c;
    }
    double t = (double)iter / (double)max_iter;
    c.r = (uint8_t)(9.0 * (1.0 - t) * t * t * t * 255.0);
    c.g = (uint8_t)(15.0 * (1.0 - t) * (1.0 - t) * t * t * 255.0);
    c.b = (uint8_t)(8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t * 255.0);
    return c;
}

__global__ void mandelbrot_kernel(RGB *pixels, MandelbrotConfig cfg) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= cfg.width || y >= cfg.height) return;

    double cx = cfg.x_min + (cfg.x_max - cfg.x_min) * (double)x / (double)(cfg.width - 1);
    double cy = cfg.y_min + (cfg.y_max - cfg.y_min) * (double)y / (double)(cfg.height - 1);
    int iter = mandelbrot_iterations_device(cx, cy, cfg.max_iter);
    pixels[y * cfg.width + x] = colorize_device(iter, cfg.max_iter);
}

static void parse_args(int argc, char **argv, MandelbrotConfig *config, const char **output) {
    *config = default_config();
    *output = "results/mandelbrot_cuda.ppm";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) config->width = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) config->height = atoi(argv[++i]);
        else if (strcmp(argv[i], "--max-iter") == 0 && i + 1 < argc) config->max_iter = atoi(argv[++i]);
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) *output = argv[++i];
    }
}

int main(int argc, char **argv) {
    MandelbrotConfig cfg;
    const char *output;
    parse_args(argc, argv, &cfg, &output);

    RGB *host = (RGB *)malloc((size_t)cfg.width * (size_t)cfg.height * sizeof(RGB));
    RGB *device = NULL;
    cudaMalloc(&device, (size_t)cfg.width * (size_t)cfg.height * sizeof(RGB));

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    dim3 block(16, 16);
    dim3 grid((cfg.width + block.x - 1) / block.x, (cfg.height + block.y - 1) / block.y);

    cudaEventRecord(start);
    mandelbrot_kernel<<<grid, block>>>(device, cfg);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float ms = 0.0f;
    cudaEventElapsedTime(&ms, start, stop);

    cudaMemcpy(host, device, (size_t)cfg.width * (size_t)cfg.height * sizeof(RGB), cudaMemcpyDeviceToHost);
    int rc = write_ppm(output, host, cfg.width, cfg.height);

    cudaFree(device);
    free(host);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    if (rc != 0) {
        fprintf(stderr, "Failed to write %s\n", output);
        return 1;
    }

    printf("[cuda] %dx%d max_iter=%d kernel_time=%.3f ms output=%s\n", cfg.width, cfg.height, cfg.max_iter, ms, output);
    return 0;
}

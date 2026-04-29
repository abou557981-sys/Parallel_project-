#include "mandelbrot_common.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static double now_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

static void parse_args(int argc, char **argv, MandelbrotConfig *config, const char **output, int *sched_dynamic) {
    *config = default_config();
    *output = "results/mandelbrot_omp.ppm";
    *sched_dynamic = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) config->width = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) config->height = atoi(argv[++i]);
        else if (strcmp(argv[i], "--max-iter") == 0 && i + 1 < argc) config->max_iter = atoi(argv[++i]);
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) *output = argv[++i];
        else if (strcmp(argv[i], "--dynamic") == 0) *sched_dynamic = 1;
    }
}

int main(int argc, char **argv) {
    MandelbrotConfig cfg;
    const char *output;
    int sched_dynamic;
    parse_args(argc, argv, &cfg, &output, &sched_dynamic);

    RGB *pixels = malloc((size_t)cfg.width * (size_t)cfg.height * sizeof(RGB));
    if (!pixels) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    double start = now_seconds();
    if (sched_dynamic) {
        omp_set_schedule(omp_sched_dynamic, 1);
    } else {
        omp_set_schedule(omp_sched_static, 0);
    }

#pragma omp parallel for schedule(runtime)
    for (int y = 0; y < cfg.height; y++) {
        double cy = cfg.y_min + (cfg.y_max - cfg.y_min) * (double)y / (double)(cfg.height - 1);
        for (int x = 0; x < cfg.width; x++) {
            double cx = cfg.x_min + (cfg.x_max - cfg.x_min) * (double)x / (double)(cfg.width - 1);
            int iter = mandelbrot_iterations(cx, cy, cfg.max_iter);
            pixels[y * cfg.width + x] = mandelbrot_colorize(iter, cfg.max_iter);
        }
    }
    double elapsed = now_seconds() - start;

    int rc = write_ppm(output, pixels, cfg.width, cfg.height);
    free(pixels);

    if (rc != 0) {
        fprintf(stderr, "Failed to write %s\n", output);
        return 2;
    }

    printf("[openmp] threads=%d %dx%d max_iter=%d time=%.6f s output=%s\n",
           omp_get_max_threads(), cfg.width, cfg.height, cfg.max_iter, elapsed, output);
    return 0;
}

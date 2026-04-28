#include "mandelbrot_common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef struct {
    MandelbrotConfig cfg;
    RGB *pixels;
    int row_start;
    int row_end;
} WorkerArgs;

static double now_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

static void *worker_fn(void *arg) {
    WorkerArgs *wa = (WorkerArgs *)arg;

    for (int y = wa->row_start; y < wa->row_end; y++) {
        double cy = wa->cfg.y_min + (wa->cfg.y_max - wa->cfg.y_min) * (double)y / (double)(wa->cfg.height - 1);
        for (int x = 0; x < wa->cfg.width; x++) {
            double cx = wa->cfg.x_min + (wa->cfg.x_max - wa->cfg.x_min) * (double)x / (double)(wa->cfg.width - 1);
            int iter = mandelbrot_iterations(cx, cy, wa->cfg.max_iter);
            wa->pixels[y * wa->cfg.width + x] = mandelbrot_colorize(iter, wa->cfg.max_iter);
        }
    }

    return NULL;
}

static void parse_args(int argc, char **argv, MandelbrotConfig *config, const char **output, int *num_threads) {
    *config = default_config();
    *output = "results/mandelbrot_pthreads.ppm";
    *num_threads = 8;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) config->width = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) config->height = atoi(argv[++i]);
        else if (strcmp(argv[i], "--max-iter") == 0 && i + 1 < argc) config->max_iter = atoi(argv[++i]);
        else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) *num_threads = atoi(argv[++i]);
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) *output = argv[++i];
    }

    if (*num_threads < 1) *num_threads = 1;
}

int main(int argc, char **argv) {
    MandelbrotConfig cfg;
    const char *output;
    int num_threads;
    parse_args(argc, argv, &cfg, &output, &num_threads);

    RGB *pixels = malloc((size_t)cfg.width * (size_t)cfg.height * sizeof(RGB));
    pthread_t *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    WorkerArgs *args = malloc((size_t)num_threads * sizeof(WorkerArgs));
    if (!pixels || !threads || !args) {
        fprintf(stderr, "Allocation failed\n");
        free(pixels); free(threads); free(args);
        return 1;
    }

    int base_rows = cfg.height / num_threads;
    int remainder = cfg.height % num_threads;

    double start = now_seconds();
    int row = 0;
    for (int t = 0; t < num_threads; t++) {
        int rows = base_rows + (t < remainder ? 1 : 0);
        args[t].cfg = cfg;
        args[t].pixels = pixels;
        args[t].row_start = row;
        args[t].row_end = row + rows;
        row += rows;

        if (pthread_create(&threads[t], NULL, worker_fn, &args[t]) != 0) {
            fprintf(stderr, "Failed creating thread %d\n", t);
            return 2;
        }
    }

    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }
    double elapsed = now_seconds() - start;

    int rc = write_ppm(output, pixels, cfg.width, cfg.height);
    free(pixels);
    free(threads);
    free(args);

    if (rc != 0) {
        fprintf(stderr, "Failed to write %s\n", output);
        return 3;
    }

    printf("[pthreads] threads=%d %dx%d max_iter=%d time=%.6f s output=%s\n",
           num_threads, cfg.width, cfg.height, cfg.max_iter, elapsed, output);
    return 0;
}

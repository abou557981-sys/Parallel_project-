#include "mandelbrot_common.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void parse_args(int argc, char **argv, MandelbrotConfig *config, const char **output) {
    *config = default_config();
    *output = "results/mandelbrot_mpi.ppm";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) config->width = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) config->height = atoi(argv[++i]);
        else if (strcmp(argv[i], "--max-iter") == 0 && i + 1 < argc) config->max_iter = atoi(argv[++i]);
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) *output = argv[++i];
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank = 0;
    int size = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MandelbrotConfig cfg;
    const char *output;
    parse_args(argc, argv, &cfg, &output);

    int *rows_per_rank = malloc((size_t)size * sizeof(int));
    int *row_offsets = malloc((size_t)size * sizeof(int));
    int base_rows = cfg.height / size;
    int rem_rows = cfg.height % size;

    int offset = 0;
    for (int r = 0; r < size; r++) {
        rows_per_rank[r] = base_rows + (r < rem_rows ? 1 : 0);
        row_offsets[r] = offset;
        offset += rows_per_rank[r];
    }

    int local_rows = rows_per_rank[rank];
    RGB *local_pixels = malloc((size_t)local_rows * (size_t)cfg.width * sizeof(RGB));
    if (!local_pixels) {
        fprintf(stderr, "Rank %d allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for (int y = 0; y < local_rows; y++) {
        int global_y = row_offsets[rank] + y;
        double cy = cfg.y_min + (cfg.y_max - cfg.y_min) * (double)global_y / (double)(cfg.height - 1);
        for (int x = 0; x < cfg.width; x++) {
            double cx = cfg.x_min + (cfg.x_max - cfg.x_min) * (double)x / (double)(cfg.width - 1);
            int iter = mandelbrot_iterations(cx, cy, cfg.max_iter);
            local_pixels[y * cfg.width + x] = mandelbrot_colorize(iter, cfg.max_iter);
        }
    }

    int *recv_counts = NULL;
    int *recv_displs = NULL;
    RGB *global_pixels = NULL;

    if (rank == 0) {
        recv_counts = malloc((size_t)size * sizeof(int));
        recv_displs = malloc((size_t)size * sizeof(int));
        global_pixels = malloc((size_t)cfg.width * (size_t)cfg.height * sizeof(RGB));

        int disp = 0;
        for (int r = 0; r < size; r++) {
            recv_counts[r] = rows_per_rank[r] * cfg.width * (int)sizeof(RGB);
            recv_displs[r] = disp;
            disp += recv_counts[r];
        }
    }

    int local_bytes = local_rows * cfg.width * (int)sizeof(RGB);
    MPI_Gatherv(local_pixels, local_bytes, MPI_BYTE,
                global_pixels, recv_counts, recv_displs, MPI_BYTE,
                0, MPI_COMM_WORLD);

    double elapsed = MPI_Wtime() - start;

    if (rank == 0) {
        if (write_ppm(output, global_pixels, cfg.width, cfg.height) != 0) {
            fprintf(stderr, "Failed to write %s\n", output);
        } else {
            printf("[mpi] ranks=%d %dx%d max_iter=%d time=%.6f s output=%s\n",
                   size, cfg.width, cfg.height, cfg.max_iter, elapsed, output);
        }
    }

    free(global_pixels);
    free(recv_counts);
    free(recv_displs);
    free(local_pixels);
    free(rows_per_rank);
    free(row_offsets);

    MPI_Finalize();
    return 0;
}

#ifndef MANDELBROT_COMMON_H
#define MANDELBROT_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
    int max_iter;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
} MandelbrotConfig;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;

int mandelbrot_iterations(double cx, double cy, int max_iter);
RGB mandelbrot_colorize(int iter, int max_iter);
int write_ppm(const char *filename, const RGB *pixels, int width, int height);
MandelbrotConfig default_config(void);

#ifdef __cplusplus
}
#endif

#endif

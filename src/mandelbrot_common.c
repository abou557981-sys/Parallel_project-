#include "mandelbrot_common.h"

#include <math.h>
#include <stdio.h>

int mandelbrot_iterations(double cx, double cy, int max_iter) {
    double zx = 0.0;
    double zy = 0.0;

    for (int i = 0; i < max_iter; i++) {
        double zx2 = zx * zx - zy * zy + cx;
        double zy2 = 2.0 * zx * zy + cy;
        zx = zx2;
        zy = zy2;

        if ((zx * zx + zy * zy) > 4.0) {
            return i;
        }
    }

    return max_iter;
}

RGB mandelbrot_colorize(int iter, int max_iter) {
    RGB color;

    if (iter >= max_iter) {
        color.r = 0;
        color.g = 0;
        color.b = 0;
        return color;
    }

    double t = (double)iter / (double)max_iter;
    color.r = (uint8_t)(9.0 * (1.0 - t) * t * t * t * 255.0);
    color.g = (uint8_t)(15.0 * (1.0 - t) * (1.0 - t) * t * t * 255.0);
    color.b = (uint8_t)(8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t * 255.0);
    return color;
}

int write_ppm(const char *filename, const RGB *pixels, int width, int height) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }

    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    if ((int)fwrite(pixels, sizeof(RGB), (size_t)(width * height), fp) != width * height) {
        fclose(fp);
        return -2;
    }

    fclose(fp);
    return 0;
}

MandelbrotConfig default_config(void) {
    MandelbrotConfig config;
    config.width = 1920;
    config.height = 1080;
    config.max_iter = 1000;
    config.x_min = -2.5;
    config.x_max = 1.0;
    config.y_min = -1.0;
    config.y_max = 1.0;
    return config;
}

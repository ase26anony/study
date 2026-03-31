/* test1.c - Floating-point intensive with nested loops */
#include <math.h>

volatile double global_d = 3.14159;

void compute_mandelbrot(double *output, int iterations, int size) {
    double x_min = -2.0, x_max = 1.0;
    double y_min = -1.5, y_max = 1.5;
    double x_step = (x_max - x_min) / size;
    double y_step = (y_max - y_min) / size;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double x0 = x_min + i * x_step;
            double y0 = y_min + j * y_step;
            double x = 0.0, y = 0.0;
            int iter = 0;
            
            while (x*x + y*y <= 4.0 && iter < iterations) {
                double x_temp = x*x - y*y + x0;
                y = 2.0*x*y + y0;
                x = x_temp;
                iter++;
            }
            
            // Complex data dependency chain
            double val = (double)iter / iterations;
            output[i*size + j] = sin(val) * cos(val) * exp(val) * global_d;
        }
    }
}

void matrix_multiply(double *a, double *b, double *c, int n) {
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            double aik = a[i*n + k];
            for (int j = 0; j < n; j++) {
                c[i*n + j] += aik * b[k*n + j];
            }
        }
    }
}

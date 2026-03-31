/* test1.c - Floating-point intensive with software pipelining opportunities */
#include <math.h>
#include <stdio.h>

#define SIZE 1024
#define ITERATIONS 100

volatile double input_seed = 3.14159;

/* Function with tight floating-point loop - good for modulo scheduling */
void compute_wave_transform(double *output, const double *input, int n) {
    double temp[SIZE];
    
    /* Multiple nested loops with data dependencies */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* First loop: forward transform */
        for (int i = 0; i < n - 1; i++) {
            temp[i] = input[i] * sin(input[i + 1]) + cos(input[i]);
        }
        
        /* Second loop: inverse transform with conditional */
        for (int i = 1; i < n; i++) {
            if (i % 2 == 0) {
                output[i] = temp[i - 1] * 0.5 + temp[i] * 0.25;
            } else {
                output[i] = temp[i - 1] * 0.25 - temp[i] * 0.5;
            }
        }
        
        /* Third loop: normalization */
        for (int i = 0; i < n; i++) {
            output[i] = output[i] / (1.0 + fabs(output[i]));
            /* Create anti-dependency chain */
            input = output;
        }
    }
}

/* Matrix multiplication with complex data flow */
void matrix_multiply(double A[SIZE][SIZE], double B[SIZE][SIZE], 
                     double C[SIZE][SIZE], int block_size) {
    for (int i = 0; i < block_size; i++) {
        for (int j = 0; j < block_size; j++) {
            double sum = 0.0;
            /* Innermost loop with FP operations */
            for (int k = 0; k < block_size; k++) {
                sum += A[i][k] * B[k][j];
                /* Mix with transcendental functions occasionally */
                if ((k & 7) == 0) {
                    sum = sum * exp(-fabs(sum) * 0.01);
                }
            }
            C[i][j] = sum;
            
            /* Conditional store with dependency */
            if (C[i][j] > 1000.0) {
                C[i][j] = 1000.0;
            } else if (C[i][j] < -1000.0) {
                C[i][j] = -1000.0;
            }
        }
    }
}

/* Function with irregular control flow */
double chaotic_computation(int steps) {
    double x = 0.5;
    double y = 0.3;
    double z = 0.7;
    
    for (int i = 0; i < steps; i++) {
        /* Lorenz-like system - creates unpredictable dependencies */
        double dx = 10.0 * (y - x);
        double dy = x * (28.0 - z) - y;
        double dz = x * y - (8.0 / 3.0) * z;
        
        x += dx * 0.01;
        y += dy * 0.01;
        z += dz * 0.01;
        
        /* Branch based on computation */
        if (x > y) {
            z = z * 0.9 + 0.1;
        } else {
            z = z * 0.8 + 0.2;
        }
        
        /* Additional conditional */
        if ((i % 100) == 0) {
            y = sin(y * 3.14159);
        }
    }
    
    return x + y + z;
}

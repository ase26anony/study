/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

volatile double input[SIZE];
volatile double output[SIZE];

/* Complex FP computation with data dependencies */
void fp_compute_intensive(int n) {
    double acc = 0.0;
    
    /* Outer loop creates scheduling pressure */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Inner loop with FP operations */
        for (int i = 0; i < n; i++) {
            /* Mixed FP operations creating RAW dependencies */
            double x = input[i];
            double y = sin(x * 0.5);
            double z = cos(x + y);
            
            /* Chain of dependent operations */
            double a = y * z;
            double b = a / (x + 1.0);
            double c = sqrt(fabs(b));
            
            /* Memory store with dependency */
            output[i] = c + acc;
            
            /* Accumulator creates loop-carried dependency */
            acc += c * 0.01;
        }
        
        /* Conditional branch creates control flow */
        if (iter % 10 == 0) {
            /* Additional computation on branch */
            for (int j = 0; j < n/2; j++) {
                output[j] *= 1.1;
            }
        }
    }
}

/* Matrix-like operations for modulo scheduling */
void matrix_operations(int n) {
    double temp[SIZE];
    
    /* Triple nested loop - good for software pipelining */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                /* Complex dependency chain */
                sum += input[i * n + k] * input[k * n + j];
                sum = fmod(sum, 1000.0);
            }
            temp[i * n + j] = sum;
        }
    }
    
    /* Copy back with transformation */
    for (int i = 0; i < n * n; i++) {
        output[i] = tan(temp[i]);
    }
}

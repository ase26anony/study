/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

volatile double input_data[SIZE];
volatile double output_data[SIZE];

/* Function with complex data dependencies and scheduling pressure */
void compute_transform(int iterations) {
    double temp[SIZE];
    int i, j, k;
    
    /* Initialize with some values */
    for (i = 0; i < SIZE; i++) {
        input_data[i] = (double)i * 0.1;
    }
    
    /* Multiple nested loops with mixed operations */
    for (k = 0; k < iterations; k++) {
        /* First computation phase - creates scheduling pressure */
        for (i = 1; i < SIZE - 1; i++) {
            /* Complex floating-point operations with dependencies */
            double a = input_data[i-1];
            double b = input_data[i];
            double c = input_data[i+1];
            
            /* Mixed operations that create instruction-level parallelism needs */
            temp[i] = (a * b) + (c * 0.5);
            temp[i] = temp[i] / (1.0 + fabs(b));
            temp[i] = sin(temp[i]) * cos(temp[i]);
        }
        
        /* Second phase with branches - creates control flow complexity */
        for (i = 0; i < SIZE; i++) {
            if (temp[i] > 0.5) {
                output_data[i] = temp[i] * 2.0;
            } else if (temp[i] < -0.5) {
                output_data[i] = temp[i] / 2.0;
            } else {
                output_data[i] = sqrt(fabs(temp[i]));
            }
            
            /* Additional arithmetic to increase instruction mix */
            output_data[i] += (i % 10) * 0.01;
        }
        
        /* Third phase - reduction operation */
        double sum = 0.0;
        for (i = 0; i < SIZE; i += 4) {
            /* Unrolled loop for scheduling */
            sum += output_data[i];
            sum += output_data[i+1];
            sum += output_data[i+2];
            sum += output_data[i+3];
        }
        
        /* Use the sum to prevent dead code elimination */
        if (sum > 1000.0) {
            for (i = 0; i < SIZE; i++) {
                input_data[i] = output_data[i] * 0.99;
            }
        }
    }
}

/* Another function with different pattern */
void matrix_operations(int n) {
    double mat1[64][64];
    double mat2[64][64];
    double result[64][64];
    int i, j, k;
    
    /* Initialize matrices */
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            mat1[i][j] = (double)(i + j) * 0.1;
            mat2[i][j] = (double)(i * j) * 0.05;
        }
    }
    
    /* Matrix multiplication with scheduling opportunities */
    for (i = 0; i < n; i++) {
        for (j = 0; j < 64; j++) {
            for (k = 0; k < 64; k++) {
                result[j][k] = 0.0;
                for (int l = 0; l < 64; l++) {
                    result[j][k] += mat1[j][l] * mat2[l][k];
                }
                /* Conditional operation inside innermost loop */
                if (result[j][k] > 100.0) {
                    result[j][k] = log(result[j][k]);
                }
            }
        }
    }
}

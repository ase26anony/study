/* test1.c - Floating-point intensive with software pipelining opportunities */
#include <math.h>
#include <stdio.h>

#define SIZE 1024
#define ITERATIONS 100

volatile double input[SIZE];
volatile double output[SIZE];

/* Function with tight FP loop for modulo scheduling */
void compute_fp_transform(int n, double scale) {
    double temp[SIZE];
    
    /* Multiple nested loops with data dependencies */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* First loop: forward computation */
        for (int i = 0; i < n; i++) {
            double x = input[i] * scale;
            temp[i] = sin(x) * cos(x) + exp(x * 0.01);
        }
        
        /* Second loop: backward computation with branches */
        for (int i = n - 1; i >= 0; i--) {
            if (temp[i] > 0.5) {
                output[i] = temp[i] * 2.0 + sqrt(temp[i]);
            } else {
                output[i] = temp[i] * 0.5 - log(fabs(temp[i]) + 1.0);
            }
        }
        
        /* Third loop: reduction with complex dependencies */
        double sum = 0.0;
        for (int i = 0; i < n; i += 4) {
            double t1 = output[i] * output[i + 1];
            double t2 = output[i + 2] * output[i + 3];
            sum += t1 - t2 + (t1 > t2 ? t1 : t2);
        }
        
        /* Update scale with feedback */
        scale = fmod(scale + sum * 0.001, 2.0 * M_PI);
    }
}

/* Another function with different pattern */
void matrix_fp_operations(int size) {
    double mat1[SIZE][SIZE];
    double mat2[SIZE][SIZE];
    double result[SIZE][SIZE];
    
    /* Initialize with volatile to prevent optimization */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            mat1[i][j] = (i + j) * 0.1;
            mat2[i][j] = (i - j) * 0.2;
        }
    }
    
    /* Matrix multiplication with scheduling pressure */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                sum += mat1[i][k] * mat2[k][j];
                /* Conditional inside innermost loop creates scheduling complexity */
                if (sum > 1000.0) {
                    sum = fmod(sum, 1000.0);
                }
            }
            result[i][j] = sum;
            
            /* Cross-iteration dependency */
            if (j > 0) {
                result[i][j] += result[i][j-1] * 0.1;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    double total = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            total += result[i][j];
        }
    }
    printf("Matrix total: %f\n", total);
}

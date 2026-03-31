/* File 1: compute-intensive.c - Floating point calculations with tight loops */
#include <math.h>
#include <stdio.h>

/* Complex floating-point computation with data dependencies */
double compute_pressure(int iterations, double* input, double* output) {
    volatile double sum = 0.0;  /* volatile to prevent optimization */
    double temp[256];
    
    /* Outer loop creates scheduling pressure */
    for (int i = 0; i < iterations; i++) {
        /* Inner loop with mixed FP operations */
        for (int j = 0; j < 256; j++) {
            /* Create data dependencies between iterations */
            double x = input[j] * 1.234567;
            double y = x + sin(j * 0.01);
            double z = y * cos(x);
            temp[j] = z / (1.0 + fabs(y));
            
            /* Cross-iteration dependency */
            if (j > 0) {
                temp[j] += temp[j-1] * 0.5;
            }
        }
        
        /* Conditional branch creates basic block boundaries */
        if (i % 3 == 0) {
            for (int k = 0; k < 128; k++) {
                output[k] = temp[k] * 0.8 + output[k] * 0.2;
            }
        } else if (i % 3 == 1) {
            for (int k = 0; k < 128; k++) {
                output[k] = temp[k+128] * 0.6 + output[k] * 0.4;
            }
        } else {
            for (int k = 0; k < 128; k++) {
                output[k] = (temp[k] + temp[k+128]) * 0.5;
            }
        }
        
        sum += output[i % 128];
    }
    
    return sum;
}

/* Matrix multiplication with software pipelining potential */
void matrix_multiply(int n, double A[n][n], double B[n][n], double C[n][n]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            /* Innermost loop - prime candidate for software pipelining */
            for (int k = 0; k < n; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
            
            /* Conditional store creates scheduling barrier */
            if (sum < 0) {
                C[i][j] = -sum;
            }
        }
    }
}

/* Function with irregular access pattern */
double irregular_compute(int size, double* data) {
    double result = 0.0;
    int i = 0;
    
    /* Mixed loop types */
    while (i < size) {
        int stride = (i % 8) + 1;
        
        /* Unpredictable memory access pattern */
        for (int j = i; j < size && j < i + stride; j++) {
            result += data[j] * data[size - j - 1];
            
            /* Branch inside innermost loop */
            if (data[j] > 0.5) {
                result -= 0.1;
            } else {
                result += 0.05;
            }
        }
        
        i += stride;
        
        /* Function call acts as scheduling barrier */
        result = fmod(result, 1000.0);
    }
    
    return result;
}

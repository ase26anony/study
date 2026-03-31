/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>

#define SIZE 256
#define ITERATIONS 100

/* Complex floating-point computation with data dependencies */
double compute_kernel(double a, double b, int iterations) {
    double result = 0.0;
    double temp = a;
    
    /* Create scheduling pressure with mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Multiple dependent floating-point operations */
        temp = sin(temp) * cos(b) + tan(a * b);
        result += temp * sqrt(fabs(temp));
        
        /* Conditional creates branch scheduling requirements */
        if (temp > 0.5) {
            result *= 1.1;
            temp = log(fabs(temp) + 1.0);
        } else {
            result *= 0.9;
            temp = exp(-fabs(temp));
        }
        
        /* More arithmetic to create instruction mix */
        for (int j = 0; j < 4; j++) {
            result += pow(temp, j + 1) / (j + 1);
        }
    }
    
    return result;
}

/* Matrix-style computation with memory accesses */
void matrix_transform(double in[SIZE][SIZE], double out[SIZE][SIZE]) {
    volatile int seed = 12345; /* Prevent optimization */
    
    /* Nested loops create scheduling opportunities */
    for (int i = 1; i < SIZE - 1; i++) {
        for (int j = 1; j < SIZE - 1; j++) {
            /* Stencil computation with data dependencies */
            out[i][j] = (in[i-1][j] + in[i+1][j] + 
                        in[i][j-1] + in[i][j+1]) * 0.25;
            
            /* Additional computation to increase basic block size */
            out[i][j] += sin(in[i][j] * 0.01) * cos(out[i][j]);
            
            /* Conditional with side effects */
            if ((i * j) % 7 == 0) {
                out[i][j] *= 1.5;
                out[i][j] = fmod(out[i][j], 100.0);
            }
        }
    }
}

/* Main computation function */
double intensive_computation(int param) {
    double array[SIZE][SIZE];
    double result = 0.0;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array[i][j] = sin(i * 0.1) * cos(j * 0.1);
        }
    }
    
    /* Multiple passes to create scheduling context */
    for (int pass = 0; pass < 3; pass++) {
        double temp[SIZE][SIZE];
        matrix_transform(array, temp);
        
        /* Mix with kernel computation */
        for (int i = 0; i < SIZE; i += 8) {
            result += compute_kernel(temp[i][i], param * 0.01, 10);
        }
        
        /* Copy back for next iteration */
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                array[i][j] = temp[i][j];
            }
        }
    }
    
    return result;
}

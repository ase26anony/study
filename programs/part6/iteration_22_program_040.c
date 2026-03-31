/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

volatile double global_seed = 3.14159;

/* Complex floating-point computation with data dependencies */
double compute_polynomial(int iterations, double* results) {
    double sum = 0.0;
    double x = global_seed;
    
    /* Outer loop creates scheduling pressure */
    for (int i = 0; i < iterations; i++) {
        double term = x;
        
        /* Inner polynomial evaluation with mixed operations */
        for (int j = 0; j < 8; j++) {
            term = term * x + (j * 0.1);
            term = term / (x + 1.0);
            term = sin(term) * cos(term);
        }
        
        /* Conditional creates branch scheduling challenges */
        if (term > 0.5) {
            sum += term * 2.0;
            results[i] = term * 1.5;
        } else {
            sum -= term * 0.5;
            results[i] = term * 0.75;
        }
        
        /* Update x with chaotic behavior to prevent optimization */
        x = fmod(x * 3.7 * (i + 1), 1.0) + 0.1;
    }
    
    return sum;
}

/* Matrix-like operations with memory dependencies */
void matrix_operations(int size, double* a, double* b, double* c) {
    for (int i = 0; i < size; i++) {
        double acc = 0.0;
        for (int j = 0; j < size; j++) {
            /* Mixed operations creating instruction mix */
            double val = a[i * size + j] * b[j * size + i];
            val = val + (i * j * 0.01);
            val = val / (global_seed + 1.0);
            acc += val;
            
            /* Store with address computation */
            c[i * size + j] = acc;
        }
        
        /* Barrier-like operation */
        if (i % 4 == 0) {
            for (int k = 0; k < size; k++) {
                c[i * size + k] *= 1.1;
            }
        }
    }
}

/* Main driver for test1 */
double test1_main(int argc, char** argv) {
    int size = argc > 1 ? atoi(argv[1]) % 50 + 10 : 20;
    
    double* results = (double*)malloc(size * sizeof(double));
    double* a = (double*)malloc(size * size * sizeof(double));
    double* b = (double*)malloc(size * size * sizeof(double));
    double* c = (double*)malloc(size * size * sizeof(double));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < size; i++) {
        results[i] = sin(i * 0.1);
        for (int j = 0; j < size; j++) {
            a[i * size + j] = sin(i * j * 0.01);
            b[i * size + j] = cos(i * j * 0.01);
        }
    }
    
    double sum1 = compute_polynomial(size, results);
    matrix_operations(size, a, b, c);
    
    /* Final reduction */
    double final_sum = sum1;
    for (int i = 0; i < size * size; i++) {
        final_sum += c[i] * 0.01;
    }
    
    free(results);
    free(a);
    free(b);
    free(c);
    
    return final_sum;
}

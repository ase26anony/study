/* test1.c - Floating point intensive with software pipelining opportunities */
#include <math.h>

volatile double input = 3.14159;

double compute_polynomial(int iterations) {
    double result = 0.0;
    double x = input;
    
    /* Nested loops create scheduling pressure */
    for (int i = 0; i < iterations; i++) {
        double term = 1.0;
        for (int j = 0; j < 8; j++) {
            /* Mixed FP operations */
            term = term * x + (j * 0.1);
            term = term / (j + 1.0);
            term = sqrt(fabs(term)) + 0.5;
        }
        
        /* Conditional creates control flow */
        if (term > 2.0) {
            result += term * 0.8;
        } else {
            result += term * 1.2;
        }
        
        /* Memory access pattern */
        result = result * (1.0 - 1.0/(i+2.0));
    }
    
    return result;
}

/* Another function with different pattern */
double matrix_multiply(int size) {
    volatile double a[16][16];
    volatile double b[16][16];
    double c[16][16];
    
    /* Initialize */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            a[i][j] = i * 0.1 + j * 0.01;
            b[i][j] = i * 0.2 - j * 0.02;
        }
    }
    
    /* Triple nested loop - high scheduling pressure */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            c[i][j] = 0.0;
            for (int k = 0; k < size; k++) {
                /* Complex expression with dependencies */
                c[i][j] += a[i][k] * b[k][j] 
                          + sin(a[i][k] * 0.5) 
                          - cos(b[k][j] * 0.3);
            }
            
            /* Conditional in inner loop */
            if (c[i][j] < 0) {
                c[i][j] = -c[i][j];
            }
        }
    }
    
    /* Reduce result */
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum += c[i][j];
        }
    }
    
    return sum;
}

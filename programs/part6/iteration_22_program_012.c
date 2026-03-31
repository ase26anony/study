/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>

volatile double global_d = 3.14159;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    int i, j;
    
    /* Outer loop creates scheduling pressure */
    for (i = 0; i < iterations; i++) {
        double term = 1.0;
        
        /* Inner loop with mixed operations */
        for (j = 1; j <= 100; j++) {
            /* Create data dependencies */
            term = term * (double)j / (double)(j + 1);
            
            /* Mix with trigonometric functions */
            if (j % 2 == 0) {
                term += sin((double)j * 0.01);
            } else {
                term -= cos((double)j * 0.01);
            }
            
            /* Memory access pattern */
            term *= global_d;
        }
        
        /* Conditional update */
        if (i % 3 == 0) {
            sum += term / (i + 1);
        } else if (i % 3 == 1) {
            sum -= term / (i + 1);
        } else {
            sum += term * 0.5 / (i + 1);
        }
    }
    
    return sum;
}

/* Another function with different pattern */
void matrix_multiply_accumulate(double *a, double *b, double *c, int n) {
    int i, j, k;
    
    /* Triple nested loop - creates complex scheduling scenario */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                /* Multiple dependencies */
                double a_val = a[i * n + k];
                double b_val = b[k * n + j];
                
                /* Mixed operations */
                if ((i + j + k) % 2 == 0) {
                    sum += a_val * b_val;
                } else {
                    sum += a_val * b_val * 0.5;
                }
                
                /* Prevent optimization */
                sum += global_d * 0.0001;
            }
            c[i * n + j] = sum;
        }
    }
}

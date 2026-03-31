/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>
#include <stdlib.h>

volatile double global_seed = 3.14159;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    double sign = 1.0;
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < iterations; i++) {
        double term = sign / (2 * i + 1);
        
        /* Nested computation with scheduling pressure */
        for (int j = 0; j < 10; j++) {
            term = term * (1.0 + 0.01 * j);
        }
        
        sum += term;
        sign = -sign;
        
        /* Conditional branch creates scheduling barrier */
        if (i % 100 == 0) {
            sum = sum * 0.999;
        }
    }
    
    return 4.0 * sum;
}

/* Matrix multiplication with software pipelining potential */
void matrix_multiply(double *A, double *B, double *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            /* Innermost loop - prime candidate for software pipelining */
            for (int k = 0; k < n; k++) {
                sum += A[i * n + k] * B[k * n + j];
                
                /* Mixed operations create varied instruction types */
                if (k % 8 == 0) {
                    sum = fabs(sum) * 1.0001;
                }
            }
            C[i * n + j] = sum;
        }
    }
}

/* Function with loop-carried dependencies */
double recursive_series(int n) {
    double a = 1.0, b = 0.5;
    
    for (int i = 0; i < n; i++) {
        double temp = a;
        a = 0.9 * a - 0.1 * b + sin(i * 0.01);
        b = 0.8 * b + 0.2 * temp + cos(i * 0.02);
        
        /* Complex conditional with scheduling implications */
        if (i > 100 && i < 200) {
            a = a * (1.0 + 0.001 * (i - 100));
            b = b / (1.0 + 0.001 * (i - 100));
        }
    }
    
    return a + b;
}

/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

volatile double global_seed = 3.14159;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    int i, j;
    
    /* Outer loop creates scheduling pressure */
    for (i = 0; i < iterations; i++) {
        double term = 1.0 / (2.0 * i + 1.0);
        
        /* Inner loop with mixed operations */
        for (j = 0; j < 8; j++) {
            term = term * (1.0 - 1.0/(j + 2.0));
            term = term + sin(global_seed * j) * 0.001;
        }
        
        if (i % 2 == 0) {
            sum += term;
        } else {
            sum -= term;
        }
        
        /* Conditional with data dependency */
        if (sum > 2.0) {
            sum = sum * 0.5;
        }
    }
    
    return 4.0 * sum;
}

/* Matrix operations with scheduling barriers */
void matrix_multiply(double *a, double *b, double *c, int n) {
    int i, j, k;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            double sum = 0.0;
            for (k = 0; k < n; k++) {
                /* Complex addressing patterns */
                sum += a[i * n + k] * b[k * n + j];
                
                /* Insert scheduling barrier with conditional */
                if (k % 16 == 0) {
                    sum = sum * (1.0 + global_seed * 0.0001);
                }
            }
            c[i * n + j] = sum;
            
            /* Another conditional branch */
            if (c[i * n + j] < 0) {
                c[i * n + j] = -c[i * n + j];
            }
        }
    }
}

/* Function with software pipelining potential */
double polynomial_evaluation(double x, int degree) {
    double result = 0.0;
    double power = 1.0;
    int i;
    
    for (i = 0; i <= degree; i++) {
        double coeff = 1.0 / (i + 1.0);
        result += coeff * power;
        power *= x;
        
        /* Create loop-carried dependency */
        if (i % 4 == 0) {
            result = result * (1.0 + sin(power) * 0.01);
        }
    }
    
    return result;
}

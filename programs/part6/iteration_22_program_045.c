/* test1.c - Floating-point intensive computations */
#include <math.h>
#include <stdlib.h>

/* Complex floating-point computation with data dependencies */
double compute_pressure(int n, double* input) {
    double sum = 0.0;
    double prod = 1.0;
    
    /* Outer loop creates scheduling pressure */
    for (int i = 0; i < n; i++) {
        double acc = input[i];
        
        /* Inner loop with mixed operations */
        for (int j = 0; j < 8; j++) {
            acc = acc * 1.01 + sin(acc * 0.1);
            acc = acc / 1.02 + cos(acc * 0.05);
        }
        
        /* Conditional creates branch scheduling needs */
        if (acc > 0.5) {
            sum += acc * acc;
            prod *= acc;
        } else {
            sum += acc;
            prod /= (fabs(acc) + 0.001);
        }
        
        /* More arithmetic to create instruction mix */
        sum = sum * 0.999 + 0.001;
        prod = prod * 0.998 + 0.002;
    }
    
    return sum + prod;
}

/* Matrix multiplication with scheduling barriers */
void matrix_multiply(int size, double** a, double** b, double** result) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                /* Complex expression with dependencies */
                sum += a[i][k] * b[k][j] + 
                       sin(a[i][k] * 0.01) * cos(b[k][j] * 0.01);
            }
            result[i][j] = sum;
            
            /* Conditional store */
            if (sum > 100.0) {
                result[i][j] = log(fabs(sum) + 1.0);
            }
        }
    }
}

/* Function with loop unrolling opportunities */
double unrolled_computation(int iterations) {
    volatile double seed = 3.14159; /* Prevent optimization */
    double a = seed, b = seed * 2, c = seed * 3;
    
    for (int i = 0; i < iterations; i++) {
        /* Unrollable sequence */
        a = a * 1.1 + b * 0.9;
        b = b * 0.95 + c * 1.05;
        c = c * 1.01 + a * 0.99;
        
        a = sin(a) + cos(b);
        b = tan(c) * 0.5 + a;
        c = exp(b * 0.1) + log(fabs(a) + 1.0);
        
        /* Memory access pattern */
        double temp = a;
        a = b;
        b = c;
        c = temp;
    }
    
    return a + b + c;
}

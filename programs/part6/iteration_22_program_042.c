/* File 1: compute-intensive.c - Floating point calculations with tight loops */
#include <math.h>
#include <stdlib.h>

/* Complex floating-point computation with data dependencies */
double compute_polynomial(int n, volatile double* input) {
    double result = 0.0;
    double coeff = 1.0;
    
    /* Outer loop creates scheduling pressure */
    for (int i = 0; i < n; i++) {
        double term = input[i];
        double power = term;
        
        /* Inner loop with mixed operations */
        for (int j = 0; j < 8; j++) {
            power = power * term;
            coeff = coeff * 0.95;
            term = term + sin(power * 0.01);
        }
        
        /* Conditional creates branch scheduling needs */
        if (term > 0) {
            result += term * coeff;
        } else {
            result -= fabs(term) * coeff;
        }
        
        /* Memory access pattern creates dependencies */
        coeff = coeff + input[i % 16] * 0.1;
    }
    
    return result;
}

/* Matrix multiplication with software pipelining opportunities */
void matrix_multiply(int size, double A[][64], double B[][64], double C[][64]) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            /* Innermost loop - prime candidate for software pipelining */
            for (int k = 0; k < size; k++) {
                sum += A[i][k] * B[k][j];
                /* Add some computation to create more scheduling opportunities */
                sum = sum + sin(sum * 0.0001);
            }
            C[i][j] = sum;
        }
    }
}

/* Function with irregular control flow */
double irregular_computation(int iterations) {
    double x = 0.5;
    double y = 0.3;
    double z = 0.7;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex conditional network */
        if (x > 0.5) {
            x = sin(y) * cos(z);
            if (y < 0.3) {
                y = tan(x) * 0.5;
            } else {
                y = atan(x) * 0.7;
            }
        } else if (x < -0.5) {
            x = cos(y) * sin(z);
            z = z * 0.9 + 0.1;
        } else {
            x = x * 0.8 + y * 0.2;
        }
        
        /* Switch-like behavior */
        int case_val = i % 4;
        switch (case_val) {
            case 0: z = exp(-x); break;
            case 1: z = log(fabs(x) + 1.0); break;
            case 2: z = sqrt(fabs(x)); break;
            case 3: z = pow(x, 1.5); break;
        }
        
        /* Memory barrier simulation */
        asm volatile("" ::: "memory");
    }
    
    return x + y + z;
}

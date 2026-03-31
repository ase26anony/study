/* test1.c - Floating-point intensive computation */
#include <math.h>

volatile double global_input = 3.14159;

double compute_polynomial(int iterations) {
    double result = 0.0;
    double x = global_input;
    
    /* Create scheduling pressure with mixed FP operations */
    for (int i = 0; i < iterations; i++) {
        /* Multiple dependent FP operations */
        double term1 = sin(x) * cos(x);
        double term2 = exp(-x * 0.1);
        double term3 = log(1.0 + fabs(x));
        
        /* Cross-iteration dependencies */
        result += term1 * term2 - term3;
        x = fmod(x * 1.1, 2.0 * M_PI);
        
        /* Conditional creates branch scheduling needs */
        if (result > 100.0) {
            result = fmod(result, 100.0);
        }
    }
    
    /* Nested loop for additional scheduling complexity */
    for (int j = 0; j < 10; j++) {
        double acc = 0.0;
        for (int k = 0; k < 50; k++) {
            acc += sin(x + j * 0.1 + k * 0.01);
        }
        result += acc;
    }
    
    return result;
}

/* Another function with different pattern */
double matrix_multiply_accumulate(int size) {
    volatile double sum = 0.0;
    
    /* Triple nested loop - creates many scheduling opportunities */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double acc = 0.0;
            for (int k = 0; k < size; k++) {
                /* Complex addressing calculations */
                double a = sin(i * 0.1 + k * 0.01);
                double b = cos(k * 0.1 + j * 0.01);
                acc += a * b;
                
                /* Conditional inside innermost loop */
                if (acc > 1.0e6) {
                    acc *= 0.5;
                }
            }
            sum += acc;
        }
    }
    
    return sum;
}

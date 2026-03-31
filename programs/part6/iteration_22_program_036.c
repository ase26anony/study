/* test1.c - Floating-point intensive with modulo scheduling */
#include <math.h>

volatile double input = 3.14159;

double compute_polynomial(int iterations) {
    double result = 0.0;
    double x = input;
    
    /* Complex loop with data dependencies - good for software pipelining */
    for (int i = 0; i < iterations; i++) {
        /* Mixed operations creating RAW dependencies */
        double term1 = x * x * x;
        double term2 = sin(x) * cos(x);
        double term3 = exp(-x * 0.1);
        
        /* Cross-iteration dependency */
        result += term1 * term2 - term3;
        
        /* Update x with feedback */
        x = fmod(x * 1.1 + result * 0.01, 10.0);
        
        /* Conditional creates control flow complexity */
        if (x > 5.0) {
            result *= 0.95;
        } else {
            result += 0.05;
        }
    }
    
    /* Nested loop for additional scheduling pressure */
    for (int j = 0; j < 100; j++) {
        double acc = 0.0;
        for (int k = 0; k < 50; k++) {
            acc += sin(j * 0.1 + k * 0.01) * cos(k * 0.02);
        }
        result += acc * 0.001;
    }
    
    return result;
}

/* Another function with different pattern */
double matrix_multiply(int size) {
    volatile double sum = 0.0;
    
    /* Triple nested loop - creates large basic blocks */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double dot = 0.0;
            for (int k = 0; k < size; k++) {
                double a = sin(i * 0.1 + k * 0.05);
                double b = cos(k * 0.1 + j * 0.05);
                dot += a * b;
            }
            sum += dot;
        }
    }
    
    return sum;
}

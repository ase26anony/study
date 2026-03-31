/* test1.c - Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Complex floating-point computation with data dependencies */
double compute_heat_transfer(int size, double* matrix) {
    volatile double result = 0.0;  /* Prevent optimization */
    int i, j, k;
    
    /* Triple nested loop with mixed operations */
    for (i = 1; i < size - 1; i++) {
        for (j = 1; j < size - 1; j++) {
            double sum = 0.0;
            for (k = 0; k < 4; k++) {
                /* Create scheduling pressure with mixed FP ops */
                double diff = matrix[(i-1)*size + j] - matrix[i*size + (j-1)];
                sum += diff * diff * 0.25;
                
                /* More complex FP chain */
                double temp = sin(matrix[i*size + j] * 0.01);
                sum += temp * temp * 0.1;
                
                /* Integer to FP conversion */
                sum += (double)(i * j) * 0.001;
            }
            matrix[i*size + j] = matrix[i*size + j] * 0.8 + sum * 0.2;
            result += matrix[i*size + j];
        }
    }
    
    /* Additional loop with branches */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            if (matrix[i*size + j] > 0.5) {
                result += sqrt(matrix[i*size + j]);
            } else {
                result -= matrix[i*size + j] * matrix[i*size + j];
            }
        }
    }
    
    return result;
}

/* Function with software pipelining opportunities */
void process_signal(int n, double* input, double* output) {
    int i;
    double history[4] = {0.0, 0.0, 0.0, 0.0};
    
    /* Loop with carried dependencies - good for modulo scheduling */
    for (i = 0; i < n; i++) {
        double filtered = input[i] * 0.4 
                        + history[0] * 0.3 
                        + history[1] * 0.2 
                        + history[2] * 0.1;
        
        /* Shift history */
        history[2] = history[1];
        history[1] = history[0];
        history[0] = filtered;
        
        /* Nonlinear transformation */
        output[i] = tanh(filtered * 0.5);
        
        /* Conditional operation */
        if (i % 32 == 0) {
            output[i] *= 1.1;  /* Boost every 32nd sample */
        }
    }
}

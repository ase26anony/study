/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Volatile to prevent optimization */
volatile double input_data[SIZE];
volatile double output_data[SIZE];

/* Complex floating-point function with scheduling pressure */
void compute_transform(int iterations) {
    double temp[SIZE];
    int i, j, k;
    
    /* Initialize with some values */
    for (i = 0; i < SIZE; i++) {
        input_data[i] = (double)i * 0.01;
    }
    
    /* Multiple nested loops with mixed operations */
    for (k = 0; k < iterations; k++) {
        /* First computation phase */
        for (i = 0; i < SIZE - 1; i++) {
            /* Mixed FP operations creating dependencies */
            double a = input_data[i];
            double b = input_data[i + 1];
            double c = sin(a) * cos(b);
            double d = exp(a) / (fabs(b) + 1.0);
            temp[i] = c * d + sqrt(a * a + b * b);
        }
        
        /* Second phase with different pattern */
        for (i = 1; i < SIZE; i++) {
            /* More complex FP chain */
            double prev = temp[i - 1];
            double curr = temp[i];
            double t1 = prev * prev - curr * curr;
            double t2 = log(fabs(prev) + 1.0) * atan(curr);
            output_data[i] = t1 * t2 + (prev + curr) / 2.0;
            
            /* Conditional creates branch scheduling pressure */
            if (output_data[i] > 100.0) {
                output_data[i] = 100.0;
            } else if (output_data[i] < -100.0) {
                output_data[i] = -100.0;
            }
        }
        
        /* Third phase with reduction */
        double sum = 0.0;
        for (i = 0; i < SIZE; i += 4) {
            /* Unrolled loop for scheduling */
            sum += output_data[i] * 0.1;
            sum += output_data[i + 1] * 0.2;
            sum += output_data[i + 2] * 0.3;
            sum += output_data[i + 3] * 0.4;
        }
        
        /* Update input for next iteration */
        for (i = 0; i < SIZE; i++) {
            input_data[i] = output_data[i] * 0.9 + input_data[i] * 0.1;
        }
    }
}

/* Entry point for test 1 */
double test_floating_point(int argc, char **argv) {
    int iterations = ITERATIONS;
    
    /* Use command line to vary iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]) % 200 + 50;
    }
    
    compute_transform(iterations);
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += output_data[i];
    }
    
    return checksum;
}

/* Test 1: Floating-point intensive computation with nested loops */
#include <stdlib.h>
#include <math.h>

#define SIZE 256
#define ITERS 1000

volatile double input[SIZE];
volatile double output[SIZE];

void compute_transform(int iterations) {
    for (int iter = 0; iter < iterations; iter++) {
        // Multiple nested loops create scheduling pressure
        for (int i = 0; i < SIZE; i++) {
            double sum = 0.0;
            // Inner loop with data dependencies
            for (int j = 0; j < 8; j++) {
                // Mixed operations: FP multiply, add, trig functions
                sum += sin(input[i] * j) * cos(input[i] / (j + 1));
            }
            // Conditional store creates branch scheduling
            if (sum > 0.0) {
                output[i] = sum * 0.5 + input[i];
            } else {
                output[i] = -sum * 0.3 + input[i];
            }
        }
        
        // Another loop with different pattern
        for (int i = 1; i < SIZE - 1; i++) {
            // Array access with dependencies
            output[i] = (output[i-1] + output[i] + output[i+1]) / 3.0;
        }
    }
}

void init_data() {
    for (int i = 0; i < SIZE; i++) {
        input[i] = (double)(i % 100) * 0.01;
    }
}

int main_test1(int argc, char **argv) {
    init_data();
    int iters = (argc > 1) ? atoi(argv[1]) : ITERS;
    compute_transform(iters);
    return 0;
}

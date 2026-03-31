/* File 1: Floating-point intensive computation with nested loops */
#include <math.h>

#define SIZE 256
#define ITERS 1000

volatile double input[SIZE];
volatile double output[SIZE];

/* Function with tight floating-point loop - creates scheduling pressure */
void compute_transform(int iterations) {
    double temp[SIZE];
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Mixed operations: sin, cos, multiplication, addition */
        for (int i = 0; i < SIZE; i++) {
            double x = input[i];
            
            /* Create data dependencies between operations */
            double t1 = sin(x * 0.1);
            double t2 = cos(x * 0.2);
            double t3 = t1 * t2 + x;
            
            /* Conditional creates branch scheduling requirements */
            if (t3 > 0.5) {
                t3 = t3 * 0.8 + 0.2;
            } else {
                t3 = t3 * 1.2 - 0.1;
            }
            
            /* More arithmetic operations */
            for (int j = 0; j < 4; j++) {
                t3 = t3 + sin(t3 * j * 0.01);
            }
            
            output[i] = t3;
            temp[i] = t3;
        }
        
        /* Another loop with different pattern */
        for (int i = 1; i < SIZE - 1; i++) {
            output[i] = (temp[i-1] + temp[i] + temp[i+1]) / 3.0;
        }
    }
}

/* Helper function with loop unrolling candidate */
double vector_dot_product(const double* a, const double* b, int n) {
    double sum = 0.0;
    
    /* Loop with potential for unrolling and software pipelining */
    for (int i = 0; i < n; i += 4) {
        double s0 = a[i] * b[i];
        double s1 = a[i+1] * b[i+1];
        double s2 = a[i+2] * b[i+2];
        double s3 = a[i+3] * b[i+3];
        
        sum += s0 + s1 + s2 + s3;
    }
    
    return sum;
}

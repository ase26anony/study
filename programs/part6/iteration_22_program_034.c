/* File 1: Floating-point intensive computation with nested loops */
#include <math.h>

volatile double input1 = 3.14159;
volatile double input2 = 2.71828;

double compute_fp_intensive(int iterations) {
    double result = 0.0;
    double a = input1;
    double b = input2;
    
    /* Outer loop creates scheduling pressure */
    for (int i = 0; i < iterations; i++) {
        double temp = a;
        
        /* Inner loop with mixed FP operations */
        for (int j = 0; j < 100; j++) {
            temp = temp * b + sin(temp);
            temp = cos(temp) / (fabs(temp) + 1.0);
            
            /* Conditional creates branch scheduling needs */
            if (temp > 2.0) {
                temp = sqrt(temp);
            } else {
                temp = temp * temp;
            }
            
            result += temp;
        }
        
        a = result / (i + 1);
        b = b * 1.01;
    }
    
    return result;
}

/* Another function with different pattern */
void matrix_multiply(double A[10][10], double B[10][10], double C[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < 10; k++) {
                C[i][j] += A[i][k] * B[k][j];
                /* Add some conditional to create scheduling complexity */
                if (C[i][j] > 1000.0) {
                    C[i][j] = log(C[i][j]);
                }
            }
        }
    }
}

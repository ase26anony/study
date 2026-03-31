/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdlib.h>

/* Complex floating-point computation with data dependencies */
double compute_pressure(int n, double *input) {
    volatile double sum = 0.0;  /* volatile prevents optimization */
    double *temp = (double*)malloc(n * sizeof(double));
    
    if (!temp) return 0.0;
    
    /* Nested loops creating scheduling pressure */
    for (int i = 0; i < n; i++) {
        double acc = input[i];
        for (int j = 0; j < 8; j++) {
            /* Mixed operations: multiply, add, trigonometric */
            acc = sin(acc) * 1.2345 + cos(acc) * 0.9876;
            acc = acc * acc - sqrt(fabs(acc));
        }
        temp[i] = acc;
    }
    
    /* Another loop with conditional branches */
    for (int i = 1; i < n - 1; i++) {
        if (temp[i] > 0.5) {
            sum += temp[i] * temp[i-1] + temp[i+1];
        } else {
            sum -= temp[i] / (temp[i-1] + 0.001);
        }
    }
    
    free(temp);
    return sum;
}

/* Matrix multiplication with scheduling barriers */
void matrix_multiply(int size, double A[size][size], double B[size][size], 
                     double C[size][size]) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                /* Complex expression with multiple dependencies */
                sum += A[i][k] * B[k][j] + 
                       sin(A[i][k]) * cos(B[k][j]);
            }
            C[i][j] = sum;
        }
    }
}

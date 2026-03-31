/* test1.c - Floating-point intensive computations */
#include <math.h>

volatile double global_d = 3.14159;

double compute_pi_approximation(int iterations) {
    double sum = 0.0;
    int sign = 1;
    
    /* Create scheduling pressure with mixed operations */
    for (int i = 0; i < iterations; i++) {
        double term = 1.0 / (2 * i + 1);
        sum += sign * term;
        sign = -sign;
        
        /* Add some memory operations */
        global_d = global_d * 0.99 + term * 0.01;
    }
    
    return 4.0 * sum;
}

double matrix_multiply(int size) {
    double a[100][100];
    double b[100][100];
    double c[100][100];
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            a[i][j] = (i + j) * 0.01;
            b[i][j] = (i - j) * 0.02;
        }
    }
    
    /* Nested loops with data dependencies */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            c[i][j] = 0.0;
            for (int k = 0; k < size; k++) {
                c[i][j] += a[i][k] * b[k][j];
                /* Create complex scheduling graph */
                if (k % 2 == 0) {
                    c[i][j] += sin(c[i][j] * 0.1);
                } else {
                    c[i][j] -= cos(c[i][j] * 0.1);
                }
            }
        }
    }
    
    /* Reduce result */
    double total = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            total += c[i][j];
        }
    }
    
    return total;
}

void test1_operations(int n) {
    double pi_approx = compute_pi_approximation(n);
    double matrix_result = matrix_multiply(n % 50 + 10);
    
    /* Use results to prevent optimization */
    global_d = pi_approx + matrix_result;
}

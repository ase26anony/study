/* Test 1: Tight floating-point loop for software pipelining */
#include <math.h>

#define SIZE 1024

void compute_pressure(double *a, double *b, double *c, int n) {
    volatile int start = n;  /* Prevent optimization */
    
    /* Complex loop with data dependencies */
    for (int i = 0; i < n; i++) {
        /* Multiple dependent FP operations */
        double t1 = a[i] * b[i];
        double t2 = sin(t1) + cos(a[i]);
        double t3 = t2 * t2 - sqrt(fabs(b[i]));
        double t4 = t3 / (t1 + 1.0);
        c[i] = t4 * exp(t3);
        
        /* Conditional creates control flow */
        if (c[i] > 100.0) {
            c[i] = log(fabs(c[i]));
        } else {
            c[i] = pow(c[i], 1.5);
        }
    }
}

/* Nested loops with varying bounds */
void matrix_transform(double mat[SIZE][SIZE], int n) {
    for (int i = 1; i < n - 1; i++) {
        for (int j = 1; j < n - 1; j++) {
            /* Stencil computation with dependencies */
            mat[i][j] = (mat[i-1][j] + mat[i+1][j] + 
                        mat[i][j-1] + mat[i][j+1]) * 0.25;
            
            /* Additional computation to increase pressure */
            for (int k = 0; k < 4; k++) {
                mat[i][j] += sin(mat[i][j] * k) * 0.1;
            }
        }
    }
}

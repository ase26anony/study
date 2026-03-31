/* test1.c - Floating-point intensive with nested loops */
#include <math.h>
#include <stdio.h>

volatile double input = 3.14159;
volatile int iterations = 1000;

double compute_intensive(double base, int n) {
    double result = 0.0;
    double temp;
    int i, j, k;
    
    /* Triple nested loop creates scheduling pressure */
    for (i = 0; i < n; i++) {
        temp = base;
        for (j = 0; j < 50; j++) {
            /* Mixed FP operations */
            temp = sin(temp) * cos(temp) + tan(temp/2.0);
            
            /* Inner loop with data dependencies */
            for (k = 0; k < 10; k++) {
                temp = temp * 1.01 - 0.5;
                if (temp > 100.0) {
                    temp = temp / 2.0;
                } else if (temp < -100.0) {
                    temp = temp * (-0.9);
                }
            }
        }
        result += temp;
        
        /* Branch creates control flow for scheduler */
        if (i % 7 == 0) {
            result = sqrt(fabs(result));
        } else if (i % 13 == 0) {
            result = pow(result, 1.5);
        }
    }
    
    return result;
}

/* Another function with different pattern */
void matrix_operations(double *a, double *b, double *c, int size) {
    int i, j, k;
    
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            double sum = 0.0;
            for (k = 0; k < size; k++) {
                /* Memory accesses create scheduling constraints */
                sum += a[i * size + k] * b[k * size + j];
            }
            c[i * size + j] = sum;
            
            /* Conditional store */
            if (c[i * size + j] < 0) {
                c[i * size + j] = -c[i * size + j];
            }
        }
    }
}

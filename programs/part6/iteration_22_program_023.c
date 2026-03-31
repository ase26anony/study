/* Complex floating-point calculations with data dependencies */
#include <math.h>

volatile double input1 = 3.14159;
volatile double input2 = 2.71828;

double compute_intensive(int iterations) {
    double result = 0.0;
    double a = input1;
    double b = input2;
    
    /* Nested loops with mixed operations */
    for (int i = 0; i < iterations; i++) {
        double temp = a;
        for (int j = 0; j < 100; j++) {
            /* Create scheduling pressure with FP operations */
            temp = sin(temp) * cos(b) + tan(a * b);
            temp = sqrt(fabs(temp)) + log(fabs(temp) + 1.0);
            
            /* Conditional creates branch scheduling needs */
            if (temp > 0) {
                temp = temp * 0.5;
            } else {
                temp = temp * 0.25;
            }
        }
        result += temp;
        a = result * 0.9;
        b = result * 1.1;
    }
    
    return result;
}

/* Function with software pipelining opportunities */
double matrix_multiply(int size) {
    double mat1[100][100];
    double mat2[100][100];
    double result[100][100];
    
    /* Initialize matrices */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            mat1[i][j] = i * 0.1 + j * 0.01;
            mat2[i][j] = i * 0.01 - j * 0.1;
        }
    }
    
    /* Triple nested loop - ideal for modulo scheduling */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0.0;
            for (int k = 0; k < size; k++) {
                /* Complex expression with dependencies */
                sum += mat1[i][k] * mat2[k][j] 
                     + sin(mat1[i][k]) * cos(mat2[k][j]);
            }
            result[i][j] = sum;
            
            /* Conditional store */
            if (result[i][j] < 0) {
                result[i][j] = -result[i][j];
            }
        }
    }
    
    /* Compute final result */
    double final = 0.0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            final += result[i][j] * (i + j);
        }
    }
    
    return final;
}

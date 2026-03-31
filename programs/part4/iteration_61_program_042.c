/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be preserved through optimization */
static volatile int sink __attribute__((used));

/* Complex loop with multiple carried dependencies */
double complex_recurrence(double *arr1, double *arr2, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum / 2.71828 * arr1[i] + arr2[i-1];
        
        /* Another distance-1 dependency with multiplication */
        prod = prod * (sum + 1.0) / 1.414;
        
        /* Mix in some expensive operations to create resource pressure */
        x = x * cos(prod * 0.01) + sin(sum * 0.01);
        
        /* Store intermediate results to prevent optimization */
        arr1[i] = x;
        arr2[i] = sum + prod;
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependency chain */
int integer_recurrence(int *data, int n) {
    int acc1 = data[0];
    int acc2 = 1;
    int acc3 = 2;
    
    /* Loop with multiple interdependent carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Chain of distance-1 dependencies */
        acc1 = acc1 * 3 + data[i];
        acc2 = acc2 + acc1 / 7;
        acc3 = acc3 * 5 - acc2;
        
        /* Create anti-dependencies by reusing variables */
        data[i-1] = acc3;
    }
    
    return acc1 + acc2 + acc3;
}

/* Loop with array-based carried dependencies */
float array_recurrence(float *a, float *b, float *c, int n) {
    float result = 0.0f;
    
    /* Classic recurrence pattern: a[i] depends on a[i-1] */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency through array */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Additional computation with potential resource contention */
        b[i] = b[i] / 1.234f * a[i];
        c[i] = sqrtf(fabsf(c[i] + a[i]));
        
        result += a[i] + b[i] + c[i];
    }
    
    return result;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements 
     * to prevent aggressive optimization */
    double arr1[N] __attribute__((used));
    double arr2[N] __attribute__((used));
    int int_data[M] __attribute__((used));
    float float_a[M] __attribute__((used));
    float float_b[M] __attribute__((used));
    float float_c[M] __attribute__((used));
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 0.1 + 1.0;
        arr2[i] = sin(i * 0.05) + 2.0;
    }
    
    for (int i = 0; i < M; i++) {
        int_data[i] = i * 3 - 7;
        float_a[i] = i * 0.2f;
        float_b[i] = cos(i * 0.1f) + 1.0f;
        float_c[i] = i * 0.3f + 0.5f;
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = complex_recurrence(arr1, arr2, N);
    int result2 = integer_recurrence(int_data, M);
    float result3 = array_recurrence(float_a, float_b, float_c, M);
    
    /* Force all results to be used to prevent dead code elimination */
    sink = result2;
    
    /* Print results to create observable side effects */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    
    /* Additional volatile store to ensure loops aren't optimized away */
    volatile double final_check = result1 + result2 + result3;
    
    return (int)(final_check * 0.01);
}

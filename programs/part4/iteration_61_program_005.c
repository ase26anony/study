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
        prod = prod * (sum + 1.0) / 3.14159;
        
        /* Mix in some expensive operations to create resource pressure */
        x = x * cos(prod * 0.01) / sin(sum * 0.01);
        
        /* Cross-iteration dependency through array */
        arr1[i] = arr1[i-1] * 0.99 + x;
    }
    
    return sum + prod + x;
}

/* Integer loop with carried dependency and resource constraints */
int integer_recurrence(int *data, int n) {
    int acc = data[0];
    int temp = 1;
    
    /* Loop with distance-1 dependencies and integer multiplies */
    for (int i = 1; i < n; i++) {
        /* Carried dependency chain */
        acc = acc * 37 + data[i];
        
        /* Another dependency to create complexity */
        temp = (temp * acc) >> 3;
        
        /* Use result in next iteration */
        data[i] = (data[i-1] + temp) & 0xFF;
    }
    
    return acc + temp;
}

/* Main function with observable side effects */
int main(void) {
    /* Declare and initialize arrays with volatile elements to prevent optimization */
    double arr1[N] __attribute__((used));
    double arr2[N] __attribute__((used));
    int int_data[M] __attribute__((used));
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = (double)(i % 37) * 0.1;
        arr2[i] = (double)(i % 23) * 0.2;
    }
    
    for (int i = 0; i < M; i++) {
        int_data[i] = i * 3;
    }
    
    /* Execute loops with carried dependencies */
    double result1 = complex_recurrence(arr1, arr2, N);
    int result2 = integer_recurrence(int_data, M);
    
    /* Force side effects to prevent dead code elimination */
    sink = result2;
    
    /* Print results to ensure loops aren't optimized away */
    printf("Results: %f, %d\n", result1, result2);
    
    /* Use results in a way that can't be optimized out */
    if (sink > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return 0;
}

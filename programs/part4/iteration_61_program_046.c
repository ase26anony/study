/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Force variables to be kept and prevent optimization */
static volatile int sink;

/* Complex loop with multiple carried dependencies */
double complex_dependency_loop(double *a, double *b, double *c, int n) {
    double sum = 1.0;
    double prod = 2.0;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] * 3.14159 + b[i];
        
        /* Another distance-1 dependency with division (high latency) */
        c[i] = c[i-1] / 2.71828 * a[i];
        
        /* Cross-dependency between different variables */
        sum = sum + a[i] * c[i];  /* sum has self-dependency */
        prod = prod * (sum + 1.0); /* prod has self-dependency */
    }
    
    return sum + prod;
}

/* Integer loop with artificial resource contention */
int integer_resource_loop(int *arr1, int *arr2, int n) {
    int acc1 = arr1[0];
    int acc2 = arr2[0];
    int temp;
    
    /* Loop with multiple integer operations that compete for ALU */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependencies */
        acc1 = acc1 * 7 + arr1[i];      /* Multiplication is multi-cycle */
        acc2 = acc2 / 3 + arr2[i];      /* Division is high latency */
        
        /* Cross-iteration dependency chain */
        temp = acc1 - acc2;
        arr1[i] = temp * arr1[i-1];     /* Another distance-1 dependency */
        arr2[i] = arr2[i-1] + temp;     /* Yet another distance-1 */
    }
    
    return acc1 + acc2;
}

/* Mixed-type loop to stress the scheduler */
float mixed_operations(float *farr, double *darr, int n) {
    float fsum = farr[0];
    double dprod = darr[0];
    
    for (int i = 1; i < n; i++) {
        /* Type conversions add complexity */
        fsum = fsum + (float)dprod * farr[i];
        
        /* High-latency double operations */
        dprod = dprod * (1.0 + sin(darr[i-1]));  /* Distance-1 with math function */
        
        /* Cross-type dependency */
        farr[i] = fsum * (float)dprod;
        darr[i] = darr[i-1] + (double)farr[i];  /* Another distance-1 */
    }
    
    return fsum;
}

int main(void) {
    /* Declare and initialize arrays with volatile elements 
     * to prevent dead code elimination */
    volatile double a[N], b[N], c[N];
    volatile int arr1[M], arr2[M];
    volatile float farr[N];
    volatile double darr[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        farr[i] = i * 0.4f;
        darr[i] = i * 0.5;
    }
    
    for (int i = 0; i < M; i++) {
        arr1[i] = i * 2;
        arr2[i] = i * 3;
    }
    
    /* Execute loops with different dependency patterns */
    double result1 = complex_dependency_loop((double*)a, (double*)b, (double*)c, N);
    int result2 = integer_resource_loop((int*)arr1, (int*)arr2, M);
    float result3 = mixed_operations((float*)farr, (double*)darr, N);
    
    /* Force results to be used to prevent optimization */
    sink = (int)result1 + result2 + (int)result3;
    
    /* Print results to create observable side effect */
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    printf("Sink: %d\n", sink);
    
    return 0;
}

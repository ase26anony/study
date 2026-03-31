/* test_modulo_sched.c
 * Designed to trigger modulo scheduler debug output in GCC's RTL optimizer
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define ITERS 128

/* Force variables to be used and prevent optimization */
static volatile int sink __attribute__((used));

/* Complex loop with multiple carried dependencies */
double complex_recurrence(double *arr1, double *arr2, int n) {
    double sum = 1.0;
    double prod = 2.0;
    double x = 3.14159;
    
    /* Loop 1: Multiple carried dependencies with high-latency operations */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency: sum[i] depends on sum[i-1] */
        sum = sum + arr1[i] * arr2[i];
        
        /* Another distance-1 dependency with floating-point division */
        x = x / 3.1415926535 * (sum + 1.0);
        
        /* Cross dependency: prod depends on both sum and previous prod */
        prod = prod * (x + 0.5);
        
        /* Array-based carried dependency */
        arr1[i] = arr1[i-1] * 1.01 + arr2[i];
    }
    
    return sum + prod + x;
}

/* Integer loop with artificial resource contention */
int integer_recurrence(int *arr, int n) {
    int acc = 1;
    int mult = 2;
    
    /* Loop 2: Integer operations with carried dependencies */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependency chain */
        acc = acc * 3 + arr[i];
        
        /* Another carried dependency competing for multiplier units */
        mult = (mult * 7) ^ acc;
        
        /* Array recurrence */
        arr[i] = arr[i-1] + (acc % 17);
    }
    
    return acc + mult;
}

/* Mixed-type loop with complex dependency web */
float mixed_dependencies(float *farr, int *iarr, int n) {
    float f1 = 1.5f;
    float f2 = 2.5f;
    int i1 = 1;
    
    /* Loop 3: Mixed operations creating scheduling pressure */
    for (int j = 1; j < n; j++) {
        /* Floating-point division (high latency) with carried dependency */
        f1 = f1 / 1.7f + farr[j];
        
        /* Integer multiplication with carried dependency */
        i1 = i1 * 3 + iarr[j];
        
        /* Cross-type dependency */
        f2 = f2 * (float)i1 + f1;
        
        /* Array carried dependency */
        farr[j] = farr[j-1] * 0.99f + f2;
        
        /* Another integer recurrence */
        iarr[j] = iarr[j-1] + (int)(f1 * 10);
    }
    
    return f1 + f2;
}

int main(void) {
    /* Initialize arrays with volatile elements to prevent optimization */
    double arr1[N], arr2[N];
    int iarr1[N], iarr2[N];
    float farr[N];
    
    /* Initialize arrays with non-trivial patterns */
    for (int i = 0; i < N; i++) {
        arr1[i] = sin(i * 0.1) + 1.0;
        arr2[i] = cos(i * 0.05) + 2.0;
        iarr1[i] = (i * 17) % 23;
        iarr2[i] = (i * 13) % 31;
        farr[i] = (float)i * 0.25f;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    
    /* Execute loops multiple times to increase scheduling pressure */
    for (int iter = 0; iter < ITERS; iter++) {
        result1 += complex_recurrence(arr1, arr2, N);
        result2 += integer_recurrence(iarr1, N);
        result3 += mixed_dependencies(farr, iarr2, N);
        
        /* Shuffle array elements slightly to vary dependencies */
        for (int i = 1; i < N; i++) {
            arr1[i] += 0.001 * i;
            iarr1[i] ^= 1;
            farr[i] *= 1.0001f;
        }
    }
    
    /* Force all results to be used and observable */
    sink = result2;
    
    printf("Results: %f, %d, %f\n", result1, result2, result3);
    
    return 0;
}

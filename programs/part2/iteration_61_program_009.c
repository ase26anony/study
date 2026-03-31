/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test_recurrence_fp(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;      /* Intra-iteration dependence */
        sum += arr[i];               /* Reduction */
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_latency(int *arr, double *farr, int n, int step) {
    int total = 0;
    /* Mixed integer/floating-point with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer recurrence */
        arr[i] = arr[i-1] * 3 + step;
        
        /* Floating-point operation with higher latency */
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        
        /* Memory access with potential aliasing */
        if (i > 1) {
            arr[i] += arr[i-2];      /* Distance-2 dependence */
        }
        
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline))
float test_pointer_chase(float **ptrs, float *data, int n) {
    float result = 0.0f;
    /* Pointer chasing with distance-1 dependence */
    for (int i = 0; i < n-1; i++) {
        /* Set up pointer for next iteration */
        ptrs[i+1] = &data[i];
        
        /* Use pointer from current/previous iteration */
        if (i > 0) {
            *ptrs[i] = *ptrs[i-1] * 2.0f + data[i];
        }
        
        result += *ptrs[i];
    }
    return result;
}

__attribute__((noinline))
long test_complex_chain(long *a, long *b, long *c, int n) {
    long sum = 0;
    /* Complex chain with multiple dependences */
    for (int i = 2; i < n; i++) {
        /* Multiple distance-1 dependences */
        a[i] = a[i-1] + b[i-2];      /* Mixed distances */
        b[i] = a[i] * 3 - b[i-1];    /* Distance-1 */
        c[i] = c[i-1] + a[i-1] * b[i]; /* Multiple inputs */
        
        /* Memory operations that may alias */
        if (i % 4 == 0) {
            a[i] += c[i-2];
        }
        
        sum += a[i] + b[i] - c[i];
    }
    return sum;
}

__attribute__((noinline))
double test_nested_dependences(double *x, double *y, int n, double alpha) {
    double total = 0.0;
    /* Nested calculations with varied latencies */
    for (int i = 3; i < n; i++) {
        /* Chain of dependences across iterations */
        double t1 = x[i-1] * alpha;
        double t2 = t1 + y[i-2];
        x[i] = t2 * t2 - x[i-3];     /* Distance-3 */
        y[i] = y[i-1] + x[i] * 0.5;  /* Distance-1 */
        
        /* Additional FP operations */
        y[i] = y[i] * 1.1 + 0.01;
        
        total += x[i] + y[i];
    }
    return total;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    int *arr2 = (int*)malloc(n * sizeof(int));
    double *arr3 = (double*)malloc(n * sizeof(double));
    long *arr4 = (long*)malloc(n * sizeof(long));
    long *arr5 = (long*)malloc(n * sizeof(long));
    long *arr6 = (long*)malloc(n * sizeof(long));
    double *arr7 = (double*)malloc(n * sizeof(double));
    double *arr8 = (double*)malloc(n * sizeof(double));
    float **ptrs = (float**)malloc(n * sizeof(float*));
    float *data = (float*)malloc(n * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 75) * 0.2;
        arr4[i] = i % 30;
        arr5[i] = i % 40;
        arr6[i] = i % 60;
        arr7[i] = (double)(i % 80) * 0.15;
        arr8[i] = (double)(i % 90) * 0.25;
        data[i] = (float)i * 0.3f;
        ptrs[i] = &data[i % 10];
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    long result4 = 0;
    double result5 = 0.0;
    
    /* Call test functions multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        result1 += test_recurrence_fp(arr1, n, 1.05 + iter*0.01);
        result2 += test_mixed_latency(arr2, arr3, n, iter + 1);
        result3 += test_pointer_chase(ptrs, data, n);
        result4 += test_complex_chain(arr4, arr5, arr6, n);
        result5 += test_nested_dependences(arr7, arr8, n, 0.95 + iter*0.02);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %.2f, %d, %.2f, %ld, %.2f\n", 
           result1, result2, result3, result4, result5);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    free(arr6);
    free(arr7);
    free(arr8);
    free(ptrs);
    free(data);
    
    return 0;
}

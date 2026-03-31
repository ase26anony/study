/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int total = 0;
    /* Mixed integer/FP with loop-carried dependence */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        arr[i] = (int)farr[i] * 3 + arr[i-1];
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence with memory latency */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];
        result += *ptrs[i];
    }
    return result;
}

__attribute__((noinline))
static long test4_complex_chain(long *a, long *b, double *c, int n) {
    long acc = 0;
    /* Multiple interleaved dependences with different latencies */
    for (int i = 1; i < n; i++) {
        c[i] = c[i-1] * 0.99 + (double)a[i];      // FP mult + load
        a[i] = a[i-1] + (long)(c[i] * 100.0);     // FP->int conv + add
        b[i] = b[i-1] * a[i] + i;                 // integer multiply
        acc += a[i] + b[i] + (long)c[i];
    }
    return acc;
}

__attribute__((noinline))
static double test5_memory_aliasing(double *arr1, double *arr2, int n) {
    double sum = 0.0;
    /* Potential aliasing creates conservative memory dependences */
    for (int i = 1; i < n; i++) {
        arr1[i] = arr1[i-1] + arr2[i] * 2.0;      // Load from arr2
        arr2[i] = arr1[i] * 0.5 + (double)i;      // Store to arr2
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    double *arr2 = (double*)malloc(n * sizeof(double));
    int *iarr = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    long *larr1 = (long*)malloc(n * sizeof(long));
    long *larr2 = (long*)malloc(n * sizeof(long));
    float **ptrs = (float**)malloc(n * sizeof(float*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        iarr[i] = i % 25;
        farr[i] = (float)i * 0.3f;
        larr1[i] = i % 30;
        larr2[i] = i % 40;
        ptrs[i] = &farr[i];
    }
    
    /* Call test functions to trigger modulo scheduling analysis */
    double sum1 = test1_fp_recurrence(arr1, n, 1.05);
    int sum2 = test2_mixed_latency(iarr, arr2, n);
    float sum3 = test3_pointer_chase(ptrs, farr, n);
    long sum4 = test4_complex_chain(larr1, larr2, arr1, n);
    double sum5 = test5_memory_aliasing(arr1, arr2, n);
    
    /* Aggregate results to prevent dead code elimination */
    double total = sum1 + sum2 + sum3 + sum4 + sum5;
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr);
    free(farr);
    free(larr1);
    free(larr2);
    free(ptrs);
    
    return 0;
}

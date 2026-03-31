/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop-carried FP recurrence with distance=1 */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + arr[i] * 0.75;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int total = 0;
    /* Mixed integer/FP with memory dependencies */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        arr[i] = (int)farr[i] * 3 + arr[i-1];
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating distance=1 dependence */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 1.5f + data[i];
        sum += *ptrs[i+1];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long *a, long *b, long *c, int n) {
    long acc = 0;
    /* Multiple interleaved recurrences with different distances */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] * b[i-2] + c[i];
        b[i] = a[i-1] + b[i-1] * 7;
        c[i] = c[i-1] * 3 - a[i-2];
        acc += a[i] + b[i] - c[i];
    }
    return acc;
}

__attribute__((noinline))
static double test5_memory_aliasing(double *arr1, double *arr2, int n) {
    double sum = 0.0;
    /* Potential aliasing creates conservative memory dependencies */
    for (int i = 1; i < n; i++) {
        arr1[i] = arr2[i-1] * 2.5 + arr1[i];
        arr2[i] = arr1[i-1] * 0.8 + arr2[i];
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
    float **ptrs = (float**)malloc(n * sizeof(float*));
    long *la = (long*)malloc(n * sizeof(long));
    long *lb = (long*)malloc(n * sizeof(long));
    long *lc = (long*)malloc(n * sizeof(long));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i % 10) * 0.1;
        arr2[i] = (i % 7) * 0.2;
        iarr[i] = i * 3;
        farr[i] = (float)i * 0.5f;
        ptrs[i] = &farr[i];
        la[i] = i * 2;
        lb[i] = i * 3;
        lc[i] = i * 5;
    }
    
    /* Call test functions to trigger modulo scheduling */
    double sum1 = test1_fp_recurrence(arr1, n, 1.05);
    int sum2 = test2_mixed_latency(iarr, arr2, n);
    float sum3 = test3_pointer_chase(ptrs, farr, n);
    long sum4 = test4_complex_chain(la, lb, lc, n);
    double sum5 = test5_memory_aliasing(arr1, arr2, n);
    
    /* Aggregate results to prevent dead code elimination */
    double total = sum1 + sum2 + sum3 + sum4 + sum5;
    printf("Total: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr);
    free(farr);
    free(ptrs);
    free(la);
    free(lb);
    free(lc);
    
    return 0;
}

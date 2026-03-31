/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with loop-carried dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_recurrence(int *arr, double *farr, int n) {
    int sum = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        arr[i] = (int)farr[i] * 3 + arr[i-1];
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        float val = *ptrs[i];
        data[i+1] = val * 2.5f + (float)i;
        ptrs[i+1] = &data[i+1];
        sum += val;
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long *a, long *b, double *c, int n) {
    long total = 0;
    /* Complex chain with multiple dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence on a */
        a[i] = a[i-1] * 7 + b[i];
        
        /* Distance-1 dependence on c with FP operation */
        c[i] = c[i-1] * 1.001 + (double)a[i];
        
        /* Intra-iteration dependence + distance-1 on b */
        b[i] = (long)(c[i] * 100.0) + b[i-1] / 3;
        
        total += a[i] + b[i];
    }
    return total;
}

__attribute__((noinline))
static double test5_multi_access(double *arr1, double *arr2, int n) {
    double sum = 0.0;
    /* Multiple memory accesses with potential aliasing */
    for (int i = 1; i < n; i++) {
        /* Two distance-1 dependences */
        arr1[i] = arr1[i-1] * 1.1 + arr2[i-1];
        arr2[i] = arr2[i-1] * 0.9 + arr1[i];
        
        /* Additional operations to increase DDG complexity */
        double temp = arr1[i] * arr2[i];
        arr1[i] += temp * 0.5;
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    if (n < 10) n = 10;  /* Ensure minimum size */
    if (n > 10000) n = 10000;  /* Limit for safety */
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    double *arr2 = (double*)malloc(n * sizeof(double));
    int *iarr = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    float **ptrs = (float**)malloc(n * sizeof(float*));
    long *larr1 = (long*)malloc(n * sizeof(long));
    long *larr2 = (long*)malloc(n * sizeof(long));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        iarr[i] = i * 3;
        farr[i] = (float)i * 0.5f;
        ptrs[i] = &farr[i];
        larr1[i] = i * 2L;
        larr2[i] = i * 3L;
    }
    
    double total = 0.0;
    
    /* Call test functions - each with different recurrence patterns */
    total += test1_fp_recurrence(arr1, n, 1.05);
    total += (double)test2_mixed_recurrence(iarr, arr2, n);
    total += (double)test3_pointer_chase(ptrs, farr, n);
    total += (double)test4_complex_chain(larr1, larr2, arr1, n);
    total += test5_multi_access(arr1, arr2, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr);
    free(farr);
    free(ptrs);
    free(larr1);
    free(larr2);
    
    return 0;
}

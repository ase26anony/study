/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
static double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int* arr, double* farr, int n) {
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
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence with memory latency */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 0.9f + data[i];
        sum += *ptrs[i+1];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long* a, long* b, double* c, int n) {
    long result = 0;
    /* Multiple interleaved dependences with different latencies */
    for (int i = 1; i < n; i++) {
        c[i] = c[i-1] * 2.5 + (double)a[i];      // FP mult + load
        a[i] = a[i-1] + (long)(c[i] * 10.0);     // FP-to-int + add
        b[i] = b[i-1] * a[i] + i;                // integer multiply
        result += a[i] + b[i] + (long)c[i];
    }
    return result;
}

__attribute__((noinline))
static double test5_nested_dep(double* arr1, double* arr2, int n) {
    double acc = 0.0;
    /* Two separate recurrence chains */
    for (int i = 2; i < n; i++) {
        arr1[i] = arr1[i-1] * 1.1 + arr1[i-2] * 0.9;
        arr2[i] = arr2[i-1] * 0.8 + arr1[i] * 0.5;
        acc += arr1[i] + arr2[i];
    }
    return acc;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    double* arr2 = (double*)malloc(n * sizeof(double));
    int* iarr = (int*)malloc(n * sizeof(int));
    long* larr1 = (long*)malloc(n * sizeof(long));
    long* larr2 = (long*)malloc(n * sizeof(long));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* fdata = (float*)malloc(n * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        iarr[i] = i * 3;
        larr1[i] = i * 5L;
        larr2[i] = i * 7L;
        fdata[i] = (float)i * 0.3f;
        ptrs[i] = &fdata[i];
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_recurrence_fp(arr1, n, 1.05);
    total += (double)test2_mixed_latency(iarr, arr2, n);
    total += (double)test3_pointer_chase(ptrs, fdata, n);
    total += (double)test4_complex_chain(larr1, larr2, arr1, n);
    total += test5_nested_dep(arr1, arr2, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr);
    free(larr1);
    free(larr2);
    free(ptrs);
    free(fdata);
    
    return 0;
}

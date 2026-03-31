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
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
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
    /* Pointer-based recurrence with memory loads */
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
    long acc = 0;
    /* Multiple interleaved recurrences with different latencies */
    for (int i = 1; i < n; i++) {
        /* FP recurrence chain */
        c[i] = c[i-1] * 1.5 + (double)a[i];
        
        /* Integer recurrence with multiplication (higher latency) */
        a[i] = a[i-1] * 3 + b[i];
        
        /* Another integer recurrence */
        b[i] = b[i-1] + (long)c[i];
        
        /* Cross-iteration dependence with memory */
        acc += a[i] + b[i] + (long)c[i];
    }
    return acc;
}

__attribute__((noinline))
static double test5_nested_dep(double *arr1, double *arr2, int n) {
    double sum = 0.0;
    /* Multiple distance-1 dependences in sequence */
    for (int i = 2; i < n; i++) {
        double t1 = arr1[i-1] * 0.7;
        double t2 = arr2[i-2] * 1.3;
        arr1[i] = t1 + (double)i * 0.2;
        arr2[i] = t2 + arr1[i-1];
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (size < 10) size = SIZE;
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(size * sizeof(double));
    double *arr2 = (double*)malloc(size * sizeof(double));
    int *iarr1 = (int*)malloc(size * sizeof(int));
    int *iarr2 = (int*)malloc(size * sizeof(int));
    float *farr = (float*)malloc(size * sizeof(float));
    float **ptrs = (float**)malloc(size * sizeof(float*));
    
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        iarr1[i] = i % 37;
        iarr2[i] = i % 41;
        farr[i] = (float)i * 0.3f;
        ptrs[i] = &farr[i];
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_fp_recurrence(arr1, size, 1.05);
    total += (double)test2_mixed_latency(iarr1, arr2, size);
    total += (double)test3_pointer_chase(ptrs, farr, size);
    total += (double)test4_complex_chain((long*)iarr1, (long*)iarr2, arr1, size);
    total += test5_nested_dep(arr1, arr2, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr1);
    free(iarr2);
    free(farr);
    free(ptrs);
    
    return (int)total % 256;
}

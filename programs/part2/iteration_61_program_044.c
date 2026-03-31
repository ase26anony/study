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
    /* Loop-carried FP recurrence with distance=1 */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + arr[i] * 0.5;  /* Distance-1 dependence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP with loop-carried dependence */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];  /* FP recurrence */
        arr[i] = arr[i-1] + (int)farr[i];             /* Integer recurrence */
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 1.5f + data[i];  /* Distance-1 through pointers */
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long* arr, double* darr, int n) {
    long acc = 0;
    /* Multiple operations with different latencies in a chain */
    for (int i = 1; i < n; i++) {
        darr[i] = darr[i-1] * 2.0;           /* FP multiply (latency) */
        arr[i] = arr[i-1] + (long)darr[i];   /* Integer add with cast */
        arr[i] = arr[i] * 3;                 /* Integer multiply (higher latency) */
        acc += arr[i];
    }
    return acc;
}

__attribute__((noinline))
static double test5_memory_aliasing(double* a, double* b, int n) {
    double sum = 0.0;
    /* Potential aliasing creates memory dependence ambiguity */
    for (int i = 1; i < n; i++) {
        a[i] = b[i-1] * 1.1 + a[i];      /* Load from b[i-1], store to a[i] */
        b[i] = a[i-1] * 0.9 + b[i];      /* Load from a[i-1], store to b[i] */
        sum += a[i] + b[i];
    }
    return sum;
}

/* Main driver with volatile bounds to prevent constant propagation */
int main(int argc, char** argv) {
    volatile int vsize = SIZE;  /* volatile prevents constant folding */
    int n = vsize;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = SIZE;
    }
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* data = (float*)malloc(n * sizeof(float));
    long* arr4 = (long*)malloc(n * sizeof(long));
    double* arr5a = (double*)malloc(n * sizeof(double));
    double* arr5b = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i;
        arr3[i] = (double)(i % 50) * 0.2;
        data[i] = (float)(i % 30) * 0.3f;
        ptrs[i] = &data[(i + 1) % n];  /* Create pointer chain */
        arr4[i] = (long)(i % 40);
        arr5a[i] = (double)(i % 60) * 0.4;
        arr5b[i] = (double)(i % 70) * 0.5;
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    result += test1_recurrence_fp(arr1, n, 1.05);
    result += (double)test2_mixed_latency(arr2, arr3, n);
    result += (double)test3_pointer_chase(ptrs, data, n);
    result += (double)test4_complex_chain(arr4, arr3, n);
    result += test5_memory_aliasing(arr5a, arr5b, n);
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(arr4);
    free(arr5a);
    free(arr5b);
    
    return 0;
}

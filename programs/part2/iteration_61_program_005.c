/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
static double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with FP recurrence: distance-1 dependence */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP operations with memory aliasing */
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
    /* Pointer-based recurrence with memory accesses */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 0.9f + data[i];
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long* arr, double* darr, int n, long seed) {
    long acc = seed;
    /* Complex chain with multiple dependences */
    for (int i = 1; i < n; i++) {
        darr[i] = darr[i-1] * 2.5 - (double)acc;
        arr[i] = (long)(darr[i] * 100.0) + arr[i-1] * 3;
        acc = arr[i] / 7 + acc * 2;
    }
    return acc;
}

__attribute__((noinline))
static double test5_nested_dep(double* a, double* b, double* c, int n) {
    double sum = 0.0;
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] * 0.8 + b[i-2] * 0.2;
        b[i] = b[i-1] * 1.1 + c[i-1];
        c[i] = c[i-1] * 0.95 + a[i];
        sum += a[i] + b[i] + c[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    if (n < 10) n = 10;  /* Ensure minimum size */
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* data = (float*)malloc(n * sizeof(float));
    long* arr4 = (long*)malloc(n * sizeof(long));
    double* darr = (double*)malloc(n * sizeof(double));
    double* a = (double*)malloc(n * sizeof(double));
    double* b = (double*)malloc(n * sizeof(double));
    double* c = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i * 2;
        arr3[i] = (double)(i % 50) * 0.3;
        data[i] = (float)(i % 30) * 0.5f;
        ptrs[i] = &data[(i + 1) % n];
        arr4[i] = i * 3L;
        darr[i] = (double)(i % 40) * 0.4;
        a[i] = (double)(i % 20) * 0.25;
        b[i] = (double)(i % 25) * 0.35;
        c[i] = (double)(i % 15) * 0.45;
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling */
    result += test1_recurrence_fp(arr1, n, 1.05);
    result += (double)test2_mixed_latency(arr2, arr3, n);
    result += (double)test3_pointer_chase(ptrs, data, n);
    result += (double)test4_complex_chain(arr4, darr, n, 12345L);
    result += test5_nested_dep(a, b, c, n);
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(arr4);
    free(darr);
    free(a);
    free(b);
    free(c);
    
    return 0;
}

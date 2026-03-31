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
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence with distance-1 */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long* arr, double* darr, int n) {
    long total = 0;
    /* Complex chain with multiple dependences */
    for (int i = 2; i < n; i++) {
        darr[i] = darr[i-1] * darr[i-2] + (double)i;
        arr[i] = arr[i-1] + (long)(darr[i] * 100.0);
        arr[i] *= arr[i-2] + 1;
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static double test5_memory_aliasing(double* a, double* b, int n) {
    double sum = 0.0;
    /* Potential aliasing creates memory dependence edges */
    for (int i = 1; i < n; i++) {
        a[i] = b[i-1] * 3.14 + a[i-1];
        b[i] = a[i] * 0.5 + b[i-1];
        sum += a[i] + b[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
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
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i;
        arr3[i] = (double)(i % 50) * 0.2;
        data[i] = (float)(i % 30) * 0.3f;
        arr4[i] = i * 2L;
        darr[i] = (double)(i % 40) * 0.4;
        a[i] = (double)(i % 60) * 0.5;
        b[i] = (double)(i % 70) * 0.6;
    }
    
    /* Set up pointer array for chasing */
    for (int i = 0; i < n; i++) {
        ptrs[i] = &data[(i * 7) % n];  /* Create aliasing pattern */
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling */
    result += test1_recurrence_fp(arr1, n, 1.05);
    result += (double)test2_mixed_latency(arr2, arr3, n);
    result += (double)test3_pointer_chase(ptrs, data, n);
    result += (double)test4_complex_chain(arr4, darr, n);
    result += test5_memory_aliasing(a, b, n);
    
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
    
    return 0;
}

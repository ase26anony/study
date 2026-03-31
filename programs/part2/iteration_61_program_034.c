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
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int* arr, double* farr, int n) {
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
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float sum = 0.0f;
    /* Pointer-based recurrence with memory loads */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 0.9f + data[i];
        sum += *ptrs[i+1];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_dependence(long* arr, double* darr, int n) {
    long acc = 0;
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        darr[i] = darr[i-1] * darr[i-2] * 0.95;
        arr[i] = arr[i-1] + (long)(darr[i] * 100.0);
        acc += arr[i] * i;
    }
    return acc;
}

__attribute__((noinline))
static int test5_multi_op_recurrence(int* a, int* b, int* c, int n) {
    int sum = 0;
    /* Multiple operations with loop-carried dependences */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] * 3 + b[i];      // Distance-1 dependence on a
        b[i] = b[i-1] + a[i] * 2;      // Distance-1 dependence on b
        c[i] = c[i-1] + a[i] + b[i];   // Distance-1 dependence on c
        sum += a[i] + b[i] + c[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* data = (float*)malloc(n * sizeof(float));
    long* arr4 = (long*)malloc(n * sizeof(long));
    double* darr = (double*)malloc(n * sizeof(double));
    int* a = (int*)malloc(n * sizeof(int));
    int* b = (int*)malloc(n * sizeof(int));
    int* c = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 75) * 0.3;
        data[i] = (float)(i % 60) * 0.2f;
        arr4[i] = i % 40;
        darr[i] = (double)(i % 30) * 0.4;
        a[i] = i % 20;
        b[i] = i % 25;
        c[i] = i % 15;
    }
    
    /* Set up pointer array for chasing */
    for (int i = 0; i < n; i++) {
        ptrs[i] = &data[(i * 7) % n];
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    long result4 = 0L;
    int result5 = 0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    result1 = test1_recurrence_fp(arr1, n, 1.05);
    result2 = test2_mixed_latency(arr2, arr3, n);
    result3 = test3_pointer_chase(ptrs, data, n);
    result4 = test4_complex_dependence(arr4, darr, n);
    result5 = test5_multi_op_recurrence(a, b, c, n);
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4 + result5;
    printf("Final aggregated result: %f\n", final_result);
    
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

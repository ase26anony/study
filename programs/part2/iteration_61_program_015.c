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
        /* FP multiplication with loop-carried dependence */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional FP operation with intra-iteration dependence */
        arr[i] = arr[i] * 0.99 + 1.0;
        /* Mix with integer operation */
        sum += arr[i] + (double)(i % 7);
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int* arr, double* darr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Integer multiplication (higher latency than add) */
        int temp = arr[i-1] * 3;
        /* Store with potential aliasing */
        arr[i] = temp + i;
        /* FP operation using previous iteration's FP value */
        darr[i] = darr[i-1] * 1.01 + darr[i];
        /* Complex expression to create scheduling constraints */
        total += arr[i] + (int)(darr[i] * 100.0);
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float* arr, int n, float* coeffs) {
    float* ptr = arr;
    float result = 0.0f;
    /* Pointer-based recurrence with memory loads */
    for (int i = 0; i < n-1; i++) {
        /* Load from address computed in previous iteration */
        float val = *ptr;
        /* FP multiply-add chain */
        val = val * coeffs[i % 8] + 2.5f;
        /* Store to next location */
        *(ptr + 1) = val;
        ptr++;
        /* Additional computation to increase loop body size */
        result += val * (float)(i & 3);
    }
    return result;
}

__attribute__((noinline))
static long test4_complex_chain(long* arr, double* brr, int n) {
    long acc = arr[0];
    double dacc = brr[0];
    /* Multiple interleaved recurrence chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: integer recurrence */
        long temp1 = acc * 7L + arr[i];
        /* Chain 2: floating-point recurrence */
        double temp2 = dacc * 1.1 + brr[i];
        /* Cross-chain dependence */
        temp1 += (long)(temp2 * 10.0);
        /* Store results for next iteration */
        acc = temp1;
        dacc = temp2;
        /* Memory store with potential aliasing */
        arr[i] = acc;
        brr[i] = dacc;
    }
    return acc;
}

/* Use volatile to prevent constant propagation */
volatile int g_size = SIZE;

int main(int argc, char** argv) {
    /* Use command line or volatile to prevent loop unrolling */
    int n = (argc > 1) ? atoi(argv[1]) : g_size;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;  /* Ensure loops run enough iterations */
    
    /* Initialize arrays with non-zero values */
    double* arr1 = (double*)malloc(SIZE * sizeof(double));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    double* arr3 = (double*)malloc(SIZE * sizeof(double));
    float* arr4 = (float*)malloc(SIZE * sizeof(float));
    float* coeffs = (float*)malloc(8 * sizeof(float));
    long* arr5 = (long*)malloc(SIZE * sizeof(long));
    double* arr6 = (double*)malloc(SIZE * sizeof(double));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)(i % 100) * 0.5;
        arr2[i] = i * 2;
        arr3[i] = (double)(i % 50) * 1.5;
        arr4[i] = (float)(i % 30) * 0.7f;
        if (i < 8) coeffs[i] = 0.9f + i * 0.05f;
        arr5[i] = i * 3L;
        arr6[i] = (double)(i % 60) * 0.3;
    }
    
    /* Call test functions to trigger modulo scheduling analysis */
    double result1 = test1_recurrence_fp(arr1, n, 1.05);
    int result2 = test2_mixed_latency(arr2, arr3, n);
    float result3 = test3_pointer_chase(arr4, n, coeffs);
    long result4 = test4_complex_chain(arr5, arr6, n);
    
    /* Use results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4;
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(coeffs);
    free(arr5);
    free(arr6);
    
    return 0;
}

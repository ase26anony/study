/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimization and inlining */
volatile int g_volatile_size = SIZE;
static int g_checksum = 0;

/* Test 1: Floating-point recurrence with distance-1 dependence */
__attribute__((noinline))
double test_fp_recurrence(double* arr, int n) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Mix operations with different latencies */
        double temp = arr[i-1] * 1.01;      /* FP multiply */
        arr[i] = temp + arr[i] * 0.5;       /* FP multiply + add */
        sum += arr[i];
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
__attribute__((noinline))
int test_int_recurrence(int* arr, int* brr, int n) {
    int sum = 0;
    /* Multiple loop-carried dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence chain */
        int val1 = arr[i-1] * 3;            /* Integer multiply */
        int val2 = val1 + brr[i];           /* Integer add */
        arr[i] = val2 * 2;                  /* Another multiply */
        
        /* Additional intra-iteration dependence */
        brr[i] = arr[i] + i;                /* Use current iteration value */
        sum += arr[i] + brr[i];
    }
    return sum;
}

/* Test 3: Mixed FP and integer with pointer chasing */
__attribute__((noinline))
double test_mixed_recurrence(double* darr, int* iarr, int n) {
    double dsum = 0.0;
    int isum = 0;
    
    /* Complex recurrence pattern */
    for (int i = 1; i < n; i++) {
        /* FP recurrence */
        double fp_val = darr[i-1] * 2.5;
        darr[i] = fp_val + 1.0 / (iarr[i] + 1.0);  /* FP division */
        
        /* Integer recurrence with memory access */
        int int_val = iarr[i-1] * 2;
        iarr[i] = int_val + (int)darr[i];
        
        dsum += darr[i];
        isum += iarr[i];
    }
    return dsum + isum;
}

/* Test 4: Nested recurrences for more complex DDG */
__attribute__((noinline))
float test_nested_recurrence(float* farr1, float* farr2, int n) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Two independent recurrence chains */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance-1 */
        float val1 = farr1[i-1] * 1.1f;
        farr1[i] = val1 + farr1[i-2] * 0.9f;  /* distance-2 */
        
        /* Chain 2: distance-1 with different operations */
        float val2 = farr2[i-1] / 1.3f;        /* FP division */
        farr2[i] = val2 * val2 + 2.0f;         /* FP multiply + add */
        
        sum1 += farr1[i];
        sum2 += farr2[i];
    }
    return sum1 + sum2;
}

/* Test 5: Artificial heavy-latency chain */
__attribute__((noinline))
double test_heavy_latency(double* arr, int n) {
    double acc = arr[0];
    
    /* Single heavy recurrence chain */
    for (int i = 1; i < n; i++) {
        /* Multiple high-latency operations in chain */
        double t1 = acc * 1.23456789;      /* FP multiply */
        double t2 = t1 / 0.987654321;      /* FP division */
        double t3 = t2 * t2;               /* FP square */
        acc = t3 + arr[i];                 /* FP add with memory */
        arr[i] = acc;
    }
    return acc;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int use_size = g_volatile_size;
    int n = (argc > 1) ? atoi(argv[1]) : use_size;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;  /* Ensure enough iterations */
    
    /* Allocate and initialize arrays */
    double* darr1 = (double*)malloc(SIZE * sizeof(double));
    double* darr2 = (double*)malloc(SIZE * sizeof(double));
    int* iarr1 = (int*)malloc(SIZE * sizeof(int));
    int* iarr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr1 = (float*)malloc(SIZE * sizeof(float));
    float* farr2 = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        darr1[i] = 1.0 + i * 0.01;
        darr2[i] = 2.0 - i * 0.005;
        iarr1[i] = i * 3;
        iarr2[i] = i * 7;
        farr1[i] = i * 0.1f;
        farr2[i] = i * 0.2f;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test_fp_recurrence(darr1, n);
    total += test_int_recurrence(iarr1, iarr2, n);
    total += test_mixed_recurrence(darr2, iarr1, n);
    total += test_nested_recurrence(farr1, farr2, n);
    total += test_heavy_latency(darr1, n);
    
    /* Use results to prevent dead code elimination */
    g_checksum = (int)total;
    printf("Result: %f (checksum: %d)\n", total, g_checksum);
    
    /* Cleanup */
    free(darr1);
    free(darr2);
    free(iarr1);
    free(iarr2);
    free(farr1);
    free(farr2);
    
    return g_checksum != 0 ? 0 : 1;
}

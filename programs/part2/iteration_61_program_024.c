/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with loop-carried dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdlib.h>
#include <stdio.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline, optimize("no-unroll-loops")))
double test1_fp_recurrence(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.01;  /* Self-dependence within iteration */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
float test2_mixed_latency(float* farr, int* iarr, int n) {
    float acc = farr[0];
    int int_acc = iarr[0];
    
    /* Mixed integer/float recurrence with memory accesses */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependences on both float and int arrays */
        farr[i] = farr[i-1] * 1.01f + (float)int_acc * 0.5f;
        iarr[i] = iarr[i-1] * 3 + i;  /* Integer multiplication latency */
        
        /* Additional operations with different latencies */
        float temp = farr[i] * farr[i-1];  /* FP multiplication */
        int_acc = iarr[i] + (int)temp;     /* Mixed type operation */
        
        /* Memory store with potential aliasing */
        farr[i] = temp + (float)iarr[i];
    }
    return farr[n-1];
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test3_pointer_chase(double** ptrs, double* data, int n) {
    double sum = 0.0;
    
    /* Initialize pointer chain */
    for (int i = 0; i < n; i++) {
        ptrs[i] = &data[i];
    }
    
    /* Pointer chasing with distance-1 dependence */
    for (int i = 1; i < n; i++) {
        /* Load through pointer from previous iteration */
        double val = *ptrs[i-1];
        
        /* Compute new value with FP operations */
        val = val * 1.5 + (double)i * 0.25;
        
        /* Store and update pointer */
        data[i] = val;
        ptrs[i] = &data[i];
        
        sum += val;
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test4_complex_dependence(int* arr1, int* arr2, int n, int factor) {
    int sum = 0;
    
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        /* Two separate distance-1 chains */
        arr1[i] = arr1[i-1] * factor + arr1[i-2];  /* Distance 1 and 2 */
        arr2[i] = arr2[i-1] + arr1[i] * 3;         /* Mixed dependence */
        
        /* Additional operations to increase DDG complexity */
        int temp = arr1[i] * arr2[i];  /* Integer multiplication */
        arr2[i] = temp >> 2;           /* Shift operation */
        
        sum += arr2[i];
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test5_nested_dependence(double* arr, int n) {
    /* Complex recurrence with multiple uses */
    for (int i = 3; i < n; i++) {
        /* Multiple distance-1 dependences in single expression */
        arr[i] = (arr[i-1] * 0.3 + arr[i-2] * 0.4) * arr[i-3];
        
        /* Additional FP operations with latency */
        arr[i] = arr[i] * arr[i] + 1.0;
        arr[i] = 1.0 / (arr[i] + 1.0);  /* FP division has high latency */
    }
    
    /* Compute checksum */
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    float* farr = (float*)malloc(n * sizeof(float));
    int* iarr = (int*)malloc(n * sizeof(int));
    double** ptrs = (double**)malloc(n * sizeof(double*));
    double* data = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        farr[i] = (float)(i % 50) * 0.2f;
        iarr[i] = i % 25;
        data[i] = (double)(i % 30) * 0.3;
        arr2[i] = i % 20;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling */
    total += test1_fp_recurrence(arr1, n, 1.05);
    total += test2_mixed_latency(farr, iarr, n);
    total += test3_pointer_chase(ptrs, data, n);
    total += test4_complex_dependence(iarr, arr2, n, 7);
    total += test5_nested_dependence(arr1, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(farr);
    free(iarr);
    free(ptrs);
    free(data);
    free(arr2);
    
    return 0;
}

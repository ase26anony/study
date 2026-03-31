/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline, optimize("no-unroll-loops")))
double test_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;      /* FP multiply */
        sum += arr[i];               /* FP add with accumulation */
    }
    return sum;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer dependence */
        arr[i] = arr[i-1] * 3 + i;   /* Integer multiply */
        
        /* Distance-1 FP dependence with different latency */
        farr[i] = farr[i-1] * 1.01 + farr[i];
        
        /* Cross-type operation creating additional dependences */
        arr[i] += (int)(farr[i] * 2.0);
        
        /* Memory store with potential aliasing */
        total += arr[i];
    }
    return total;
}

__attribute__((noinline, optimize("no-unroll-loops")))
double test_pointer_chase(double** ptrs, double* vals, int n) {
    double result = 0.0;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Load through pointer (memory latency) */
        double current = *ptrs[i];
        
        /* Distance-1 pointer update */
        ptrs[i+1] = (double*)((size_t)ptrs[i] + sizeof(double));
        
        /* FP operation chain */
        vals[i+1] = current * 1.5 + vals[i];
        
        /* Additional dependent operations */
        result += vals[i+1];
    }
    return result;
}

__attribute__((noinline, optimize("no-unroll-loops")))
int test_complex_recurrence(int* a, int* b, double* c, int n) {
    int sum = 0;
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        /* Two independent distance-1 chains */
        a[i] = a[i-1] + b[i-2] * 2;      /* Mixed distances */
        b[i] = b[i-1] * a[i-1] + i;      /* Integer multiply */
        
        /* FP chain with memory access */
        c[i] = c[i-1] * c[i-2] + (double)a[i];
        
        /* Complex expression with multiple uses */
        sum += a[i] + b[i] + (int)c[i];
    }
    return sum;
}

/* Use volatile to prevent constant propagation */
volatile int g_size = SIZE;

int main(int argc, char** argv) {
    /* Use command line or volatile to prevent loop unrolling */
    int n = (argc > 1) ? atoi(argv[1]) : g_size;
    if (n < 10) n = SIZE;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    double** ptrs = (double**)malloc(n * sizeof(double*));
    int* arr4 = (int*)malloc(n * sizeof(int));
    int* arr5 = (int*)malloc(n * sizeof(int));
    double* arr6 = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 30) * 0.3;
        ptrs[i] = &arr3[i];
        arr4[i] = i % 20;
        arr5[i] = i % 25;
        arr6[i] = (double)(i % 40) * 0.4;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test_recurrence_fp(arr1, n, 1.05);
    total += (double)test_mixed_latency(arr2, arr3, n);
    total += test_pointer_chase(ptrs, arr1, n);
    total += (double)test_complex_recurrence(arr4, arr5, arr6, n);
    
    /* Prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(arr4);
    free(arr5);
    free(arr6);
    
    return 0;
}

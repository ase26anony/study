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
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int total = 0;
    /* Multiple loop-carried dependences with mixed operations */
    for (int i = 1; i < n; i++) {
        /* Integer recurrence with multiplication (higher latency) */
        arr[i] = arr[i-1] * 3 + i;
        
        /* Floating-point recurrence */
        farr[i] = farr[i-1] * 1.01 + farr[i] * 0.99;
        
        /* Memory access with potential aliasing */
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence - distance-1 dependence */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];
        result += *ptrs[i];
    }
    return result;
}

__attribute__((noinline))
static long test4_complex_chain(long *a, long *b, double *c, int n) {
    long acc = 0;
    /* Complex chain with multiple dependences */
    for (int i = 1; i < n; i++) {
        /* Chain 1: integer multiply recurrence */
        a[i] = a[i-1] * 7 + b[i];
        
        /* Chain 2: floating-point recurrence */
        c[i] = c[i-1] * 1.5 - c[i];
        
        /* Chain 3: mixed operation with memory */
        b[i] = (a[i] >> 3) + (long)(c[i] * 100.0);
        
        /* Final accumulation with non-trivial operation */
        acc += a[i] * b[i] + (long)c[i];
    }
    return acc;
}

__attribute__((noinline))
static double test5_nested_dependences(double *arr1, double *arr2, int n) {
    double sum = 0.0;
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        /* Two separate recurrence chains */
        arr1[i] = arr1[i-1] * 1.1 + arr1[i-2] * 0.9;
        arr2[i] = arr2[i-1] * 0.8 - arr2[i-2] * 0.2;
        
        /* Cross-dependence between chains */
        sum += arr1[i] * arr2[i-1] + arr2[i] * arr1[i-1];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    if (n < 10) n = 10;  /* Ensure minimum size for loops */
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    double *arr2 = (double*)malloc(n * sizeof(double));
    int *int_arr = (int*)malloc(n * sizeof(int));
    long *long_arr1 = (long*)malloc(n * sizeof(long));
    long *long_arr2 = (long*)malloc(n * sizeof(long));
    float **ptrs = (float**)malloc(n * sizeof(float*));
    float *float_data = (float*)malloc(n * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        int_arr[i] = i * 3;
        long_arr1[i] = i * 5L;
        long_arr2[i] = i * 7L;
        float_data[i] = (float)i * 0.3f;
        ptrs[i] = &float_data[i];
    }
    
    double total = 0.0;
    
    /* Call test functions - each with different recurrence patterns */
    total += test1_fp_recurrence(arr1, n, 1.05);
    total += (double)test2_mixed_latency(int_arr, arr2, n);
    total += (double)test3_pointer_chase(ptrs, float_data, n);
    total += (double)test4_complex_chain(long_arr1, long_arr2, arr1, n);
    total += test5_nested_dependences(arr1, arr2, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(int_arr);
    free(long_arr1);
    free(long_arr2);
    free(ptrs);
    free(float_data);
    
    return (int)(total / 1000.0);
}

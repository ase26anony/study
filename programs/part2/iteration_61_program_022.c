/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + (double)i * 1.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int sum = 0;
    /* Mixed integer/float with memory aliasing */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        arr[i] = (int)farr[i] * 3 + arr[i-1];
        sum += arr[i];
    }
    return sum;
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
    long total = 0;
    /* Multiple interleaved dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence chain */
        a[i] = a[i-1] * 7 + b[i];
        
        /* Intra-iteration dependence */
        b[i] = a[i] / 3 + i;
        
        /* Floating-point with distance-1 */
        c[i] = c[i-1] * 1.001 + (double)a[i];
        
        /* Another distance-1 chain */
        if (i > 1) {
            a[i] += b[i-1];
        }
        
        total += a[i] + b[i] + (long)c[i];
    }
    return total;
}

__attribute__((noinline))
static double test5_nested_dependences(double *arr1, double *arr2, int n) {
    double sum = 0.0;
    /* Multiple recurrence patterns */
    for (int i = 2; i < n; i++) {
        /* Two separate distance-1 chains */
        arr1[i] = arr1[i-1] * 1.1 + arr1[i-2] * 0.9;
        arr2[i] = arr2[i-1] * 0.95 + arr1[i] * 1.05;
        
        /* Cross-iteration mixed operation */
        sum += arr1[i] * arr2[i-1] - arr2[i] * arr1[i-1];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    if (n < 10) n = 10;  /* Ensure minimum size */
    
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
        arr1[i] = (double)(i + 1) * 0.5;
        arr2[i] = (double)(i + 2) * 0.3;
        int_arr[i] = i * 3;
        long_arr1[i] = i * 5L;
        long_arr2[i] = i * 7L;
        float_data[i] = (float)i * 1.1f;
        ptrs[i] = &float_data[i];
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    result += test1_fp_recurrence(arr1, n, 1.05);
    result += (double)test2_mixed_latency(int_arr, arr2, n);
    result += (double)test3_pointer_chase(ptrs, float_data, n);
    result += (double)test4_complex_chain(long_arr1, long_arr2, arr1, n);
    result += test5_nested_dependences(arr1, arr2, n);
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(int_arr);
    free(long_arr1);
    free(long_arr2);
    free(ptrs);
    free(float_data);
    
    return 0;
}

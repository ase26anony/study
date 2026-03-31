/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging for distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structures intact */
__attribute__((noinline))
static double test1_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    /* Loop-carried FP recurrence with distance=1 */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * factor + arr[i] * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int *arr, double *farr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.01 + (double)arr[i];
        arr[i] = (int)farr[i] * arr[i-1] + i;
        total += arr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float **ptrs, float *base, int n) {
    /* Pointer-based recurrence simulating linked list traversal */
    float sum = 0.0f;
    ptrs[0] = &base[0];
    
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through pointer */
        ptrs[i] = &base[i] + (int)(*ptrs[i-1]) % 16;
        sum += *ptrs[i];
    }
    return sum;
}

__attribute__((noinline))
static long test4_complex_chain(long *a, long *b, long *c, int n) {
    /* Multiple interleaved recurrences with different distances */
    long acc1 = a[0], acc2 = b[0];
    
    for (int i = 1; i < n; i++) {
        /* Three separate recurrence chains */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 * 5 + b[i];
        c[i] = acc1 + acc2 + c[i-1] * 7;  /* Distance-1 from c[i-1] */
        
        /* Memory operations that may alias */
        a[i] = acc1 ^ b[i];
        b[i] = acc2 ^ a[i-1];  /* Another distance-1 dependence */
    }
    return acc1 + acc2 + c[n-1];
}

__attribute__((noinline))
static double test5_nested_dep(double *x, double *y, int n, double alpha) {
    /* Complex FP recurrence with multiple uses */
    double beta = 1.0 - alpha;
    
    for (int i = 2; i < n; i++) {
        /* Multiple distance-1 dependences */
        x[i] = alpha * x[i-1] + beta * x[i-2] + y[i];
        y[i] = x[i-1] * y[i-1] * 0.3 + x[i] * 0.7;
    }
    
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += x[i] + y[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int data_size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = data_size;
    
    if (n < 4) n = 4;  /* Ensure minimum size for recurrences */
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    int *arr2 = (int*)malloc(n * sizeof(int));
    double *arr3 = (double*)malloc(n * sizeof(double));
    float **ptrs = (float**)malloc(n * sizeof(float*));
    float *base = (float*)malloc(n * sizeof(float));
    long *a = (long*)malloc(n * sizeof(long));
    long *b = (long*)malloc(n * sizeof(long));
    long *c = (long*)malloc(n * sizeof(long));
    double *x = (double*)malloc(n * sizeof(double));
    double *y = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i % 7) * 0.1;
        arr2[i] = i * 3;
        arr3[i] = (i % 5) * 0.2;
        base[i] = (i % 3) * 1.5f;
        a[i] = i * 2L;
        b[i] = i * 3L;
        c[i] = i * 5L;
        x[i] = (i % 11) * 0.05;
        y[i] = (i % 13) * 0.07;
    }
    
    /* Call test functions to trigger modulo scheduling analysis */
    double result1 = test1_fp_recurrence(arr1, n, 1.05);
    int result2 = test2_mixed_latency(arr2, arr3, n);
    float result3 = test3_pointer_chase(ptrs, base, n);
    long result4 = test4_complex_chain(a, b, c, n);
    double result5 = test5_nested_dep(x, y, n, 0.3);
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4 + result5;
    
    printf("Result: %f\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(base);
    free(a);
    free(b);
    free(c);
    free(x);
    free(y);
    
    return 0;
}

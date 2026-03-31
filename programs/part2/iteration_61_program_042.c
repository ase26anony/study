/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimization and ensure loops aren't unrolled */
static volatile int g_volatile_size = SIZE;

/* Test functions marked noinline to prevent merging */
__attribute__((noinline, noipa))
double test1_recurrence_fp(double* arr, int n) {
    double sum = 0.0;
    /* Loop-carried FP recurrence with distance=1 */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] * 1.01 + arr[i] * 0.99;  /* Distance-1 dependence */
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline, noipa))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/FP with memory aliasing */
    for (int i = 1; i < n; i++) {
        farr[i] = farr[i-1] * 1.5 + i * 0.1;      /* FP recurrence */
        arr[i] = arr[i-1] + (int)farr[i];          /* Integer recurrence */
        total += arr[i] * i;                       /* Integer multiply */
    }
    return total;
}

__attribute__((noinline, noipa))
float test3_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];   /* Distance-1 through pointers */
        result += *ptrs[i];
    }
    return result;
}

__attribute__((noinline, noipa))
long test4_complex_chain(long* a, double* b, int* c, int n) {
    long acc = 0;
    /* Multiple interleaved recurrences with different latencies */
    for (int i = 1; i < n; i++) {
        b[i] = b[i-1] * b[i] + 0.5;               /* FP recurrence */
        a[i] = a[i-1] + (long)(b[i] * 100.0);     /* Integer recurrence */
        c[i] = c[i-1] * 2 + i;                    /* Integer recurrence */
        acc += a[i] * c[i];                       /* Long multiplication */
    }
    return acc;
}

__attribute__((noinline, noipa))
double test5_nested_dep(double* arr1, double* arr2, int n) {
    double sum = 0.0;
    /* Two independent recurrence chains */
    for (int i = 2; i < n; i++) {
        arr1[i] = arr1[i-1] * 1.1 + arr1[i-2] * 0.9;  /* Distance-1 and distance-2 */
        arr2[i] = arr2[i-1] * 0.8 + arr1[i];          /* Cross-chain dependence */
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    int size = g_volatile_size;
    if (argc > 1) size = atoi(argv[1]);
    if (size < 10) size = 1000;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(size * sizeof(double));
    double* arr2 = (double*)malloc(size * sizeof(double));
    int* iarr1 = (int*)malloc(size * sizeof(int));
    int* iarr2 = (int*)malloc(size * sizeof(int));
    long* larr = (long*)malloc(size * sizeof(long));
    float** ptrs = (float**)malloc(size * sizeof(float*));
    float* fdata = (float*)malloc(size * sizeof(float));
    
    /* Initialize with non-zero values to avoid constant propagation */
    for (int i = 0; i < size; i++) {
        arr1[i] = (i % 100) * 0.01;
        arr2[i] = (i % 50) * 0.02;
        iarr1[i] = i % 37;
        iarr2[i] = i % 41;
        larr[i] = i % 29;
        fdata[i] = (i % 23) * 0.03f;
        ptrs[i] = &fdata[i];
    }
    
    /* Call test functions to trigger modulo scheduling analysis */
    double result1 = test1_recurrence_fp(arr1, size);
    int result2 = test2_mixed_latency(iarr1, arr2, size);
    float result3 = test3_pointer_chase(ptrs, fdata, size);
    long result4 = test4_complex_chain(larr, arr1, iarr2, size);
    double result5 = test5_nested_dep(arr1, arr2, size);
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4 + result5;
    
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr1);
    free(iarr2);
    free(larr);
    free(ptrs);
    free(fdata);
    
    return (int)(final_result / 1000000) % 255;
}

/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with loop-carried dependences
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
        arr[i] = arr[i-1] * factor + (double)i * 0.5;
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
static int test2_mixed_latency(int* arr, double* darr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Integer recurrence */
        arr[i] = arr[i-1] * 3 + i;
        
        /* Floating-point recurrence with different latency */
        darr[i] = darr[i-1] * 1.01 + (double)arr[i];
        
        /* Additional operations to create scheduling opportunities */
        total += arr[i] + (int)darr[i];
    }
    return total;
}

__attribute__((noinline))
static float test3_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        *ptrs[i+1] = *ptrs[i] * 2.0f + data[i];
        result += *ptrs[i];
    }
    return result;
}

__attribute__((noinline))
static double test4_complex_dependence(double* a, double* b, double* c, int n) {
    double acc = 0.0;
    /* Multiple interleaved recurrences */
    for (int i = 2; i < n; i++) {
        /* Two separate recurrence chains */
        a[i] = a[i-1] * 1.1 + b[i-2] * 0.5;
        b[i] = b[i-1] * 0.9 + a[i-1] * 0.3;
        
        /* Cross-iteration dependence with memory */
        c[i] = c[i-1] + a[i] * b[i];
        
        /* Complex expression with multiple operations */
        acc += a[i] * 2.0 + b[i] * 3.0 + c[i];
    }
    return acc;
}

__attribute__((noinline))
static int test5_integer_recurrence_multi_use(int* arr1, int* arr2, int n) {
    int sum = 0;
    /* Integer recurrence with multiple uses in next iteration */
    for (int i = 1; i < n; i++) {
        int prev = arr1[i-1] + arr2[i-1];
        arr1[i] = prev * 2 + i;
        arr2[i] = arr1[i] + prev;
        
        /* Additional computation to increase ILP potential */
        sum += arr1[i] * arr2[i] - prev;
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
    double* a = (double*)malloc(n * sizeof(double));
    double* b = (double*)malloc(n * sizeof(double));
    double* c = (double*)malloc(n * sizeof(double));
    int* arr4 = (int*)malloc(n * sizeof(int));
    int* arr5 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = (double)(i % 75) * 0.2;
        data[i] = (float)(i % 25) * 0.3f;
        a[i] = (double)(i % 60) * 0.15;
        b[i] = (double)(i % 40) * 0.25;
        c[i] = (double)(i % 80) * 0.05;
        arr4[i] = i % 30;
        arr5[i] = i % 20;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < n; i++) {
        ptrs[i] = &data[(i * 7) % n];  /* Create aliasing pattern */
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test1_recurrence_fp(arr1, n, 1.05);
    total += (double)test2_mixed_latency(arr2, arr3, n);
    total += (double)test3_pointer_chase(ptrs, data, n);
    total += test4_complex_dependence(a, b, c, n);
    total += (double)test5_integer_recurrence_multi_use(arr4, arr5, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(a);
    free(b);
    free(c);
    free(arr4);
    free(arr5);
    
    return 0;
}

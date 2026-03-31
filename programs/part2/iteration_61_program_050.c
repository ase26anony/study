/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test1_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] * factor + (double)i;
        /* Additional operations to create scheduling opportunities */
        arr[i] += arr[i] * 0.5;      /* Intra-iteration dependence */
        sum += arr[i];               /* Reduction for side effect */
    }
    return sum;
}

__attribute__((noinline))
int test2_mixed_latency(int* arr, double* farr, int n) {
    int total = 0;
    /* Mixed integer/float recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Distance-1 integer recurrence */
        int temp = arr[i-1] * 3 + i;
        
        /* Floating-point operation with higher latency */
        farr[i] = farr[i-1] * 1.01 + (double)temp;
        
        /* Memory store with potential aliasing */
        arr[i] = temp + (int)farr[i];
        
        /* Additional operations with different latencies */
        farr[i] = farr[i] * farr[i-1];  /* Another FP recurrence */
        total += arr[i] + (int)farr[i];
    }
    return total;
}

__attribute__((noinline))
float test3_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Load from pointer stored in previous iteration */
        float val = *ptrs[i];
        
        /* Compute new value with FP operations */
        val = val * 1.5f + (float)i;
        
        /* Store to next pointer location (distance=1 dependence) */
        data[i+1] = val;
        ptrs[i+1] = &data[i+1];
        
        /* Additional computation to increase loop body size */
        result += val * 0.7f;
        result = result / (result + 1.0f);  /* Creates data dependence */
    }
    return result;
}

__attribute__((noinline))
long test4_complex_recurrence(long* a, double* b, int* c, int n) {
    long acc = 0;
    /* Complex loop with multiple recurrence chains */
    for (int i = 2; i < n; i++) {
        /* Chain 1: Integer recurrence with multiplication */
        a[i] = a[i-1] * 7 - a[i-2] + i;
        
        /* Chain 2: Floating-point recurrence */
        b[i] = b[i-1] * 1.02 + b[i-2] * 0.98;
        
        /* Chain 3: Mixed type recurrence */
        c[i] = (int)(b[i] * 100.0) + c[i-1];
        
        /* Cross-chain dependencies */
        a[i] += (long)(b[i] * 10.0);
        b[i] += (double)c[i] * 0.01;
        
        /* Reduction to prevent elimination */
        acc += a[i] + (long)b[i] + c[i];
    }
    return acc;
}

/* Helper to initialize data with volatile to prevent constant propagation */
volatile int init_seed = 42;

int main(int argc, char** argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (n < 10) n = 10;
    if (n > 10000) n = 10000;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float* data = (float*)malloc(n * sizeof(float));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    long* arr4 = (long*)malloc(n * sizeof(long));
    double* arr5 = (double*)malloc(n * sizeof(double));
    int* arr6 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with volatile seed to prevent compile-time optimization */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i + init_seed);
        arr2[i] = i * 2;
        arr3[i] = (double)(i * 3);
        data[i] = (float)(i * 1.5f);
        ptrs[i] = &data[i];
        arr4[i] = i * 5L;
        arr5[i] = (double)(i * 7);
        arr6[i] = i * 11;
    }
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    long result4 = 0L;
    
    /* Call test functions multiple times to increase scheduling analysis */
    for (int iter = 0; iter < 3; iter++) {
        result1 += test1_recurrence_fp(arr1, n, 1.05 + iter * 0.01);
        result2 += test2_mixed_latency(arr2, arr3, n);
        result3 += test3_pointer_chase(ptrs, data, n);
        result4 += test4_complex_recurrence(arr4, arr5, arr6, n);
        
        /* Modify data slightly between iterations */
        arr1[0] += 0.5;
        arr2[0] += 1;
        data[0] += 0.1f;
        arr4[0] += 2;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: %f, %d, %f, %ld\n", 
           result1, result2, result3, result4);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(data);
    free(ptrs);
    free(arr4);
    free(arr5);
    free(arr6);
    
    return 0;
}

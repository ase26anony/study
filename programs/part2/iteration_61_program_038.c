/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent inlining to keep loop structure intact */
__attribute__((noinline))
double test_recurrence_fp(double* arr, int n, double factor) {
    double sum = 0.0;
    /* Loop with floating-point recurrence (distance=1) */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * factor;      /* FP multiply */
        arr[i] = temp + arr[i] * 0.5;         /* FP multiply + add */
        sum += arr[i];                        /* Accumulator */
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_recurrence(int* arr, double* darr, int n) {
    int total = 0;
    /* Mixed integer/FP recurrence with memory aliasing */
    for (int i = 1; i < n; i++) {
        /* Integer recurrence chain */
        int base = arr[i-1] * 3;              /* Integer multiply */
        arr[i] = base + i;                    /* Integer add */
        
        /* FP recurrence using integer result */
        double fp_val = darr[i-1] * 2.0;      /* FP multiply */
        darr[i] = fp_val + (double)arr[i];    /* FP add with conversion */
        
        total += arr[i];                      /* Prevent elimination */
    }
    return total;
}

__attribute__((noinline))
float test_pointer_chase(float** ptrs, float* data, int n) {
    float result = 0.0f;
    /* Pointer-based recurrence simulating linked list traversal */
    for (int i = 0; i < n-1; i++) {
        /* Load from pointer stored in previous iteration */
        float val = *ptrs[i];                  /* Memory load */
        val = val * 1.5f + data[i];            /* FP operations */
        *ptrs[i+1] = val;                      /* Memory store */
        result += val;
        
        /* Additional operations to increase DDG complexity */
        data[i+1] = data[i] * 0.8f + val;      /* Another recurrence */
    }
    return result;
}

__attribute__((noinline))
long test_multi_dependence(long* arr, double* brr, int n) {
    long sum = 0;
    /* Multiple interleaved dependence chains */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance=1 integer recurrence */
        long a = arr[i-1] + arr[i-2];          /* Uses two previous values */
        arr[i] = a * 2 - i;
        
        /* Chain 2: distance=1 FP recurrence */
        double b = brr[i-1] * brr[i-2];        /* FP multiply */
        brr[i] = b + (double)arr[i];
        
        /* Chain 3: distance=2 recurrence */
        if (i >= 4) {
            arr[i] += arr[i-4];                /* Longer distance */
        }
        
        sum += arr[i] + (long)brr[i];
    }
    return sum;
}

/* Complex loop with if-conversion opportunities */
__attribute__((noinline))
double test_conditional_recurrence(double* arr, int* mask, int n) {
    double total = 0.0;
    for (int i = 1; i < n; i++) {
        /* Conditional recurrence */
        double prev = (mask[i] > 0) ? arr[i-1] : 1.0;
        
        /* Multiple dependent FP operations */
        double x = prev * 1.1;
        double y = x * x - 0.5;
        arr[i] = y + arr[i] * 0.3;
        
        /* Another dependent operation */
        if (i % 3 == 0) {
            arr[i] *= 2.0;                     /* Conditional operation */
        }
        
        total += arr[i];
    }
    return total;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : SIZE;
    int n = size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    int* arr2 = (int*)malloc(n * sizeof(int));
    double* arr3 = (double*)malloc(n * sizeof(double));
    float** ptrs = (float**)malloc(n * sizeof(float*));
    float* data = (float*)malloc(n * sizeof(float));
    long* arr4 = (long*)malloc(n * sizeof(long));
    double* arr5 = (double*)malloc(n * sizeof(double));
    int* mask = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i * 2;
        arr3[i] = (double)(i % 50) * 0.2;
        data[i] = (float)(i % 30) * 0.3f;
        ptrs[i] = &data[(i + 1) % n];          /* Create pointer chain */
        arr4[i] = i * 3L;
        arr5[i] = (double)(i % 40) * 0.4;
        mask[i] = i % 2;
    }
    
    double result = 0.0;
    
    /* Call test functions to trigger modulo scheduling */
    result += test_recurrence_fp(arr1, n, 1.05);
    result += (double)test_mixed_recurrence(arr2, arr3, n);
    result += (double)test_pointer_chase(ptrs, data, n);
    result += (double)test_multi_dependence(arr4, arr5, n);
    result += test_conditional_recurrence(arr1, mask, n);
    
    /* Use results to prevent dead code elimination */
    printf("Final result: %f\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(ptrs);
    free(data);
    free(arr4);
    free(arr5);
    free(mask);
    
    return (int)result % 256;
}

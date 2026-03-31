/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep loop structure intact */
#define NOINLINE __attribute__((noinline))

/* Volatile to prevent constant propagation */
static volatile int g_iterations = 1000;

/* Test 1: Floating-point recurrence with mixed operations */
NOINLINE double test_fp_recurrence(double* arr, int n) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * 1.01;      /* FP multiply */
        temp += (double)i * 0.5;            /* FP add after conversion */
        arr[i] = temp + arr[i];             /* Another FP add */
        sum += arr[i];                      /* Accumulator */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
NOINLINE int test_int_recurrence(int* arr, int* brr, int n) {
    int sum = 0;
    /* Complex loop-carried chain */
    for (int i = 1; i < n; i++) {
        /* Multiple dependences across iterations */
        int val = arr[i-1] * 3;             /* Integer multiply */
        val += brr[i] >> 2;                 /* Shift operation */
        arr[i] = val + i;                   /* Integer add */
        brr[i] = arr[i-1] + brr[i-1];       /* Another distance-1 dependence */
        sum += arr[i] + brr[i];
    }
    return sum;
}

/* Test 3: Pointer-chasing recurrence */
NOINLINE double test_ptr_chase(double** ptrs, double* data, int n) {
    double sum = 0.0;
    /* Pointer-based recurrence */
    for (int i = 0; i < n-1; i++) {
        /* Load through pointer from previous iteration */
        double prev = *ptrs[i];
        /* FP operations with latency */
        double curr = prev * 2.5 + data[i];
        *ptrs[i+1] = curr;                  /* Store for next iteration */
        sum += curr;
        
        /* Additional operations to increase DDG complexity */
        data[i+1] = data[i] * 0.99 + curr;
    }
    return sum;
}

/* Test 4: Mixed FP/Int recurrence with conditional */
NOINLINE float test_mixed_recurrence(float* farr, int* iarr, int n) {
    float sum = 0.0f;
    /* Multiple recurrence chains */
    for (int i = 2; i < n; i++) {
        /* FP chain with distance 1 */
        float f1 = farr[i-1] * 1.5f;
        float f2 = f1 + farr[i-2] * 0.5f;   /* Distance 2 */
        farr[i] = f2 + (float)iarr[i];
        
        /* Integer chain with distance 1 */
        int i1 = iarr[i-1] + i;
        iarr[i] = i1 * 2 - iarr[i-2];       /* Distance 2 */
        
        /* Conditional to add control flow complexity */
        if (iarr[i] > 0) {
            sum += farr[i] * 2.0f;
        } else {
            sum += farr[i];
        }
    }
    return sum;
}

/* Test 5: Nested recurrence for complex DDG */
NOINLINE double test_nested_recurrence(double* a, double* b, double* c, int n) {
    double sum = 0.0;
    /* Multiple interacting recurrence chains */
    for (int i = 3; i < n; i++) {
        /* Chain 1: a[i] depends on a[i-1] and a[i-2] */
        double a1 = a[i-1] * 0.8;
        double a2 = a[i-2] * 0.2;
        a[i] = a1 + a2 + b[i];
        
        /* Chain 2: b[i] depends on b[i-1] and c[i-1] */
        double b1 = b[i-1] * 1.1;
        double b2 = c[i-1] * 0.9;
        b[i] = b1 - b2;
        
        /* Chain 3: c[i] depends on a[i-1] and c[i-3] */
        double c1 = a[i-1] * 0.7;
        double c2 = c[i-3] * 0.3;
        c[i] = c1 + c2;
        
        /* Complex reduction */
        sum += a[i] * b[i] + c[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent compile-time computation */
    volatile int n = g_iterations;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    int size = n;
    double total = 0.0;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(size * sizeof(double));
    int* arr2 = (int*)malloc(size * sizeof(int));
    int* arr3 = (int*)malloc(size * sizeof(int));
    double* arr4 = (double*)malloc(size * sizeof(double));
    double* arr5 = (double*)malloc(size * sizeof(double));
    double* arr6 = (double*)malloc(size * sizeof(double));
    float* arr7 = (float*)malloc(size * sizeof(float));
    int* arr8 = (int*)malloc(size * sizeof(int));
    double** ptrs = (double**)malloc(size * sizeof(double*));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = i % 30;
        arr4[i] = (double)(i % 80) * 0.2;
        arr5[i] = (double)(i % 60) * 0.3;
        arr6[i] = (double)(i % 70) * 0.4;
        arr7[i] = (float)(i % 90) * 0.1f;
        arr8[i] = i % 40;
        ptrs[i] = &arr4[i];
    }
    
    /* Call test functions to trigger modulo scheduling */
    total += test_fp_recurrence(arr1, size);
    total += (double)test_int_recurrence(arr2, arr3, size);
    total += test_ptr_chase(ptrs, arr4, size);
    total += (double)test_mixed_recurrence(arr7, arr8, size);
    total += test_nested_recurrence(arr5, arr6, arr4, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    free(arr6);
    free(arr7);
    free(arr8);
    free(ptrs);
    
    return 0;
}

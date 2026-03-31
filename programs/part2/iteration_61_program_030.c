/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep loop structures intact */
#define NOINLINE __attribute__((noinline))

/* Volatile variables to prevent constant propagation */
volatile int g_volatile_size = 1000;
volatile double g_volatile_factor = 1.01;

/* Test 1: Floating-point recurrence with mixed operations */
NOINLINE double test_fp_recurrence(double* arr, int size) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < size; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * g_volatile_factor;  /* FP multiply */
        arr[i] = temp + (double)i * 0.5;             /* FP add + conversion */
        sum += arr[i];                               /* Accumulator */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
NOINLINE int test_int_recurrence(int* arr, int size) {
    int sum = 0;
    /* Complex loop-carried chain */
    for (int i = 2; i < size; i++) {
        /* Multiple dependences across iterations */
        int t1 = arr[i-2] * 3;      /* Integer multiply */
        int t2 = t1 + arr[i-1];     /* Integer add with previous iteration */
        arr[i] = t2 ^ (i * 7);      /* XOR operation */
        sum += arr[i];
        
        /* Additional intra-iteration dependence */
        arr[i] += (arr[i] % 17);    /* Modulo operation */
    }
    return sum;
}

/* Test 3: Pointer-chasing recurrence */
NOINLINE double test_ptr_chase(double** ptrs, double* data, int size) {
    double sum = 0.0;
    /* Set up initial pointer */
    ptrs[0] = &data[0];
    
    /* Pointer-based loop-carried dependence */
    for (int i = 1; i < size - 1; i++) {
        /* Load through pointer from previous iteration */
        double val = *ptrs[i-1];
        
        /* FP operation with latency */
        val = val * 1.5 + (double)i;
        
        /* Store and update pointer for next iteration */
        data[i] = val;
        ptrs[i] = &data[i];
        
        sum += val;
    }
    return sum;
}

/* Test 4: Mixed FP/Int recurrence with conditional */
NOINLINE float test_mixed_recurrence(float* farr, int* iarr, int size) {
    float sum = 0.0f;
    /* Initialize */
    farr[0] = 1.0f;
    iarr[0] = 1;
    
    for (int i = 1; i < size; i++) {
        /* Loop-carried FP dependence */
        float ftemp = farr[i-1] * 1.1f;
        
        /* Loop-carried integer dependence */
        int itemp = iarr[i-1] + i;
        
        /* Mix types with conversion latency */
        farr[i] = ftemp + (float)itemp;
        iarr[i] = itemp % 256;
        
        /* Conditional with data dependence */
        if (iarr[i] > 128) {
            farr[i] *= 2.0f;  /* Additional FP operation */
        }
        
        sum += farr[i];
    }
    return sum;
}

/* Test 5: Complex multi-stage recurrence */
NOINLINE double test_complex_recurrence(double* arr1, double* arr2, int size) {
    double sum = 0.0;
    /* Initialize */
    arr1[0] = 1.0;
    arr2[0] = 2.0;
    
    /* Multiple interleaved recurrences */
    for (int i = 1; i < size; i++) {
        /* First recurrence chain */
        double a = arr1[i-1] * 1.01;
        double b = arr2[i-1] * 0.99;
        
        /* Cross-iteration mixing */
        arr1[i] = a + b * 0.5;
        arr2[i] = b - a * 0.3;
        
        /* Additional operations with intra-iteration dependence */
        arr1[i] = arr1[i] * arr1[i] + 1.0;
        arr2[i] = arr2[i] / (double)(i + 1);
        
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent compile-time optimization */
    volatile int size = (argc > 1) ? atoi(argv[1]) : g_volatile_size;
    if (size < 10) size = 1000;
    
    /* Allocate arrays with volatile to prevent optimization */
    double* fp_arr = (double*)malloc(size * sizeof(double));
    int* int_arr = (int*)malloc(size * sizeof(int));
    float* float_arr = (float*)malloc(size * sizeof(float));
    double** ptr_arr = (double**)malloc(size * sizeof(double*));
    double* data_arr = (double*)malloc(size * sizeof(double));
    double* arr1 = (double*)malloc(size * sizeof(double));
    double* arr2 = (double*)malloc(size * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        fp_arr[i] = (double)(i % 100) * 0.1;
        int_arr[i] = i % 97;
        float_arr[i] = (float)i * 0.3f;
        data_arr[i] = (double)(i % 50) * 0.2;
        arr1[i] = (double)(i % 30) * 0.15;
        arr2[i] = (double)(i % 40) * 0.25;
    }
    
    double total = 0.0;
    
    /* Run all tests to trigger different modulo scheduling scenarios */
    total += test_fp_recurrence(fp_arr, size);
    total += (double)test_int_recurrence(int_arr, size);
    total += test_ptr_chase(ptr_arr, data_arr, size);
    total += (double)test_mixed_recurrence(float_arr, int_arr, size);
    total += test_complex_recurrence(arr1, arr2, size);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(fp_arr);
    free(int_arr);
    free(float_arr);
    free(ptr_arr);
    free(data_arr);
    free(arr1);
    free(arr2);
    
    return 0;
}

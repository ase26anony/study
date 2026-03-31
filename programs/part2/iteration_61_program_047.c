/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimization and inlining */
static volatile int g_volatile_size = SIZE;
static volatile double g_volatile_factor = 1.01;

/* Test 1: Floating-point recurrence with mixed operations */
__attribute__((noinline))
double test_fp_recurrence(double* arr, int n) {
    double sum = 0.0;
    double factor = g_volatile_factor;
    
    /* Loop with distance-1 dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * factor;      /* FP multiply - higher latency */
        temp = temp + (double)i;              /* FP add */
        arr[i] = temp * 0.99;                 /* Another FP multiply */
        sum += arr[i];                        /* Accumulator */
    }
    
    /* Additional operations to create more DDG nodes */
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] + 1.0;
    }
    
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
__attribute__((noinline))
int test_int_recurrence(int* arr, int* brr, int n) {
    int sum = 0;
    
    /* Complex loop with multiple dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through arr */
        int val1 = arr[i-1] * 3;              /* Integer multiply */
        int val2 = brr[i] + val1;             /* Integer add */
        
        /* Another distance-1 dependence through brr */
        int val3 = brr[i-1] >> 2;             /* Shift operation */
        
        arr[i] = val2 + val3;                 /* Store with combined dependence */
        brr[i] = arr[i] * 2;                  /* Another multiply */
        
        sum += arr[i] + brr[i];               /* Complex accumulator */
    }
    
    return sum;
}

/* Test 3: Pointer-chasing recurrence */
__attribute__((noinline))
int test_pointer_chase(int** ptr_arr, int* data, int n) {
    int sum = 0;
    
    /* Initialize pointer chain */
    for (int i = 0; i < n; i++) {
        ptr_arr[i] = &data[i];
    }
    
    /* Pointer-chasing loop with distance-1 dependence */
    for (int i = 1; i < n; i++) {
        /* Load through pointer from previous iteration */
        int prev_val = *ptr_arr[i-1];
        
        /* Compute new value with multiple operations */
        int new_val = prev_val * 7 + i;
        new_val = new_val >> 1;
        
        /* Store and update pointer */
        data[i] = new_val;
        ptr_arr[i] = &data[i];
        
        sum += new_val;
    }
    
    return sum;
}

/* Test 4: Mixed FP/INT recurrence with conditional */
__attribute__((noinline))
double test_mixed_recurrence(double* farr, int* iarr, int n) {
    double sum = 0.0;
    double factor = 1.001;
    
    for (int i = 1; i < n; i++) {
        /* FP distance-1 dependence */
        double fp_val = farr[i-1] * factor;
        
        /* INT distance-1 dependence */
        int int_val = iarr[i-1] + i;
        
        /* Mixed operation */
        farr[i] = fp_val + (double)int_val;
        
        /* Conditional store to prevent simple optimization */
        if (int_val > 0) {
            iarr[i] = int_val * 2;
        } else {
            iarr[i] = 1;
        }
        
        /* Memory access with potential aliasing */
        sum += farr[i] + iarr[i];
    }
    
    return sum;
}

/* Test 5: Nested recurrence for complex DDG */
__attribute__((noinline))
int test_nested_recurrence(int* arr, int n) {
    int sum = 0;
    
    for (int i = 2; i < n; i++) {
        /* Multiple distance-1 dependences */
        int val1 = arr[i-1] + arr[i-2];       /* Depends on two previous iterations */
        int val2 = val1 * 3;
        int val3 = arr[i-1] >> 1;
        
        /* Complex computation with multiple uses */
        arr[i] = (val2 + val3) * 5;
        
        /* Additional operations to increase DDG size */
        int temp = arr[i] + i;
        arr[i] = temp - 1;
        
        sum += arr[i];
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int size = g_volatile_size;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = SIZE;
    }
    
    /* Allocate and initialize arrays */
    double* farr = (double*)malloc(size * sizeof(double));
    int* iarr1 = (int*)malloc(size * sizeof(int));
    int* iarr2 = (int*)malloc(size * sizeof(int));
    int** ptr_arr = (int**)malloc(size * sizeof(int*));
    int* data_arr = (int*)malloc(size * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < size; i++) {
        farr[i] = (double)(i % 100) * 0.1;
        iarr1[i] = i % 50;
        iarr2[i] = i % 30;
        data_arr[i] = i % 20;
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test_fp_recurrence(farr, size);
    total += (double)test_int_recurrence(iarr1, iarr2, size);
    total += (double)test_pointer_chase(ptr_arr, data_arr, size);
    total += test_mixed_recurrence(farr, iarr1, size);
    total += (double)test_nested_recurrence(iarr2, size);
    
    /* Use results to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(farr);
    free(iarr1);
    free(iarr2);
    free(ptr_arr);
    free(data_arr);
    
    return (int)total % 256;
}

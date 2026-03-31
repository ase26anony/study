/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimization and inlining */
static volatile int g_size = SIZE;
static volatile double g_factor = 1.01;

/* Test 1: Floating-point recurrence with mixed operations */
__attribute__((noinline))
double test1_fp_recurrence(double* arr, int n) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        double temp = arr[i-1] * g_factor;  /* FP multiply with latency */
        arr[i] = temp + (double)i;          /* FP add with recurrence */
        sum += arr[i];                      /* Additional accumulation */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
__attribute__((noinline))
int test2_int_recurrence(int* arr, int* brr, int n) {
    int sum = 0;
    /* Complex loop-carried dependence chain */
    for (int i = 1; i < n; i++) {
        int val1 = arr[i-1] * 3;            /* Integer multiply */
        int val2 = brr[i] + val1;           /* Integer add */
        arr[i] = val2 >> 2;                 /* Shift operation */
        brr[i-1] = arr[i] + i;              /* Store with aliasing */
        sum += arr[i] + brr[i-1];           /* Multiple uses */
    }
    return sum;
}

/* Test 3: Pointer-chasing recurrence */
__attribute__((noinline))
double test3_ptr_chase(double** ptrs, double* data, int n) {
    double sum = 0.0;
    /* Set up initial pointer */
    ptrs[0] = &data[0];
    
    /* Pointer-chasing loop with distance-1 dependence */
    for (int i = 1; i < n; i++) {
        double* prev_ptr = ptrs[i-1];
        double val = *prev_ptr * 2.5;       /* Load + FP multiply */
        data[i] = val + (double)(i * i);    /* FP add with computation */
        ptrs[i] = &data[i];                 /* Pointer assignment */
        sum += val;
    }
    return sum;
}

/* Test 4: Mixed FP/Int recurrence with conditional */
__attribute__((noinline))
double test4_mixed_recurrence(double* darr, int* iarr, int n) {
    double sum = 0.0;
    /* Multiple interleaved recurrences */
    for (int i = 1; i < n; i++) {
        /* FP recurrence chain */
        double fp_val = darr[i-1] * 1.5;
        darr[i] = fp_val + darr[i];
        
        /* Integer recurrence chain */
        int int_val = iarr[i-1] * 2;
        iarr[i] = int_val + i;
        
        /* Cross-type computation */
        sum += darr[i] * (double)iarr[i];
        
        /* Conditional to prevent over-optimization */
        if (iarr[i] > 1000) {
            darr[i] *= 0.99;
        }
    }
    return sum;
}

/* Test 5: Nested recurrence for complex DDG */
__attribute__((noinline))
double test5_nested_recurrence(double* arr, int n) {
    double sum = 0.0;
    double acc1 = arr[0];
    double acc2 = arr[0] * 0.5;
    
    /* Two parallel recurrence chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: acc1 depends on previous acc1 */
        acc1 = acc1 * g_factor + (double)i;
        
        /* Chain 2: acc2 depends on previous acc2 and acc1 */
        acc2 = acc2 * 0.8 + acc1 * 0.2;
        
        /* Store both with potential aliasing */
        arr[i] = acc1 + acc2;
        
        /* Additional computation with latency */
        sum += arr[i] * arr[i-1];  /* Distance-1 dependence */
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use command line or volatile to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : g_size;
    if (n < 10) n = 10;
    if (n > 10000) n = 10000;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    double* arr2 = (double*)malloc(n * sizeof(double));
    int* iarr1 = (int*)malloc(n * sizeof(int));
    int* iarr2 = (int*)malloc(n * sizeof(int));
    double** ptrs = (double**)malloc(n * sizeof(double*));
    double* data = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        iarr1[i] = i % 100;
        iarr2[i] = i % 200;
        data[i] = (double)(i * 2) * 0.05;
    }
    
    double total = 0.0;
    
    /* Execute all test functions to trigger modulo scheduling */
    total += test1_fp_recurrence(arr1, n);
    total += (double)test2_int_recurrence(iarr1, iarr2, n);
    total += test3_ptr_chase(ptrs, data, n);
    total += test4_mixed_recurrence(arr2, iarr1, n);
    total += test5_nested_recurrence(arr1, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr1);
    free(iarr2);
    free(ptrs);
    free(data);
    
    return (int)total % 100;
}

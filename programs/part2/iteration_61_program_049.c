/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent optimization and inlining */
volatile int g_volatile_size = SIZE;
volatile double g_volatile_factor = 1.01;

/* Test 1: Floating-point recurrence with mixed operations */
__attribute__((noinline))
double test_fp_recurrence(double* arr, int n) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * g_volatile_factor;  /* FP multiply */
        arr[i] = temp + arr[i] * 0.5;                /* FP multiply + add */
        sum += arr[i];                               /* FP add */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
__attribute__((noinline))
int test_int_recurrence(int* arr, int* brr, int n) {
    int sum = 0;
    /* Complex loop-carried dependence chain */
    for (int i = 1; i < n; i++) {
        /* Multiple dependences across iterations */
        int val1 = arr[i-1] * brr[i];      /* Integer multiply */
        int val2 = val1 + i;               /* Integer add */
        arr[i] = val2 * 3;                 /* Another multiply */
        brr[i] = arr[i-1] + brr[i-1];      /* Cross-iteration dependence */
        sum += arr[i] + brr[i];
    }
    return sum;
}

/* Test 3: Pointer-chasing recurrence */
__attribute__((noinline))
double test_pointer_chase(double* arr, int n) {
    double* ptr = arr;
    double sum = 0.0;
    
    /* Pointer-based loop-carried dependence */
    for (int i = 0; i < n-1; i++) {
        double* next_ptr = ptr + 1;
        /* Load from current, store to next with computation */
        *next_ptr = (*ptr) * 1.5 + (*next_ptr) * 0.5;
        sum += *next_ptr;
        ptr = next_ptr;  /* Pointer moves forward each iteration */
    }
    return sum;
}

/* Test 4: Mixed FP/Int recurrence with conditional */
__attribute__((noinline))
double test_mixed_recurrence(double* darr, int* iarr, int n) {
    double sum = 0.0;
    /* Multiple loop-carried dependences */
    for (int i = 1; i < n; i++) {
        /* FP operation depending on previous iteration */
        double fp_val = darr[i-1] * 1.02;
        
        /* Integer operation also with loop-carried dependence */
        int int_val = iarr[i-1] + i;
        
        /* Mix them together */
        darr[i] = fp_val + (double)int_val * 0.1;
        iarr[i] = int_val * 2;
        
        /* Conditional to add complexity */
        if (darr[i] > 100.0) {
            darr[i] = darr[i] * 0.99;
        }
        
        sum += darr[i] + iarr[i];
    }
    return sum;
}

/* Test 5: Nested recurrence for more complex DDG */
__attribute__((noinline))
double test_nested_recurrence(double* arr, int n) {
    double sum = 0.0;
    /* Two separate recurrence chains */
    double chain1 = arr[0];
    double chain2 = arr[0] * 0.5;
    
    for (int i = 1; i < n; i++) {
        /* First recurrence chain */
        chain1 = chain1 * 1.01 + arr[i];
        
        /* Second recurrence chain depending on first */
        chain2 = chain2 * 0.99 + chain1 * 0.1;
        
        /* Combine chains */
        arr[i] = chain1 + chain2;
        
        /* Additional computation to increase DDG size */
        double temp = arr[i] * arr[i-1];  /* Cross-iteration multiply */
        arr[i] = temp * 0.5 + arr[i];
        
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent constant propagation */
    volatile int use_size = g_volatile_size;
    if (argc > 1) {
        use_size = atoi(argv[1]);
        if (use_size < 10) use_size = 100;
    }
    
    int n = use_size;
    
    /* Allocate and initialize arrays */
    double* arr1 = (double*)malloc(n * sizeof(double));
    double* arr2 = (double*)malloc(n * sizeof(double));
    int* iarr1 = (int*)malloc(n * sizeof(int));
    int* iarr2 = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = (double)(i % 50) * 0.2;
        iarr1[i] = i % 77;
        iarr2[i] = i % 88;
    }
    
    double total_sum = 0.0;
    
    /* Call each test function to trigger different modulo scheduling scenarios */
    total_sum += test_fp_recurrence(arr1, n);
    total_sum += (double)test_int_recurrence(iarr1, iarr2, n);
    total_sum += test_pointer_chase(arr2, n);
    total_sum += test_mixed_recurrence(arr1, iarr1, n);
    total_sum += test_nested_recurrence(arr2, n);
    
    /* Use the result to prevent dead code elimination */
    printf("Total checksum: %f\n", total_sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(iarr1);
    free(iarr2);
    
    return 0;
}

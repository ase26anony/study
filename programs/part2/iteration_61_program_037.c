/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging code
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep loop structures intact */
#define NOINLINE __attribute__((noinline))

/* Test 1: Floating-point recurrence with mixed operations */
NOINLINE double test_fp_recurrence(double *arr, int n, double factor) {
    double sum = 0.0;
    
    /* Loop with distance-1 dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * factor;      /* FP multiply - higher latency */
        temp = temp + (double)i;              /* FP add */
        arr[i] = temp * 0.99;                 /* Another FP multiply */
        sum += arr[i];                        /* Accumulator for side effect */
    }
    
    /* Additional operations to create more DDG edges */
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 1.01 + (double)i;
    }
    
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
NOINLINE int test_int_recurrence(int *arr, int n, int *mask) {
    int total = 0;
    
    /* Complex loop with multiple dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through arr */
        int val = arr[i-1] * 3;               /* Integer multiply */
        val = val + mask[i % 16];             /* Memory load with potential aliasing */
        arr[i] = val >> 2;                    /* Shift operation */
        
        /* Another distance-1 chain */
        mask[(i-1) % 16] = mask[i % 16] + 1;
        
        total += arr[i];
    }
    
    return total;
}

/* Test 3: Pointer-chasing recurrence */
NOINLINE double test_ptr_chase(double **ptr_arr, double *data, int n) {
    double result = 0.0;
    
    /* Set up initial pointer */
    ptr_arr[0] = &data[0];
    
    /* Pointer-chasing loop with distance-1 dependence */
    for (int i = 1; i < n; i++) {
        /* Load through pointer from previous iteration */
        double prev_val = *ptr_arr[i-1];
        
        /* Compute new value with FP operations */
        double new_val = prev_val * 1.5 + (double)i;
        
        /* Store and update pointer */
        data[i] = new_val;
        ptr_arr[i] = &data[i];
        
        result += new_val;
    }
    
    return result;
}

/* Test 4: Mixed-type recurrence with conditional */
NOINLINE float test_mixed_recurrence(float *farr, int *iarr, int n) {
    float sum = 0.0f;
    
    /* Initialize first element */
    if (n > 0) {
        farr[0] = 1.0f;
        iarr[0] = 1;
    }
    
    /* Loop with both float and int recurrences */
    for (int i = 1; i < n; i++) {
        /* Float recurrence (distance-1) */
        float fval = farr[i-1] * 1.1f;
        fval = fval + (float)iarr[i-1];  /* Mix with int */
        
        /* Integer recurrence (distance-1) */
        int ival = iarr[i-1] * 2;
        ival = ival + (int)fval;         /* Mix with float */
        
        /* Conditional store creates control dependence */
        if (ival % 3 == 0) {
            fval = fval * 0.9f;
        }
        
        farr[i] = fval;
        iarr[i] = ival % 100;
        
        sum += fval;
    }
    
    return sum;
}

/* Test 5: Complex recurrence with multiple chains */
NOINLINE double test_complex_recurrence(double *a, double *b, double *c, int n) {
    double total = 0.0;
    
    /* Initialize */
    if (n > 0) {
        a[0] = 1.0;
        b[0] = 2.0;
        c[0] = 3.0;
    }
    
    /* Loop with multiple interacting recurrence chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: a[i] depends on a[i-1] and b[i-1] */
        double tmp1 = a[i-1] * b[i-1];
        tmp1 = tmp1 + (double)i;
        
        /* Chain 2: b[i] depends on b[i-1] and c[i-1] */
        double tmp2 = b[i-1] / c[i-1];
        tmp2 = tmp2 * 1.1;
        
        /* Chain 3: c[i] depends on all previous values */
        double tmp3 = a[i-1] + b[i-1] + c[i-1];
        tmp3 = tmp3 * 0.95;
        
        /* Cross-chain dependencies */
        a[i] = tmp1 * tmp2;
        b[i] = tmp2 + tmp3;
        c[i] = tmp3 - tmp1;
        
        total += a[i] + b[i] + c[i];
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int size = 1000;
    if (argc > 1) {
        size = atoi(argv[1]);
    }
    
    int n = size;
    if (n < 16) n = 16;
    
    /* Allocate arrays with volatile pointers to prevent optimizations */
    double *arr1 = (double*)malloc(n * sizeof(double));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *mask = (int*)malloc(16 * sizeof(int));
    double *data = (double*)malloc(n * sizeof(double));
    double **ptr_arr = (double**)malloc(n * sizeof(double*));
    float *farr = (float*)malloc(n * sizeof(float));
    int *iarr = (int*)malloc(n * sizeof(int));
    double *a = (double*)malloc(n * sizeof(double));
    double *b = (double*)malloc(n * sizeof(double));
    double *c = (double*)malloc(n * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100);
        arr2[i] = i;
        if (i < 16) mask[i] = i * 3;
        data[i] = (double)(i * 2);
        ptr_arr[i] = NULL;
        farr[i] = (float)(i * 0.5);
        iarr[i] = i * 2;
        a[i] = (double)i;
        b[i] = (double)(i + 1);
        c[i] = (double)(i + 2);
    }
    
    double total = 0.0;
    
    /* Call test functions to trigger modulo scheduling analysis */
    total += test_fp_recurrence(arr1, n, 1.05);
    total += (double)test_int_recurrence(arr2, n, mask);
    total += test_ptr_chase(ptr_arr, data, n);
    total += (double)test_mixed_recurrence(farr, iarr, n);
    total += test_complex_recurrence(a, b, c, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(mask);
    free(data);
    free(ptr_arr);
    free(farr);
    free(iarr);
    free(a);
    free(b);
    free(c);
    
    return 0;
}

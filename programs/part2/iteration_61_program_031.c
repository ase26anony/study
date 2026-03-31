/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with distance-1 dependences
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep loop structures intact */
#define NOINLINE __attribute__((noinline))

/* Volatile to prevent constant propagation */
static volatile int g_iterations = 1000;

/* Test 1: Floating-point recurrence with mixed operations */
NOINLINE double test_fp_recurrence(double *arr, int n) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < n; i++) {
        /* Multiple operations with different latencies */
        double temp = arr[i-1] * 1.01;      /* FP multiply (higher latency) */
        temp += (double)i * 0.5;            /* FP add + conversion */
        arr[i] = temp + arr[i];             /* Another FP add */
        sum += arr[i];                      /* Accumulator with FP add */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
NOINLINE int test_int_recurrence(int *arr, int *brr, int n) {
    int sum = 0;
    /* Complex loop-carried dependence chain */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through arr */
        int val = arr[i-1] * 3;             /* Integer multiply */
        val += brr[i];                      /* Memory load */
        arr[i] = val >> 2;                  /* Shift operation */
        
        /* Another distance-1 dependence through brr */
        brr[i] = brr[i-1] + arr[i];         /* Integer add with load */
        
        sum += arr[i] + brr[i];             /* Multiple uses */
    }
    return sum;
}

/* Test 3: Pointer-chasing recurrence */
NOINLINE double test_ptr_chase(double **ptrs, double *data, int n) {
    double sum = 0.0;
    /* Set up initial pointer */
    ptrs[0] = &data[0];
    
    /* Pointer-based recurrence: ptrs[i] depends on data from previous iteration */
    for (int i = 1; i < n; i++) {
        /* Load through pointer from previous iteration */
        double prev_val = *ptrs[i-1];
        
        /* Compute new value with FP operations */
        double new_val = prev_val * 1.5 + (double)i;
        
        /* Store and update pointer */
        data[i] = new_val;
        ptrs[i] = &data[i];
        
        /* Additional computation to increase DDG complexity */
        sum += new_val * 0.75;
    }
    return sum;
}

/* Test 4: Mixed FP/Integer with multiple recurrences */
NOINLINE float test_mixed_recurrence(float *farr, int *iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    /* Two independent recurrence chains */
    for (int i = 1; i < n; i++) {
        /* FP recurrence chain */
        float ftemp = farr[i-1] * 1.1f;
        ftemp += (float)iarr[i] * 0.01f;
        farr[i] = ftemp;
        fsum += ftemp;
        
        /* Integer recurrence chain */
        int itemp = iarr[i-1] * 2;
        itemp += (int)farr[i];
        iarr[i] = itemp % 1000;
        isum += itemp;
    }
    
    return fsum + (float)isum;
}

/* Test 5: Complex loop with conditional recurrence */
NOINLINE int test_conditional_recurrence(int *arr, int n) {
    int sum = 0;
    int prev = arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Loop-carried dependence through prev variable */
        int current;
        if (prev > 100) {
            current = prev / 2;      /* Integer division */
        } else {
            current = prev * 3 + 1;  /* Integer multiply-add */
        }
        
        /* Memory store with potential aliasing */
        arr[i] = current;
        sum += current;
        
        /* Update for next iteration */
        prev = current;
        
        /* Additional operations to create more DDG edges */
        sum += i & 0xF;  /* Bitwise AND */
    }
    return sum;
}

int main(int argc, char **argv) {
    /* Use command line or volatile for loop bounds */
    int n = g_iterations;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;  /* Ensure enough iterations */
    }
    
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(n * sizeof(double));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    double *data = (double*)malloc(n * sizeof(double));
    double **ptrs = (double**)malloc(n * sizeof(double*));
    float *farr = (float*)malloc(n * sizeof(float));
    int *iarr = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (double)(i % 100) * 0.1;
        arr2[i] = i % 50;
        arr3[i] = i % 30;
        data[i] = (double)(i % 80) * 0.2;
        farr[i] = (float)(i % 60) * 0.3f;
        iarr[i] = i % 40;
    }
    
    double total = 0.0;
    
    /* Run all tests to trigger different modulo scheduling scenarios */
    total += test_fp_recurrence(arr1, n);
    total += test_int_recurrence(arr2, arr3, n);
    total += test_ptr_chase(ptrs, data, n);
    total += test_mixed_recurrence(farr, iarr, n);
    total += test_conditional_recurrence(arr2, n);
    
    /* Use results to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(data);
    free(ptrs);
    free(farr);
    free(iarr);
    
    return 0;
}

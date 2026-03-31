/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler with loop-carried dependences
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
        double temp = arr[i-1] * factor;      /* FP multiply */
        temp = temp + (double)i;              /* FP add */
        arr[i] = temp * 0.99;                 /* Another FP multiply */
        sum += arr[i];                        /* Accumulator */
    }
    
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
NOINLINE int test_int_recurrence(int *arr, int n, int *mask) {
    int total = 0;
    
    /* Complex loop with multiple dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 dependence through memory */
        int prev = arr[i-1] & mask[i];        /* Integer ALU + memory */
        int curr = prev * 3;                   /* Integer multiply */
        curr = curr + (arr[i] >> 2);          /* Shift + add */
        arr[i] = curr;
        total ^= curr;                         /* XOR chain */
    }
    
    return total;
}

/* Test 3: Pointer-chasing recurrence */
NOINLINE float test_ptr_chase(float **ptrs, int n, float increment) {
    float result = 0.0f;
    
    /* Pointer-based distance-1 dependence */
    for (int i = 0; i < n-1; i++) {
        /* Load through pointer from previous iteration's target */
        float val = *ptrs[i];
        val = val * 1.5f + increment;         /* FP operations */
        *ptrs[i+1] = val;                     /* Store for next iteration */
        result += val;
    }
    
    return result;
}

/* Test 4: Mixed recurrence with conditional */
NOINLINE double test_mixed_recurrence(double *a, double *b, int n) {
    double acc = a[0];
    
    /* Loop with both intra and inter-iteration dependences */
    for (int i = 1; i < n; i++) {
        /* Distance-1 recurrence on acc */
        double t1 = acc * b[i];               /* FP multiply */
        double t2 = a[i] + t1;                /* FP add */
        
        /* Conditional creates control flow */
        if (t2 > 0.0) {
            acc = t2 * 0.8;                   /* Another FP multiply */
        } else {
            acc = t2 * 1.2;
        }
        
        /* Additional operations to increase DDG size */
        b[i] = acc * (double)i;
    }
    
    return acc;
}

/* Test 5: Nested recurrence for complex DDG */
NOINLINE int test_nested_recurrence(int *arr, int n) {
    int sum = arr[0];
    int prod = 1;
    
    /* Multiple recurrence chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: sum depends on previous sum */
        int new_sum = sum + arr[i];
        
        /* Chain 2: prod depends on previous prod */
        int new_prod = prod * (arr[i] & 0xFF);
        
        /* Cross-dependence between chains */
        arr[i] = new_sum ^ new_prod;
        
        /* Update recurrences */
        sum = new_sum;
        prod = new_prod % 256;
    }
    
    return sum + prod;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent constant propagation */
    volatile int size = (argc > 1) ? atoi(argv[1]) : 1000;
    int n = size;
    
    if (n < 10) n = 1000;  /* Ensure loops are non-trivial */
    
    /* Allocate and initialize test arrays */
    double *fp_arr = (double*)malloc(n * sizeof(double));
    int *int_arr = (int*)malloc(n * sizeof(int));
    int *mask_arr = (int*)malloc(n * sizeof(int));
    float **ptr_arr = (float**)malloc(n * sizeof(float*));
    float *float_storage = (float*)malloc(n * sizeof(float));
    double *arr_a = (double*)malloc(n * sizeof(double));
    double *arr_b = (double*)malloc(n * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        fp_arr[i] = (double)(i % 100) * 0.1;
        int_arr[i] = i * 3;
        mask_arr[i] = 0xFF ^ (i & 0xF);
        float_storage[i] = (float)i * 0.5f;
        ptr_arr[i] = &float_storage[i];
        arr_a[i] = (double)(i % 50) * 0.2;
        arr_b[i] = (double)(i % 30) * 0.3;
    }
    
    double total = 0.0;
    
    /* Run all test functions to trigger modulo scheduling */
    total += test_fp_recurrence(fp_arr, n, 1.01);
    total += (double)test_int_recurrence(int_arr, n, mask_arr);
    total += (double)test_ptr_chase(ptr_arr, n, 0.25f);
    total += test_mixed_recurrence(arr_a, arr_b, n);
    total += (double)test_nested_recurrence(int_arr, n);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %f\n", total);
    
    /* Cleanup */
    free(fp_arr);
    free(int_arr);
    free(mask_arr);
    free(ptr_arr);
    free(float_storage);
    free(arr_a);
    free(arr_b);
    
    return 0;
}

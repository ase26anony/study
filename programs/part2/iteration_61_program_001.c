/* test_modulo_sched.c
 * Designed to trigger GCC's modulo scheduler edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sms -c test_modulo_sched.c -mtune=itanium
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep loop structures intact */
#define NOINLINE __attribute__((noinline))

/* Test 1: Floating-point recurrence with mixed operations */
NOINLINE double test1_fp_recurrence(double *arr, int N, double factor) {
    double sum = 0.0;
    /* Loop-carried dependence: arr[i] depends on arr[i-1] */
    for (int i = 1; i < N; i++) {
        arr[i] = arr[i-1] * factor + (double)i;  /* Distance-1 dependence */
        arr[i] = arr[i] * 0.99;                  /* Intra-iteration operation */
        sum += arr[i];                           /* Reduction for side effect */
    }
    return sum;
}

/* Test 2: Integer recurrence with memory aliasing */
NOINLINE int test2_int_recurrence(int *arr, int N, int *mask) {
    int total = 0;
    /* Complex loop-carried chain with pointer aliasing */
    for (int i = 1; i < N; i++) {
        int prev = arr[i-1];                     /* Load with potential latency */
        int calc = prev * mask[i & 7];           /* Multiplication (higher latency) */
        arr[i] = calc + i;                       /* Store creating distance-1 dependence */
        total ^= arr[i];                         /* Side effect to prevent elimination */
    }
    return total;
}

/* Test 3: Mixed float/int recurrence with conditional */
NOINLINE float test3_mixed_recurrence(float *farr, int *iarr, int N) {
    float acc = 0.0f;
    /* Multiple loop-carried dependences */
    for (int i = 1; i < N; i++) {
        float fprev = farr[i-1];                 /* Float load */
        int iprev = iarr[i-1];                   /* Int load */
        
        farr[i] = fprev * 1.01f + (float)iprev;  /* Mixed operation */
        iarr[i] = iprev + (int)(fprev * 2.0f);   /* Float->int conversion */
        
        /* Additional operations to increase DDG complexity */
        acc += farr[i] * 0.5f;
        iarr[i] ^= (iarr[i] << 3);
    }
    return acc;
}

/* Test 4: Pointer-chasing recurrence (simulated linked list traversal) */
NOINLINE int test4_pointer_chase(int *base, int N, int stride) {
    int sum = 0;
    int *ptr = base;
    /* Simulate pointer chasing with distance-1 dependence */
    for (int i = 0; i < N-1; i++) {
        int val = *ptr;                          /* Load with latency */
        int next_idx = (val + stride) % N;       /* Compute next address */
        ptr = &base[next_idx];                   /* Pointer update */
        *ptr = val + i;                          /* Store to next location */
        sum += val;
    }
    return sum;
}

/* Test 5: Double-precision recurrence with multiple chains */
NOINLINE double test5_double_chain(double *darr, int N) {
    double chain1 = darr[0];
    double chain2 = darr[1];
    double result = 0.0;
    
    /* Two independent recurrence chains in same loop */
    for (int i = 2; i < N; i++) {
        /* First chain: distance-1 dependence */
        chain1 = chain1 * 1.0001 + darr[i];
        
        /* Second chain: also distance-1 */
        chain2 = chain2 * 0.9999 - darr[i-1];
        
        /* Cross-chain mixing */
        darr[i] = chain1 + chain2 * 0.5;
        
        /* Additional ops for scheduler complexity */
        result += darr[i] / (double)(i+1);
    }
    return result;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N < 10) N = 1000;  /* Ensure minimum size */
    
    /* Allocate arrays with different types */
    double *darr = (double*)malloc(N * sizeof(double));
    float *farr = (float*)malloc(N * sizeof(float));
    int *iarr = (int*)malloc(N * sizeof(int));
    int *mask = (int*)malloc(8 * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < N; i++) {
        darr[i] = (double)(i % 100) * 0.1;
        farr[i] = (float)(i % 50) * 0.2f;
        iarr[i] = i * 3;
    }
    for (int i = 0; i < 8; i++) {
        mask[i] = (i * 2 + 1);
    }
    
    double total = 0.0;
    
    /* Call all test functions to trigger modulo scheduling analysis */
    total += test1_fp_recurrence(darr, N, 1.05);
    total += (double)test2_int_recurrence(iarr, N, mask);
    total += (double)test3_mixed_recurrence(farr, iarr, N);
    total += (double)test4_pointer_chase(iarr, N, 7);
    total += test5_double_chain(darr, N);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Cleanup */
    free(darr);
    free(farr);
    free(iarr);
    free(mask);
    
    return 0;
}

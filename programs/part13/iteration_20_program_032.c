/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multiple_int_chains(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int acc1 = a[0];
    int acc2 = a[1] + a[0];
    int acc3 = b[0] * 2;
    int acc4 = c[0] ^ d[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple accumulation with dependency distance 1 */
        acc1 = acc1 + b[i] * 3;
        a[i] = acc1;
        
        /* Chain 2: More complex recurrence with multiple operations */
        acc2 = (acc2 * 7) + (c[i] << 2) - d[i];
        b[i] = acc2;
        
        /* Chain 3: Multiply-accumulate pattern */
        acc3 = acc3 * 5 + (a[i] & 0xFF);
        c[i] = acc3;
        
        /* Chain 4: XOR-shift recurrence */
        acc4 = (acc4 ^ (acc4 << 3)) + (d[i] * 11);
        d[i] = acc4;
        
        /* Additional operations to increase register pressure */
        int temp1 = a[i-1] + b[i-1];
        int temp2 = c[i-1] * d[i-1];
        int temp3 = temp1 ^ temp2;
        global_acc += temp3;
    }
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double prod1 = c[0];
    double prod2 = a[0] * b[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + a[i] * 1.5;
        sum2 = sum2 + b[i] * 2.5 - sum1 * 0.1;
        
        prod1 = prod1 * (1.0 + c[i] * 0.01);
        prod2 = prod2 * (0.95 + a[i] * b[i] * 0.001);
        
        /* Cross-chain dependencies */
        double mix1 = sum1 * prod1;
        double mix2 = sum2 * prod2;
        double mix3 = mix1 + mix2;
        
        a[i] = mix1;
        b[i] = mix2;
        c[i] = mix3;
        
        global_acc += (long long)(mix3 * 1000);
    }
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *end = data + n;
    
    int sum1 = 0;
    int sum2 = 0;
    int acc1 = *ptr1;
    int acc2 = *ptr2;
    
    while (ptr1 < end && ptr2 < end) {
        /* Two independent pointer-chasing chains */
        acc1 = acc1 * 3 + *ptr1;
        acc2 = acc2 * 5 + *ptr2;
        
        sum1 += acc1;
        sum2 += acc2;
        
        /* Additional operations on different strides */
        int diff = *ptr1 - *ptr2;
        sum1 += diff * diff;
        
        ptr1 += stride;
        ptr2 += stride;
        
        /* Modulo indexing to create complex addressing */
        int idx = (ptr1 - data) % 16;
        sum2 += data[idx] * 7;
    }
    
    global_acc += sum1 + sum2;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops(int *arr, int n) {
    int i;
    /* Variables with different dependency distances */
    int v1 = arr[0];      /* Distance 1 */
    int v2 = arr[0] + arr[1]; /* Distance 2 */
    int v3 = arr[0] * 3;  /* Distance 1 */
    int v4 = 1;           /* Distance 3 */
    
    for (i = 1; i < n; i++) {
        /* Chain with distance 1 */
        v1 = (v1 << 3) | (arr[i] & 0x7);
        arr[i] = v1;
        
        /* Chain with distance 2 (uses value from 2 iterations ago) */
        if (i >= 2) {
            v2 = v2 + arr[i-2] * 11;
            arr[i-1] ^= v2;
        }
        
        /* Chain with distance 1 but more operations */
        v3 = v3 * 17 + (arr[i] % 31);
        int temp = v3 & 0xFF;
        v3 = temp * 5;
        
        /* Chain with distance 3 */
        if (i >= 3) {
            v4 = v4 * arr[i-3] + 1;
        }
        
        /* Complex combination of all chains */
        int result = (v1 + v2) * (v3 - v4);
        global_acc += result;
    }
}

/* Test 5: Nested loops with inner loop being modulo-scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int base = a[i];
        int acc = base;
        
        /* Inner loop designed for modulo scheduling */
        for (j = 0; j < m; j++) {
            /* Multiple operations with carried dependencies */
            acc = acc * 3 + b[j];
            int prod = acc * c[j];
            int diff = prod - base;
            acc = diff / 2;
            
            /* Additional operations to increase register pressure */
            int t1 = b[j] << 2;
            int t2 = c[j] >> 1;
            int t3 = t1 ^ t2;
            acc += t3;
            
            global_acc += acc;
        }
        
        a[i] = acc;
    }
}

/* Test 6: PowerPC specific patterns using double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double sum = a[0];
    double prod = b[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple double precision operations */
        sum = sum + a[i] * 1.41421356237;
        prod = prod * (1.0 + b[i] * 0.01);
        
        /* Cross dependencies */
        double t1 = sum * prod;
        double t2 = sum + prod;
        double t3 = t1 - t2;
        
        a[i] = t1;
        b[i] = t2;
        
        global_acc += (long long)(t3 * 1000);
    }
}
#endif

/* Test 7: Vector-like operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
void test_vector_pattern(int *a, int *b, int *c, int n) {
    int i;
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    
    /* Manual unrolling to increase operations per iteration */
    for (i = 1; i < n - 3; i += 4) {
        /* Process multiple elements with dependencies */
        acc1 = acc1 * 7 + a[i];
        acc2 = acc2 * 11 + b[i];
        acc3 = acc3 * 13 + c[i];
        
        acc1 = acc1 * 3 + a[i+1];
        acc2 = acc2 * 5 + b[i+1];
        acc3 = acc3 * 7 + c[i+1];
        
        acc1 = acc1 * 2 + a[i+2];
        acc2 = acc2 * 3 + b[i+2];
        acc3 = acc3 * 5 + c[i+2];
        
        acc1 = acc1 * 11 + a[i+3];
        acc2 = acc2 * 13 + b[i+3];
        acc3 = acc3 * 17 + c[i+3];
        
        /* Cross-lane operations */
        int mix = (acc1 + acc2) * acc3;
        global_acc += mix;
        
        a[i] = acc1;
        b[i] = acc2;
        c[i] = acc3;
    }
}
#endif

/* Main test driver */
int main() {
    int i;
    
    /* Allocate and initialize test arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        darr1[i] = (double)(rand() % 100) / 10.0;
        darr2[i] = (double)(rand() % 100) / 10.0;
    }
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        /* Test 1: Multiple integer chains */
        test_multiple_int_chains(arr1, arr2, arr3, arr4, SIZE);
        
        /* Test 2: Floating point accumulation */
        test_float_accumulate(darr1, darr2, darr1, SIZE);
        
        /* Test 3: Pointer chasing */
        test_pointer_chasing(arr1, SIZE, 4);
        
        /* Test 4: Mixed operations */
        test_mixed_ops(arr2, SIZE);
        
        /* Test 5: Nested loops */
        test_nested_loops(arr3, arr4, arr1, SIZE/16, 16);
        
        /* Architecture-specific tests */
#ifdef __powerpc__
        test_powerpc_double(darr1, darr2, SIZE);
#endif
        
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_pattern(arr1, arr2, arr3, SIZE);
#endif
    }
    
    /* Output final result to ensure computations aren't optimized away */
    printf("Final accumulator value: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(darr1);
    free(darr2);
    
    return 0;
}

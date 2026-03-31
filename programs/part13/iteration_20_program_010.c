/* test-modulo-sched.c
 * 
 * Test program to trigger GCC's modulo scheduler register move logic
 * targeting uncovered lines in modulo-sched.cc:596-606
 * 
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test-modulo-sched.c -o test-modulo-sched
 * 
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains to increase register pressure */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        x1 = x1 * 3 + a[i];
        a[i] = x1;
        
        /* Chain 2: distance-1 with different operation */
        x2 = (x2 << 2) ^ b[i];
        b[i] = x2;
        
        /* Chain 3: distance-2 dependency */
        x3 = y3 * 5 - c[i];
        y3 = x3;
        c[i] = x3;
        
        /* Chain 4: mixed operations */
        x4 = (x4 + d[i]) * 7;
        d[i] = x4;
        
        /* Additional operations to increase register pressure */
        a[i] += b[i] & 0xFF;
        c[i] ^= d[i] >> 4;
    }
    
    /* Use results to prevent optimization */
    global_acc += x1 + x2 + x3 + x4;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];
        sum2 = sum2 * 0.99 + b[i];
        sum3 = sum3 + a[i] * b[i];
        
        /* Cross-iteration dependencies */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 / (1.0 + b[i] * 0.001);
        
        /* Additional operations for register pressure */
        a[i] = sum1 * 0.5 + prod1;
        b[i] = sum2 * 0.3 - prod2;
        c[i] = sum3 + a[i] * b[i];
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Mixed integer/float with strided access */
void test_mixed_strided(int *int_arr, float *float_arr, double *double_arr, int n) {
    int i, j;
    int int_acc = int_arr[0];
    float float_acc = float_arr[0];
    double double_acc = double_arr[0];
    
    /* Strided access pattern */
    for (i = 1; i < n - 3; i += 2) {
        /* Integer chain with distance 1 */
        int_acc = int_acc * 2 + int_arr[i];
        int_arr[i] = int_acc;
        
        /* Float chain */
        float_acc = float_acc * 1.5f + float_arr[i + 1];
        float_arr[i + 1] = float_acc;
        
        /* Double chain with dependency on previous iteration */
        double_acc = double_acc + double_arr[i] * 0.5;
        double_arr[i] = double_acc;
        
        /* Cross-type operations to increase complexity */
        int_arr[i + 2] = (int)(float_acc * 100) ^ int_acc;
        float_arr[i] = (float)(double_acc * 0.1) + float_acc;
    }
    
    global_acc += int_acc + (long long)(float_acc + double_acc);
}

/* Test 4: Pointer-chasing with arithmetic */
void test_pointer_chasing(int *arr, int n) {
    int *ptr1 = arr;
    int *ptr2 = arr + 1;
    int *ptr3 = arr + 2;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n - 3; i++) {
        /* Multiple pointer chains with dependencies */
        sum1 = sum1 + *ptr1 * 3;
        sum2 = sum2 ^ *ptr2;
        sum3 = sum3 * 2 + *ptr3;
        
        /* Update pointers with stride */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 1;
        
        /* Create anti-dependencies */
        *ptr1 = sum1 >> 1;
        *ptr2 = sum2 & 0xFF;
        *ptr3 = sum3 % 256;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: Nested loops with inner loop being modulo-scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    int outer_acc = 0;
    
    for (i = 0; i < n; i++) {
        int inner_acc1 = a[i];
        int inner_acc2 = b[i];
        int inner_acc3 = c[i];
        
        /* Innermost loop with carried dependencies */
        for (j = 1; j < m; j++) {
            /* Multiple dependency chains */
            inner_acc1 = inner_acc1 * 3 + a[i * m + j];
            inner_acc2 = inner_acc2 + b[i * m + j] * 5;
            inner_acc3 = (inner_acc3 << 1) ^ c[i * m + j];
            
            /* Store results creating anti-dependencies */
            a[i * m + j] = inner_acc1;
            b[i * m + j] = inner_acc2;
            c[i * m + j] = inner_acc3;
            
            /* Additional operations for register pressure */
            inner_acc1 += inner_acc2 & 0xF;
            inner_acc2 ^= inner_acc3 >> 2;
            inner_acc3 |= inner_acc1;
        }
        
        outer_acc += inner_acc1 + inner_acc2 + inner_acc3;
    }
    
    global_acc += outer_acc;
}

/* Test 6: SIMD-style operations (triggers vectorization + modulo scheduling) */
void test_simd_style(int *a, int *b, int *c, int n) {
    int i;
    int v1 = a[0], v2 = b[0], v3 = c[0];
    int w1 = a[1], w2 = b[1], w3 = c[1];
    
    /* Manual unrolling hint */
    #pragma GCC unroll 4
    for (i = 2; i < n - 2; i += 2) {
        /* Multiple parallel dependency chains */
        v1 = v1 * 7 + a[i];
        v2 = v2 * 11 + b[i];
        v3 = v3 * 13 + c[i];
        
        w1 = w1 * 3 + a[i + 1];
        w2 = w2 * 5 + b[i + 1];
        w3 = w3 * 17 + c[i + 1];
        
        /* Cross-lane operations */
        a[i] = v1 + w2;
        b[i] = v2 ^ w3;
        c[i] = v3 * w1;
        
        a[i + 1] = w1 - v2;
        b[i + 1] = w2 | v3;
        c[i + 1] = w3 + v1;
        
        /* Rotate values for next iteration */
        int t1 = v1; v1 = w1; w1 = t1;
        int t2 = v2; v2 = w2; w2 = t2;
        int t3 = v3; v3 = w3; w3 = t3;
    }
    
    global_acc += v1 + v2 + v3 + w1 + w2 + w3;
}

/* Architecture-specific tests */
#ifdef __powerpc__
/* PowerPC-specific operations that use FP/vector registers */
void test_powerpc_specific(double *a, double *b, int n) {
    int i;
    double acc1 = a[0], acc2 = b[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* FP operations that use PowerPC's multiple FP registers */
        acc1 = acc1 * 1.125 + a[i];  /* 1.125 = 9/8, not a simple power of 2 */
        acc2 = acc2 * 0.875 - b[i];  /* 0.875 = 7/8 */
        
        /* Cross-iteration dependencies */
        prod1 = prod1 * (acc1 + 0.001);
        prod2 = prod2 / (acc2 - 0.001);
        
        /* FMA-like pattern */
        a[i] = acc1 * prod1 + acc2;
        b[i] = acc2 * prod2 - acc1;
        
        /* Additional dependency chain */
        acc1 = acc1 + b[i] * 0.5;
        acc2 = acc2 - a[i] * 0.25;
    }
    
    global_acc += (long long)(acc1 + acc2 + prod1 + prod2);
}
#endif

#ifdef __ARM_FEATURE_SVE
/* ARM SVE-style loop with unknown bounds at compile time */
void test_sve_style(int *a, int *b, int n) {
    int i;
    int sum1 = a[0], sum2 = b[0];
    int diff1 = 0, diff2 = 0;
    
    /* Loop designed to trigger SVE vectorization */
    for (i = 1; i < n; i++) {
        /* Multiple independent chains */
        sum1 = sum1 + a[i] * 3;
        sum2 = sum2 - b[i] * 2;
        
        /* Distance-2 dependencies */
        diff1 = sum1 - diff2;
        diff2 = sum2 + diff1;
        
        /* Store with anti-dependencies */
        a[i] = sum1 ^ diff1;
        b[i] = sum2 | diff2;
        
        /* Additional operations */
        sum1 = (sum1 << 1) | 1;
        sum2 = (sum2 >> 1) ^ 0x55;
    }
    
    global_acc += sum1 + sum2 + diff1 + diff2;
}
#endif

/* Main test driver */
int main() {
    int i, iter;
    
    /* Allocate and initialize arrays */
    int *int_arr1 = malloc(SIZE * sizeof(int));
    int *int_arr2 = malloc(SIZE * sizeof(int));
    int *int_arr3 = malloc(SIZE * sizeof(int));
    int *int_arr4 = malloc(SIZE * sizeof(int));
    
    float *float_arr = malloc(SIZE * sizeof(float));
    double *double_arr1 = malloc(SIZE * sizeof(double));
    double *double_arr2 = malloc(SIZE * sizeof(double));
    double *double_arr3 = malloc(SIZE * sizeof(double));
    
    /* Initialize with patterned data */
    for (i = 0; i < SIZE; i++) {
        int_arr1[i] = i;
        int_arr2[i] = i * 2;
        int_arr3[i] = i * 3;
        int_arr4[i] = i * 5;
        
        float_arr[i] = i * 0.1f;
        double_arr1[i] = i * 0.01;
        double_arr2[i] = i * 0.02;
        double_arr3[i] = i * 0.03;
    }
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (iter = 0; iter < ITERS; iter++) {
        /* Call each test function */
        test_int_recurrence_multi_chain(int_arr1, int_arr2, int_arr3, int_arr4, SIZE);
        test_float_accumulate(double_arr1, double_arr2, double_arr3, SIZE);
        test_mixed_strided(int_arr1, float_arr, double_arr1, SIZE);
        test_pointer_chasing(int_arr2, SIZE);
        test_nested_loops(int_arr1, int_arr2, int_arr3, 16, 64);
        test_simd_style(int_arr1, int_arr2, int_arr3, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_specific(double_arr1, double_arr2, SIZE);
        #endif
        
        #ifdef __ARM_FEATURE_SVE
        test_sve_style(int_arr1, int_arr2, SIZE);
        #endif
        
        /* Modify data slightly each iteration */
        int_arr1[0] ^= iter;
        double_arr1[0] += iter * 0.001;
    }
    
    /* Output result to prevent optimization */
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    free(int_arr4);
    free(float_arr);
    free(double_arr1);
    free(double_arr2);
    free(double_arr3);
    
    return 0;
}

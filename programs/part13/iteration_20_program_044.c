/* test-modulo-sched.c
 * Comprehensive test to trigger GCC's modulo scheduler register move logic
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test-modulo-sched.c -o test-modulo-sched
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve test-modulo-sched.c -o test-modulo-sched
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_int_recurrence_chains(int *a, int *b, int *c, int *d, int n) {
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    /* Multiple independent dependency chains with different distances */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance 1 recurrence */
        x1 = x1 * 3 + a[i];
        
        /* Chain 2: distance 1 recurrence with different operation */
        x2 = x2 + b[i] * 7;
        
        /* Chain 3: distance 2 recurrence (uses y3 from i-2) */
        int temp = x3;
        x3 = y3 * 5 - c[i];
        y3 = temp;
        
        /* Chain 4: complex recurrence with multiple operations */
        x4 = (x4 << 2) | (d[i] & 0xFF);
        
        /* Additional operations to increase register pressure */
        a[i-1] = x1 + x2;
        b[i-1] = x3 ^ x4;
        c[i-1] = x1 * x3;
        d[i-1] = x2 + x4;
    }
    
    /* Store results to prevent optimization */
    a[n-1] = x1;
    b[n-1] = x2;
    c[n-1] = x3;
    d[n-1] = x4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    /* Multiple FP dependency chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: accumulation with multiplication */
        sum1 = sum1 * 1.01 + a[i];
        
        /* Chain 2: alternating add/multiply */
        if (i % 2) {
            sum2 = sum2 + b[i] * 2.5;
        } else {
            sum2 = sum2 * 0.99 - b[i];
        }
        
        /* Chain 3: product chain */
        prod1 = prod1 * (1.0 + c[i] * 0.001);
        
        /* Chain 4: mixed operations */
        sum3 = sum3 + (a[i] * b[i]) / (c[i] + 1.0);
        
        /* Chain 5: second product chain with dependency */
        prod2 = prod2 * (sum1 * 0.01);
        
        /* Additional operations for register pressure */
        a[i-1] = sum1 + sum2;
        b[i-1] = prod1 - prod2;
        c[i-1] = sum3 * 0.5;
    }
    
    /* Cross-store results */
    a[n-1] = sum1;
    b[n-1] = sum2;
    c[n-1] = sum3 + prod1 + prod2;
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer chains with carried dependencies */
        acc1 = acc1 + *ptr1 * 3;
        acc2 = acc2 ^ (*ptr2 + acc1);
        acc3 = acc3 | (*ptr3 * acc2);
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation for register pressure */
        *(ptr1 - 1) = acc1;
        *(ptr2 - 2) = acc2;
        *(ptr3 - 3) = acc3;
    }
    
    /* Store final accumulators */
    data[0] = acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with manual unrolling */
#pragma GCC unroll 4
void test_mixed_ops_unrolled(int *a, int *b, int *c, int n) {
    int x0 = a[0], x1 = a[1], x2 = a[2], x3 = a[3];
    int y0 = b[0], y1 = b[1], y2 = b[2], y3 = b[3];
    
    for (int i = 4; i < n; i += 4) {
        /* Unrolled operations with cross-iteration dependencies */
        x0 = x3 * 2 + a[i];
        y0 = y3 >> 1 ^ b[i];
        
        x1 = x0 + a[i+1] * 3;
        y1 = y0 | b[i+1];
        
        x2 = x1 - a[i+2];
        y2 = y1 & b[i+2];
        
        x3 = x2 * a[i+3];
        y3 = y2 + b[i+3];
        
        /* Store results with different patterns */
        c[i] = x0 + y0;
        c[i+1] = x1 * y1;
        c[i+2] = x2 ^ y2;
        c[i+3] = x3 | y3;
        
        /* Additional operations for register pressure */
        a[i-1] = x0 + x1 + x2 + x3;
        b[i-1] = y0 * y1 * y2 * y3;
    }
    
    /* Final stores */
    a[n-1] = x3;
    b[n-1] = y3;
}

/* Test 5: PowerPC specific - double precision with FMA-like patterns */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    double acc1 = a[0], acc2 = b[0], acc3 = c[0];
    double tmp1, tmp2, tmp3;
    
    for (int i = 1; i < n; i++) {
        /* Simulate FMA patterns that use multiple FP registers */
        tmp1 = acc1 * 3.14159 + a[i];
        tmp2 = acc2 * 2.71828 - b[i];
        tmp3 = acc3 * 1.41421 * c[i];
        
        /* Cross dependencies between chains */
        acc1 = tmp1 + tmp2 * 0.5;
        acc2 = tmp2 - tmp3 * 0.3;
        acc3 = tmp3 + tmp1 * 0.7;
        
        /* Additional operations for register pressure */
        a[i-1] = acc1 + acc2;
        b[i-1] = acc2 * acc3;
        c[i-1] = acc3 - acc1;
    }
    
    a[n-1] = acc1;
    b[n-1] = acc2;
    c[n-1] = acc3;
}
#endif

/* Test 6: Vector-style operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
void test_vector_style(int *a, int *b, int *c, int n) {
    int sum_a = 0, sum_b = 0, sum_c = 0;
    int prod_a = 1, prod_b = 1;
    
    for (int i = 0; i < n; i++) {
        /* Multiple accumulating chains */
        sum_a = sum_a + a[i];
        sum_b = sum_b + b[i];
        sum_c = sum_c + c[i];
        
        /* Product chains with dependencies */
        prod_a = prod_a * (a[i] + 1);
        prod_b = prod_b * (b[i] | 1);
        
        /* Cross-chain dependencies */
        a[i] = sum_a + prod_a;
        b[i] = sum_b ^ prod_b;
        c[i] = sum_c + (prod_a & prod_b);
        
        /* Additional operations */
        if (i > 0) {
            a[i-1] += b[i];
            b[i-1] ^= c[i];
            c[i-1] |= a[i];
        }
    }
}
#endif

/* Main test driver */
int main() {
    /* Allocate and initialize arrays */
    int *int_data1 = malloc(SIZE * sizeof(int));
    int *int_data2 = malloc(SIZE * sizeof(int));
    int *int_data3 = malloc(SIZE * sizeof(int));
    int *int_data4 = malloc(SIZE * sizeof(int));
    
    double *double_data1 = malloc(SIZE * sizeof(double));
    double *double_data2 = malloc(SIZE * sizeof(double));
    double *double_data3 = malloc(SIZE * sizeof(double));
    
    /* Initialize with pattern */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        int_data1[i] = rand() % 100;
        int_data2[i] = rand() % 100;
        int_data3[i] = rand() % 100;
        int_data4[i] = rand() % 100;
        
        double_data1[i] = (rand() % 100) / 10.0;
        double_data2[i] = (rand() % 100) / 10.0;
        double_data3[i] = (rand() % 100) / 10.0;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Test 1: Integer recurrence chains */
        test_int_recurrence_chains(int_data1, int_data2, int_data3, int_data4, SIZE);
        
        /* Test 2: Floating point accumulation */
        test_float_accumulation(double_data1, double_data2, double_data3, SIZE);
        
        /* Test 3: Pointer chasing */
        test_pointer_chasing(int_data1, SIZE, 4);
        
        /* Test 4: Mixed ops with unrolling */
        test_mixed_ops_unrolled(int_data2, int_data3, int_data4, SIZE);
        
        /* Architecture-specific tests */
        #ifdef __powerpc__
        test_powerpc_double(double_data1, double_data2, double_data3, SIZE);
        #endif
        
        #if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_style(int_data1, int_data2, int_data3, SIZE);
        #endif
        
        /* Accumulate results to prevent optimization */
        global_sum += int_data1[SIZE-1] + int_data2[SIZE-1] + 
                     int_data3[SIZE-1] + int_data4[SIZE-1] +
                     (long long)double_data1[SIZE-1] + 
                     (long long)double_data2[SIZE-1] +
                     (long long)double_data3[SIZE-1];
    }
    
    /* Output result */
    printf("Final result: %lld\n", global_sum);
    
    /* Cleanup */
    free(int_data1);
    free(int_data2);
    free(int_data3);
    free(int_data4);
    free(double_data1);
    free(double_data2);
    free(double_data3);
    
    return 0;
}

/* test-modulo-sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -S test-modulo-sched.c
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve -S test-modulo-sched.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent dependency chains */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        x1 = x1 * 3 + a[i] * 7;
        
        /* Chain 2: distance-1 with different operations */
        x2 = (x2 << 2) ^ b[i];
        
        /* Chain 3: distance-1 with multiply-accumulate pattern */
        x3 = x3 + a[i] * b[i];
        
        /* Chain 4: more complex recurrence */
        x4 = (x4 & 0xFFFF) * 5 + c[i] * 3;
        
        /* Additional parallel chains to increase register pressure */
        y1 = y1 * 2 + d[i];
        y2 = y2 + (y1 >> 3);
        y3 = y3 ^ (x1 & 0xFF);
        y4 = y4 * 9 - x2;
        
        /* Cross-chain dependencies to create anti-dependencies */
        a[i] = x1 + y1;
        b[i] = x2 ^ y2;
        c[i] = x3 * y3;
        d[i] = x4 - y4;
    }
    
    /* Use results to prevent optimization */
    global_sum += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + a[i] * 1.5;
        sum2 = sum2 + b[i] * 2.5;
        sum3 = sum3 + c[i] * 3.5;
        
        /* Parallel product chains */
        prod1 = prod1 * (a[i] + 0.1);
        prod2 = prod2 * (b[i] - 0.1);
        
        /* Cross dependencies */
        a[i] = sum1 * prod1;
        b[i] = sum2 * prod2;
        c[i] = sum3 + prod1 + prod2;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1, tmp2, tmp3;
    
    while (ptr3 < end) {
        /* Multiple pointer chains with carried dependencies */
        tmp1 = *ptr1;
        acc1 = acc1 * 3 + tmp1;
        
        tmp2 = *ptr2;
        acc2 = (acc2 << 1) ^ tmp2;
        
        tmp3 = *ptr3;
        acc3 = acc3 + tmp3 * 7;
        
        /* Anti-dependencies through pointer updates */
        *ptr1 = acc1;
        *ptr2 = acc2;
        *ptr3 = acc3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Additional operations to increase register pressure */
        acc1 = acc1 ^ acc2;
        acc2 = acc2 + acc3;
        acc3 = acc3 * 2 - acc1;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with manual unrolling */
void test_mixed_unrolled(int *a, int *b, int *c, int n) {
    int i;
    int r0 = a[0], r1 = b[0], r2 = c[0];
    int r3 = a[1], r4 = b[1], r5 = c[1];
    int r6 = a[2], r7 = b[2], r8 = c[2];
    
    /* Manually unrolled to increase operations per iteration */
    for (i = 3; i < n - 3; i += 3) {
        /* First set of operations */
        r0 = r0 * 5 + a[i];
        r1 = r1 + b[i] * 3;
        r2 = r2 ^ (c[i] << 1);
        
        /* Second set with different latencies */
        r3 = (r3 & r0) * 7 + a[i+1];
        r4 = r4 | (b[i+1] * 2);
        r5 = r5 - (c[i+1] >> 1);
        
        /* Third set creating more register pressure */
        r6 = r6 * 9 + r0;
        r7 = r7 + r1 * 4;
        r8 = r8 ^ r2;
        
        /* Store results creating anti-dependencies */
        a[i] = r0 + r3;
        b[i] = r1 + r4;
        c[i] = r2 + r5;
        a[i+1] = r3 + r6;
        b[i+1] = r4 + r7;
        c[i+1] = r5 + r8;
        a[i+2] = r6;
        b[i+2] = r7;
        c[i+2] = r8;
        
        /* Rotate registers to create longer dependency chains */
        int t = r0;
        r0 = r3; r3 = r6; r6 = t;
        t = r1;
        r1 = r4; r4 = r7; r7 = t;
        t = r2;
        r2 = r5; r5 = r8; r8 = t;
    }
    
    global_sum += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Test 5: PowerPC-specific double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double sum0 = a[0], sum1 = b[0];
    double prod0 = 1.0, prod1 = 1.0;
    double tmp0, tmp1;
    
    for (i = 1; i < n; i++) {
        /* Multiple double precision operations */
        tmp0 = a[i] * 1.414;
        tmp1 = b[i] * 2.718;
        
        sum0 = sum0 + tmp0;
        sum1 = sum1 + tmp1;
        
        prod0 = prod0 * (sum0 + 0.01);
        prod1 = prod1 * (sum1 - 0.01);
        
        /* Create anti-dependencies */
        a[i] = sum0 * prod0;
        b[i] = sum1 * prod1;
        
        /* More operations to increase register pressure */
        sum0 = sum0 * 0.99;
        sum1 = sum1 * 1.01;
        prod0 = prod0 / 1.1;
        prod1 = prod1 * 1.1;
    }
    
    global_sum += (long long)(sum0 + sum1 + prod0 + prod1);
}
#endif

/* Test 6: Compile-time unknown bounds to encourage vectorization */
void test_variable_bound(int *a, int *b, int n) {
    int i;
    int x = a[0], y = b[0];
    int z = x + y;
    
    /* Loop with runtime bound encourages modulo scheduling */
    for (i = 1; i < n; i++) {
        /* Multiple dependency chains */
        x = x * 3 + a[i];
        y = y * 5 + b[i];
        z = z * 7 + (x ^ y);
        
        /* Store with anti-dependency */
        a[i] = x + z;
        b[i] = y - z;
        
        /* Additional operations */
        x = x ^ (y >> 2);
        y = y + (z << 1);
        z = z * 2 - x;
    }
    
    global_sum += x + y + z;
}

/* Main driver that runs all tests repeatedly */
int main() {
    int i, j;
    
    /* Allocate and initialize arrays */
    int *data1 = (int*)malloc(SIZE * sizeof(int));
    int *data2 = (int*)malloc(SIZE * sizeof(int));
    int *data3 = (int*)malloc(SIZE * sizeof(int));
    int *data4 = (int*)malloc(SIZE * sizeof(int));
    double *fdata1 = (double*)malloc(SIZE * sizeof(double));
    double *fdata2 = (double*)malloc(SIZE * sizeof(double));
    double *fdata3 = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with pseudo-random pattern */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        data4[i] = rand() % 100;
        fdata1[i] = (double)(rand() % 100) / 10.0;
        fdata2[i] = (double)(rand() % 100) / 10.0;
        fdata3[i] = (double)(rand() % 100) / 10.0;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (j = 0; j < ITERATIONS; j++) {
        test_multi_recurrence_int(data1, data2, data3, data4, SIZE);
        test_float_recurrence(fdata1, fdata2, fdata3, SIZE);
        test_pointer_chasing(data1, SIZE, 4);
        test_mixed_unrolled(data2, data3, data4, SIZE);
        test_variable_bound(data1, data2, SIZE);
        
#ifdef __powerpc__
        test_powerpc_double(fdata1, fdata2, SIZE);
#endif
        
        /* Modify inputs slightly each iteration */
        data1[0] += j;
        data2[0] += j;
        fdata1[0] += j * 0.1;
    }
    
    /* Output result to ensure computation isn't optimized away */
    printf("Final result: %lld\n", global_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    
    return 0;
}

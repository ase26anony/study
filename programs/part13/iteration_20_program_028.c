/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        x1 = x1 * 3 + a[i] * 7;
        /* Chain 2: distance-1 with different ops */
        x2 = (x2 << 2) ^ b[i];
        /* Chain 3: distance-1 with multiply-accumulate pattern */
        x3 = x3 + c[i] * x3;
        /* Chain 4: distance-1 with bit operations */
        x4 = (x4 & 0xFFFF) | (d[i] << 16);
        
        /* Additional chains using previous iteration values */
        y1 = y1 + x1 * 5;
        y2 = y2 ^ (x2 >> 3);
        y3 = y3 * 2 + x3;
        y4 = y4 | (x4 & 0xFF);
        
        /* Cross-chain dependencies to increase pressure */
        a[i] = x1 + y1;
        b[i] = x2 - y2;
        c[i] = x3 * y3;
        d[i] = x4 ^ y4;
    }
    
    global_acc += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0], acc2 = b[0], acc3 = c[0];
    double tmp1 = 1.0, tmp2 = 2.0, tmp3 = 3.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        acc1 = acc1 * 1.01 + a[i] * tmp1;
        acc2 = acc2 * 0.99 + b[i] * tmp2;
        acc3 = acc3 * 1.02 + c[i] * tmp3;
        
        /* Cross dependencies */
        tmp1 = tmp1 + acc1 * 0.1;
        tmp2 = tmp2 - acc2 * 0.2;
        tmp3 = tmp3 * acc3 * 0.3;
        
        /* Additional operations to increase register pressure */
        a[i] = acc1 + tmp1;
        b[i] = acc2 - tmp2;
        c[i] = acc3 * tmp3;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int acc1 = *ptr1, acc2 = *ptr2, acc3 = *ptr3;
    
    while (ptr3 < end) {
        /* Multiple pointer-chasing chains */
        sum1 = sum1 + acc1 * 3;
        acc1 = *ptr1;
        ptr1 += stride;
        
        sum2 = sum2 ^ (acc2 << 1);
        acc2 = *ptr2;
        ptr2 += stride;
        
        sum3 = sum3 | (acc3 & 0xFF);
        acc3 = *ptr3;
        ptr3 += stride;
        
        /* Cross dependencies */
        sum1 = sum1 ^ sum2;
        sum2 = sum2 + sum3;
        sum3 = sum3 * sum1;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_distance(int *a, int *b, int *c, int n) {
    int i;
    int x1 = a[0], x2 = a[1];
    int y1 = b[0], y2 = b[1];
    int z1 = c[0], z2 = c[1];
    
    for (i = 2; i < n; i++) {
        /* Distance-1 dependencies */
        int t1 = x1 * 7 + a[i];
        int t2 = y1 ^ b[i] * 3;
        int t3 = z1 + c[i] << 1;
        
        /* Distance-2 dependencies */
        int u1 = x2 * 5 + t1;
        int u2 = y2 ^ t2 >> 2;
        int u3 = z2 + t3 & 0xFF;
        
        /* Update state for next iteration */
        x2 = x1; x1 = t1;
        y2 = y1; y1 = t2;
        z2 = z1; z1 = t3;
        
        /* Complex operations mixing all values */
        a[i] = u1 + u2 + u3;
        b[i] = (u1 * u2) ^ u3;
        c[i] = u1 | (u2 & u3);
    }
    
    global_acc += x1 + x2 + y1 + y2 + z1 + z2;
}

/* Test 5: Nested loops with inner loop being modulo-scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    int acc = 0;
    
    for (i = 0; i < n; i++) {
        int base = a[i];
        int sum1 = base, sum2 = base * 2, sum3 = base * 3;
        
        /* Inner loop with carried dependencies */
        for (j = 0; j < m; j++) {
            /* Multiple dependency chains */
            sum1 = sum1 * 2 + b[j];
            sum2 = sum2 + c[j] * sum1;
            sum3 = sum3 ^ (sum2 << (j & 3));
            
            /* Cross-iteration dependencies */
            b[j] = sum1 + sum2;
            c[j] = sum2 * sum3;
        }
        
        acc += sum1 + sum2 + sum3;
        a[i] = acc;
    }
    
    global_acc += acc;
}

/* Test 6: PowerPC-specific patterns using double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double acc1 = a[0], acc2 = b[0];
    double tmp1 = 1.0, tmp2 = 2.0;
    
    for (i = 1; i < n; i++) {
        /* Double precision operations that use FP registers */
        acc1 = acc1 * 1.5 + a[i] * tmp1;
        acc2 = acc2 * 2.0 - b[i] * tmp2;
        
        /* Cross dependencies */
        tmp1 = tmp1 + acc1 * 0.25;
        tmp2 = tmp2 - acc2 * 0.5;
        
        /* Additional operations */
        a[i] = acc1 * tmp1;
        b[i] = acc2 / tmp2;
    }
    
    global_acc += (long long)(acc1 + acc2 + tmp1 + tmp2);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
void test_unrolled_recurrence(int *a, int *b, int n) {
    int i;
    int x1 = a[0], x2 = b[0];
    int y1 = a[1], y2 = b[1];
    
    /* Process 4 elements per iteration */
    for (i = 2; i < n - 4; i += 4) {
        /* Unrolled operations with dependencies */
        x1 = x1 * 3 + a[i];
        y1 = y1 + x1 * 5;
        a[i] = x1 + y1;
        
        x2 = x2 ^ (b[i] << 1);
        y2 = y2 | x2;
        b[i] = x2 - y2;
        
        x1 = x1 * 2 + a[i+1];
        y1 = y1 ^ x1;
        a[i+1] = x1 * y1;
        
        x2 = (x2 + b[i+1]) & 0xFF;
        y2 = y2 * x2;
        b[i+1] = x2 | y2;
        
        x1 = x1 + a[i+2] * 7;
        y1 = y1 - x1;
        a[i+2] = x1 & y1;
        
        x2 = x2 * 3 ^ b[i+2];
        y2 = y2 + x2;
        b[i+2] = x2 ^ y2;
        
        x1 = (x1 << 2) + a[i+3];
        y1 = y1 * x1;
        a[i+3] = x1 - y1;
        
        x2 = x2 / 2 + b[i+3];
        y2 = y2 ^ x2;
        b[i+3] = x2 + y2;
    }
    
    global_acc += x1 + x2 + y1 + y2;
}

/* Main test driver */
int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *data4 = malloc(SIZE * sizeof(int));
    double *fdata1 = malloc(SIZE * sizeof(double));
    double *fdata2 = malloc(SIZE * sizeof(double));
    double *fdata3 = malloc(SIZE * sizeof(double));
    
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
    
    start = clock();
    
    /* Run multiple test patterns to increase chance of triggering modulo scheduling */
    for (i = 0; i < ITERS; i++) {
        test_multi_recurrence_int(data1, data2, data3, data4, SIZE);
        test_float_recurrence(fdata1, fdata2, fdata3, SIZE);
        test_pointer_chasing(data1, SIZE, 3);
        test_mixed_distance(data1, data2, data3, SIZE);
        test_nested_loops(data1, data2, data3, 100, 10);
        test_unrolled_recurrence(data1, data2, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double(fdata1, fdata2, SIZE);
        #endif
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Modulo scheduling test completed.\n");
    printf("Global accumulator: %lld\n", global_acc);
    printf("Time used: %f seconds\n", cpu_time_used);
    printf("Iterations: %d\n", ITERS);
    
    /* Verify results aren't optimized away */
    if (global_acc == 0) {
        printf("WARNING: All computations may have been optimized away!\n");
    }
    
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    
    return 0;
}

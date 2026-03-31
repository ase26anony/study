/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

volatile int global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x = a[0];
    int y = b[0];
    int z = c[0];
    int w = d[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains */
        x = x * 3 + a[i];          /* Chain 1: distance 1 */
        y = y + x * b[i];          /* Chain 2: depends on chain 1 */
        z = z * 5 - y + c[i];      /* Chain 3: depends on chain 2 */
        w = (w << 2) ^ z + d[i];   /* Chain 4: depends on chain 3 */
        
        /* Additional operations to increase register pressure */
        a[i] = x ^ y;
        b[i] = y & z;
        c[i] = z | w;
        d[i] = w + x;
    }
    
    global_sum += x + y + z + w;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *f1, float *f2, float *f3, int n) {
    float acc1 = f1[0];
    float acc2 = f2[0];
    float acc3 = f3[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP recurrence chains */
        acc1 = acc1 * 1.5f + f1[i];
        acc2 = acc2 + acc1 * f2[i];
        acc3 = acc3 * 0.9f - acc2 + f3[i];
        
        /* Cross-chain dependencies */
        f1[i] = acc1 + acc3;
        f2[i] = acc2 * acc1;
        f3[i] = acc3 - acc2;
        
        /* Additional operations to prevent optimization */
        if (i % 8 == 0) {
            acc1 = acc1 * 0.99f;
            acc3 = acc3 + 0.01f;
        }
    }
    
    global_sum += (int)(acc1 + acc2 + acc3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer chains with dependencies */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 + sum1 + *ptr2;
        sum3 = sum3 ^ sum2 + *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation to increase pressure */
        if (i % 4 == 0) {
            sum1 = sum1 & 0xFF;
            sum3 = sum3 | 0x55;
        }
    }
    
    global_sum += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops(int *arr1, int *arr2, int *arr3, int n) {
    int v1 = arr1[0];
    int v2 = arr2[0];
    int v3 = arr3[0];
    int v4 = arr1[1];
    int v5 = arr2[1];
    
    for (int i = 2; i < n; i++) {
        /* Complex web of dependencies */
        int t1 = v1 * v2 + arr1[i];
        int t2 = v3 + v4 * arr2[i];
        int t3 = v5 ^ t1 + arr3[i];
        int t4 = t2 - t3 * v1;
        int t5 = t1 & t4 + v2;
        
        /* Update recurrence variables with different distances */
        v1 = t3;          /* Distance 1 */
        v2 = t4 + v3;     /* Distance 1 with extra dependency */
        v3 = t5;          /* Distance 1 */
        v4 = v1 * 2;      /* Distance 2 (through v1) */
        v5 = v2 + v4;     /* Distance 2 */
        
        /* Store results to prevent elimination */
        arr1[i-1] = t1;
        arr2[i-1] = t2;
        arr3[i-1] = t3;
    }
    
    global_sum += v1 + v2 + v3 + v4 + v5;
}

/* Test 5: Double precision floating point for PowerPC targeting */
void test_double_accumulate(double *d1, double *d2, double *d3, int n) {
    double acc1 = d1[0];
    double acc2 = d2[0];
    double acc3 = d3[0];
    double acc4 = d1[1];
    double acc5 = d2[1];
    
    for (int i = 2; i < n; i++) {
        /* Multiple double precision recurrence chains */
        acc1 = acc1 * 1.01 + d1[i];
        acc2 = acc2 + acc1 * d2[i];
        acc3 = acc3 * 0.99 - acc2 + d3[i];
        acc4 = acc4 + acc3 * acc1;
        acc5 = acc5 * 1.1 - acc4;
        
        /* Cross dependencies */
        d1[i-1] = acc1 + acc5;
        d2[i-1] = acc2 * acc3;
        d3[i-1] = acc4 - acc5;
        
        /* Periodic operations to break patterns */
        if (i % 16 == 0) {
            acc1 = acc1 * 0.5;
            acc3 = acc3 + 0.25;
        }
    }
    
    global_sum += (int)(acc1 + acc2 + acc3 + acc4 + acc5);
}

/* Test 6: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_unrolled_loop(int *a, int *b, int *c, int n) {
    int x = a[0];
    int y = b[0];
    int z = c[0];
    
    for (int i = 1; i < n - 3; i += 4) {
        /* Manually unrolled operations with carried dependencies */
        x = x * 3 + a[i];
        y = y + x * b[i];
        z = z ^ y + c[i];
        
        x = x * 2 + a[i+1];
        y = y - x * b[i+1];
        z = z | y + c[i+1];
        
        x = x + 5 + a[i+2];
        y = y * x + b[i+2];
        z = z & y + c[i+2];
        
        x = x * 7 + a[i+3];
        y = y / (x + 1) + b[i+3];
        z = z ^ x + c[i+3];
        
        /* Store results */
        a[i] = x;
        b[i] = y;
        c[i] = z;
    }
    
    global_sum += x + y + z;
}

/* Test 7: Compile-time unknown bounds to trigger vectorization */
void test_variable_bound(int *data, int n) {
    int sum1 = data[0];
    int sum2 = data[1];
    int sum3 = 0;
    
    /* Loop with variable bound - harder to optimize away */
    for (int i = 2; i < n; i++) {
        sum1 = sum1 * 2 + data[i];
        sum2 = sum2 + sum1 * data[i-1];
        sum3 = sum3 ^ sum2 + data[i-2];
        
        /* Additional operations */
        data[i-1] = sum1 & 0xFF;
        data[i-2] = sum2 | 0xAA;
        
        /* Conditional that doesn't break the loop */
        if (sum3 > 1000) {
            sum3 = sum3 % 1000;
        }
    }
    
    global_sum += sum1 + sum2 + sum3;
}

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, int *c, int *d, float *f1, float *f2, float *f3, 
                 double *d1, double *d2, double *d3, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 5) % 100;
        d[i] = (i * 7) % 100;
        f1[i] = (i % 100) * 0.1f;
        f2[i] = (i % 100) * 0.2f;
        f3[i] = (i % 100) * 0.3f;
        d1[i] = (i % 100) * 0.01;
        d2[i] = (i % 100) * 0.02;
        d3[i] = (i % 100) * 0.03;
    }
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int *a = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int *b = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int *c = (int*)aligned_alloc(64, SIZE * sizeof(int));
    int *d = (int*)aligned_alloc(64, SIZE * sizeof(int));
    
    float *f1 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *f2 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    float *f3 = (float*)aligned_alloc(64, SIZE * sizeof(float));
    
    double *d1 = (double*)aligned_alloc(64, SIZE * sizeof(double));
    double *d2 = (double*)aligned_alloc(64, SIZE * sizeof(double));
    double *d3 = (double*)aligned_alloc(64, SIZE * sizeof(double));
    
    /* Initialize data */
    init_arrays(a, b, c, d, f1, f2, f3, d1, d2, d3, SIZE);
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Rotate through different tests to exercise various patterns */
        switch (iter % 7) {
            case 0:
                test_multi_recurrence_int(a, b, c, d, SIZE);
                break;
            case 1:
                test_float_accumulate(f1, f2, f3, SIZE);
                break;
            case 2:
                test_pointer_chasing(a, 8, SIZE/8);
                break;
            case 3:
                test_mixed_ops(a, b, c, SIZE);
                break;
            case 4:
                test_double_accumulate(d1, d2, d3, SIZE);
                break;
            case 5:
                test_unrolled_loop(a, b, c, SIZE);
                break;
            case 6:
                test_variable_bound(a, SIZE);
                break;
        }
        
        /* Modify inputs slightly each iteration */
        if (iter % 100 == 0) {
            a[0] = iter % 256;
            f1[0] = (iter % 256) * 0.1f;
            d1[0] = (iter % 256) * 0.01;
        }
    }
    
    printf("Final global sum: %d\n", global_sum);
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(f1); free(f2); free(f3);
    free(d1); free(d2); free(d3);
    
    return 0;
}

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
    /* Multiple independent dependency chains */
    int x = a[0], y = b[0], z = c[0], w = d[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: simple recurrence */
        x = x * 3 + a[i];
        
        /* Chain 2: recurrence with different distance */
        y = y + b[i] * 7 - y/2;
        
        /* Chain 3: cross-iteration dependency */
        z = z * 5 + c[i-1] * 2;
        
        /* Chain 4: more complex recurrence */
        w = (w << 2) | (d[i] & 0xFF);
        
        /* Additional operations to increase register pressure */
        a[i] = x ^ y;
        b[i] = y + z;
        c[i] = z * w;
        d[i] = w - x;
    }
    
    global_acc += x + y + z + w;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *f1, float *f2, double *d1, double *d2, int n) {
    int i;
    float f_acc1 = f1[0], f_acc2 = f2[0];
    double d_acc1 = d1[0], d_acc2 = d2[0];
    
    for (i = 1; i < n; i++) {
        /* Float recurrence chains */
        f_acc1 = f_acc1 * 1.5f + f1[i];
        f_acc2 = f_acc2 * 0.9f - f2[i-1];
        
        /* Double recurrence chains */
        d_acc1 = d_acc1 * 2.0 + d1[i];
        d_acc2 = d_acc2 * 0.5 + d2[i-1];
        
        /* Cross-type operations to increase pressure */
        f1[i] = f_acc1 + (float)d_acc1;
        f2[i] = f_acc2 * (float)d_acc2;
        d1[i] = d_acc1 - (double)f_acc1;
        d2[i] = d_acc2 / (double)(f_acc2 + 1.0f);
    }
    
    global_acc += (long long)(f_acc1 + f_acc2 + d_acc1 + d_acc2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int i;
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2*stride;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (i = 0; i < n; i++) {
        /* Multiple pointer chains with dependencies */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 + *ptr2 * sum1;
        sum3 = sum3 * 3 - *ptr3 + sum2;
        
        /* Update pointers with stride */
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Additional computation to use results */
        *ptr1 = sum1 & 0xFF;
        *ptr2 = sum2 | 0xAA;
        *ptr3 = sum3 ^ 0x55;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with manual unrolling */
void test_mixed_ops_unrolled(short *s1, short *s2, int *i1, int *i2, int n) {
    int i;
    int acc1 = i1[0], acc2 = i2[0];
    short s_acc1 = s1[0], s_acc2 = s2[0];
    
    /* Manual unrolling to increase operations per iteration */
    for (i = 1; i < n - 3; i += 4) {
        /* Unrolled iteration 1 */
        acc1 = acc1 * 7 + i1[i];
        acc2 = acc2 + i2[i] * acc1;
        s_acc1 = s_acc1 + s1[i] * 3;
        s_acc2 = s_acc2 - s2[i] + s_acc1;
        
        /* Unrolled iteration 2 */
        acc1 = acc1 ^ i1[i+1];
        acc2 = acc2 | i2[i+1];
        s_acc1 = s_acc1 & s1[i+1];
        s_acc2 = s_acc2 << (s2[i+1] & 3);
        
        /* Unrolled iteration 3 */
        acc1 = acc1 + i1[i+2] * 11;
        acc2 = acc2 - i2[i+2] / 3;
        s_acc1 = s_acc1 | s1[i+2];
        s_acc2 = s_acc2 >> (s2[i+2] & 1);
        
        /* Unrolled iteration 4 */
        acc1 = (acc1 << 2) + i1[i+3];
        acc2 = (acc2 >> 1) ^ i2[i+3];
        s_acc1 = s_acc1 * s1[i+3];
        s_acc2 = s_acc2 + s2[i+3] - s_acc1;
        
        /* Store results to create anti-dependencies */
        i1[i] = acc1;
        i2[i] = acc2;
        s1[i] = s_acc1;
        s2[i] = s_acc2;
    }
    
    global_acc += acc1 + acc2 + s_acc1 + s_acc2;
}

/* Test 5: PowerPC-specific double operations for FP register pressure */
#ifdef __powerpc__
void test_powerpc_double_ops(double *d1, double *d2, double *d3, double *d4, int n) {
    int i;
    double acc1 = d1[0], acc2 = d2[0], acc3 = d3[0], acc4 = d4[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple double-precision recurrence chains */
        acc1 = acc1 * 1.1 + d1[i];
        acc2 = acc2 * 0.9 - d2[i-1];
        acc3 = acc3 * 2.0 + d3[i] * acc1;
        acc4 = acc4 / 1.5 + d4[i-1] * acc2;
        
        /* Cross-dependencies between chains */
        d1[i] = acc1 + acc3;
        d2[i] = acc2 - acc4;
        d3[i] = acc3 * acc1;
        d4[i] = acc4 / (acc2 + 1.0);
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3 + acc4);
}
#endif

/* Test 6: Vector-style operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
void test_vector_style_ops(int *data, int *mask, int n) {
    int i;
    int sum1 = data[0], sum2 = data[1];
    int prod1 = 1, prod2 = 1;
    
    for (i = 2; i < n; i++) {
        /* Strided access patterns that encourage vectorization */
        int idx1 = i;
        int idx2 = i * 2 % n;
        int idx3 = i * 3 % n;
        
        /* Multiple dependency chains */
        sum1 = sum1 + data[idx1] * mask[i];
        sum2 = sum2 - data[idx2] + sum1;
        prod1 = prod1 * (data[idx3] | 1);
        prod2 = prod2 & (mask[i] ^ 0xFFFF);
        
        /* Anti-dependencies through array writes */
        data[idx1] = sum1;
        data[idx2] = sum2;
        mask[i] = prod1 + prod2;
    }
    
    global_acc += sum1 + sum2 + prod1 + prod2;
}
#endif

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, int *c, int *d, 
                 float *f1, float *f2, double *d1, double *d2,
                 short *s1, short *s2, int *i1, int *i2) {
    int i;
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 5;
        f1[i] = i * 0.1f;
        f2[i] = i * 0.2f;
        d1[i] = i * 0.3;
        d2[i] = i * 0.4;
        s1[i] = i & 0xFFFF;
        s2[i] = (i * 3) & 0xFFFF;
        i1[i] = i * 7;
        i2[i] = i * 11;
    }
}

int main() {
    /* Allocate and initialize test arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    float *f1 = malloc(SIZE * sizeof(float));
    float *f2 = malloc(SIZE * sizeof(float));
    double *d1 = malloc(SIZE * sizeof(double));
    double *d2 = malloc(SIZE * sizeof(double));
    short *s1 = malloc(SIZE * sizeof(short));
    short *s2 = malloc(SIZE * sizeof(short));
    int *i1 = malloc(SIZE * sizeof(int));
    int *i2 = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c || !d || !f1 || !f2 || !d1 || !d2 || !s1 || !s2 || !i1 || !i2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, d, f1, f2, d1, d2, s1, s2, i1, i2);
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multi_recurrence_int(a, b, c, d, SIZE);
        test_float_accumulate(f1, f2, d1, d2, SIZE);
        test_pointer_chasing(a, 4, SIZE/4);
        test_mixed_ops_unrolled(s1, s2, i1, i2, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double_ops(d1, d2, d1, d2, SIZE);
        #endif
        
        #if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_style_ops(a, b, SIZE);
        #endif
        
        /* Modify array contents slightly each iteration */
        a[iter % SIZE] = iter;
        b[iter % SIZE] = iter * 2;
    }
    
    printf("Final accumulator value: %lld\n", global_acc);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(f1); free(f2); free(d1); free(d2);
    free(s1); free(s2); free(i1); free(i2);
    
    return 0;
}

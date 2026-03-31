/* test_modulo_sched.c - Test program to cover modulo scheduling register move logic */
/* Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched */
/* For PowerPC: add -mtune=powerpc -mcpu=power8 */
/* For ARM SVE: add -march=armv8-a+sve -ftree-vectorize */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x1 = a[0], x2 = a[1], x3 = a[2], x4 = a[3];
    int y1 = b[0], y2 = b[1];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: distance-1 recurrence */
        x1 = x1 * 3 + b[i] * 7;
        a[i] = x1;
        
        /* Chain 2: distance-1 recurrence with different ops */
        x2 = (x2 << 2) ^ (b[i] + i);
        c[i] = x2;
        
        /* Chain 3: distance-1 recurrence with multiply */
        x3 = x3 * 5 - d[i] * 3;
        
        /* Chain 4: distance-1 recurrence with bit ops */
        x4 = (x4 & 0xFFFF) | (d[i] << 16);
        
        /* Chain 5: distance-2 recurrence (uses values from i-2) */
        if (i >= 2) {
            y1 = y1 + a[i-2] * 11;
            y2 = y2 ^ (c[i-2] * 13);
        }
        
        /* Additional operations to increase register pressure */
        b[i] = b[i] + (x1 & 0xFF) - (x2 >> 4);
        d[i] = d[i] * 2 + (x3 & 0xFFF);
    }
    
    /* Accumulate results to prevent optimization */
    global_sum += x1 + x2 + x3 + x4 + y1 + y2;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *f1, float *f2, float *f3, float *f4, int n) {
    int i;
    float acc1 = f1[0], acc2 = f2[0], acc3 = f3[0];
    float tmp1, tmp2, tmp3;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP recurrence chains */
        acc1 = acc1 * 1.5f + f1[i] * 2.3f;
        acc2 = acc2 * 0.9f - f2[i] * 1.7f;
        acc3 = acc3 + f3[i] * acc1;
        
        /* Cross-chain dependencies */
        tmp1 = acc1 * acc2;
        tmp2 = acc2 + acc3;
        tmp3 = acc1 - acc3;
        
        /* Store results creating anti-dependencies */
        f1[i] = tmp1;
        f2[i] = tmp2;
        f3[i] = tmp3;
        
        /* Additional computation for register pressure */
        f4[i] = f4[i-1] + tmp1 * tmp2 - tmp3;
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr = data;
    int *end = data + n;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Unrolled pointer chasing to increase register pressure */
    while (ptr < end - 3) {
        /* Multiple accumulators with carried dependencies */
        sum1 = sum1 + *ptr;
        sum2 = sum2 ^ *(ptr + 1);
        sum3 = sum3 * 3 + *(ptr + 2);
        
        /* Anti-dependency: read then write */
        *ptr = sum1 & 0xFF;
        *(ptr + 1) = sum2 | 0x100;
        *(ptr + 2) = sum3 ^ 0x200;
        
        ptr += stride;
        
        /* Additional computation to prevent simplification */
        sum1 = sum1 ^ (sum2 << 1);
        sum2 = sum2 + (sum3 >> 2);
        sum3 = sum3 * 5 - sum1;
    }
    
    global_sum += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with array accumulation */
void test_mixed_ops(short *s1, short *s2, int *i1, int *i2, int n) {
    int i;
    short acc_s1 = s1[0], acc_s2 = s2[0];
    int acc_i1 = i1[0], acc_i2 = i2[0];
    
    for (i = 1; i < n; i++) {
        /* Mixed-width operations requiring moves */
        acc_s1 = (acc_s1 * s1[i]) >> 3;
        acc_s2 = acc_s2 + (s2[i] & acc_s1);
        
        /* Integer recurrences */
        acc_i1 = acc_i1 * 7 + i1[i] * 11;
        acc_i2 = (acc_i2 << 1) | (i2[i] & 1);
        
        /* Store creating anti-dependencies */
        s1[i] = acc_s1;
        s2[i] = acc_s2;
        i1[i] = acc_i1;
        i2[i] = acc_i2;
        
        /* Cross-type operations */
        acc_s1 = acc_s1 + (acc_i1 & 0xFFFF);
        acc_i1 = acc_i1 ^ (acc_s2 << 16);
    }
    
    global_sum += acc_s1 + acc_s2 + acc_i1 + acc_i2;
}

/* Test 5: Nested loops with inner loop being modulo scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    int outer_acc = 0;
    
    for (i = 0; i < n; i++) {
        int inner_acc1 = a[i];
        int inner_acc2 = b[i];
        
        /* Inner loop with carried dependencies */
        for (j = 1; j < m; j++) {
            /* Multiple recurrence chains */
            inner_acc1 = inner_acc1 * 3 + c[j] * 7;
            inner_acc2 = (inner_acc2 << 1) ^ (inner_acc1 & 0xFF);
            
            /* Store with anti-dependency */
            a[i * m + j] = inner_acc1;
            b[i * m + j] = inner_acc2;
            
            /* Additional computation */
            c[j] = c[j-1] + inner_acc1 - inner_acc2;
        }
        
        outer_acc += inner_acc1 + inner_acc2;
    }
    
    global_sum += outer_acc;
}

/* Test 6: PowerPC-specific double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *d1, double *d2, double *d3, int n) {
    int i;
    double acc1 = d1[0], acc2 = d2[0], acc3 = d3[0];
    
    for (i = 1; i < n; i++) {
        /* Double precision recurrences */
        acc1 = acc1 * 1.5 + d1[i] * 2.5;
        acc2 = acc2 * 0.75 - d2[i] * 1.25;
        acc3 = acc3 + d3[i] * acc1;
        
        /* Cross dependencies */
        d1[i] = acc1 * acc2;
        d2[i] = acc2 + acc3;
        d3[i] = acc1 - acc3;
        
        /* More operations for register pressure */
        acc1 = acc1 + (d1[i] * 0.1);
        acc2 = acc2 - (d2[i] * 0.2);
        acc3 = acc3 * (1.0 + d3[i] * 0.01);
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3);
}
#endif

/* Test 7: Manual unrolling to increase register pressure */
void test_manual_unroll(int *a, int *b, int *c, int n) {
    int i;
    int acc1 = a[0], acc2 = b[0], acc3 = c[0];
    int acc4 = a[1], acc5 = b[1], acc6 = c[1];
    
    /* Manually unrolled loop */
    for (i = 2; i < n - 4; i += 4) {
        /* Unrolled iteration 1 */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 ^ (b[i] * 5);
        acc3 = acc3 + (acc1 & acc2);
        a[i] = acc3;
        
        /* Unrolled iteration 2 */
        acc4 = acc4 * 7 + a[i+1];
        acc5 = acc5 | (b[i+1] << 2);
        acc6 = acc6 - (acc4 ^ acc5);
        b[i+1] = acc6;
        
        /* Unrolled iteration 3 */
        acc1 = acc1 + acc4 * 11;
        acc2 = acc2 ^ acc6;
        acc3 = acc3 * 13 + c[i+2];
        c[i+2] = acc3;
        
        /* Unrolled iteration 4 */
        acc4 = acc4 - acc1;
        acc5 = acc5 * 17 + acc2;
        acc6 = acc6 ^ acc3;
        a[i+3] = acc4 + acc5 + acc6;
        
        /* Rotate registers to create move requirements */
        int tmp = acc1;
        acc1 = acc4;
        acc4 = acc2;
        acc2 = acc5;
        acc5 = acc3;
        acc3 = acc6;
        acc6 = tmp;
    }
    
    global_sum += acc1 + acc2 + acc3 + acc4 + acc5 + acc6;
}

int main() {
    int i, j;
    
    /* Allocate and initialize arrays */
    int *a1 = malloc(SIZE * sizeof(int));
    int *a2 = malloc(SIZE * sizeof(int));
    int *a3 = malloc(SIZE * sizeof(int));
    int *a4 = malloc(SIZE * sizeof(int));
    short *s1 = malloc(SIZE * sizeof(short));
    short *s2 = malloc(SIZE * sizeof(short));
    float *f1 = malloc(SIZE * sizeof(float));
    float *f2 = malloc(SIZE * sizeof(float));
    float *f3 = malloc(SIZE * sizeof(float));
    float *f4 = malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern data */
    for (i = 0; i < SIZE; i++) {
        a1[i] = i;
        a2[i] = i * 2;
        a3[i] = i * 3;
        a4[i] = i * 5;
        s1[i] = i & 0xFFFF;
        s2[i] = (i * 7) & 0xFFFF;
        f1[i] = i * 1.1f;
        f2[i] = i * 2.2f;
        f3[i] = i * 3.3f;
        f4[i] = i * 4.4f;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (j = 0; j < ITERS; j++) {
        test_multi_recurrence_int(a1, a2, a3, a4, SIZE);
        test_float_accumulate(f1, f2, f3, f4, SIZE);
        test_pointer_chasing(a1, SIZE, 3);
        test_mixed_ops(s1, s2, a1, a2, SIZE);
        test_nested_loops(a1, a2, a3, 16, 64);
        test_manual_unroll(a1, a2, a3, SIZE);
        
        #ifdef __powerpc__
        {
            double *d1 = malloc(SIZE * sizeof(double));
            double *d2 = malloc(SIZE * sizeof(double));
            double *d3 = malloc(SIZE * sizeof(double));
            for (i = 0; i < SIZE; i++) {
                d1[i] = i * 1.5;
                d2[i] = i * 2.5;
                d3[i] = i * 3.5;
            }
            test_powerpc_double(d1, d2, d3, SIZE);
            free(d1); free(d2); free(d3);
        }
        #endif
        
        /* Modify inputs slightly each iteration */
        a1[0] += j;
        f1[0] += j * 0.1f;
    }
    
    /* Output result to ensure computation isn't optimized away */
    printf("Final result: %lld\n", global_sum);
    
    /* Cleanup */
    free(a1); free(a2); free(a3); free(a4);
    free(s1); free(s2);
    free(f1); free(f2); free(f3); free(f4);
    
    return 0;
}

/* Test program for GCC modulo scheduler register move coverage */
/* Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Volatile to prevent optimization */
volatile long g_result = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_int_recurrence(int *a, int *b, int *c, int *d, int n) {
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    /* Multiple independent dependency chains with different distances */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance 1 recurrence */
        x1 = x1 * 3 + a[i] - y1;
        y1 = x1 + b[i] * 7;
        
        /* Chain 2: distance 1 recurrence with different ops */
        x2 = (x2 << 2) | (b[i] & 0xFF);
        y2 = y2 ^ (x2 * 5);
        
        /* Chain 3: distance 1 recurrence */
        x3 = x3 + c[i] * 11;
        y3 = y3 - x3 / 3;
        
        /* Chain 4: distance 1 recurrence */
        x4 = (x4 & 0xFFFF) * d[i];
        y4 = y4 | (x4 << 3);
        
        /* Additional operations to increase register pressure */
        a[i-1] = x1 + y1;
        b[i-1] = x2 ^ y2;
        c[i-1] = x3 * y3;
        d[i-1] = x4 | y4;
    }
    
    g_result += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating point accumulation with mixed operations */
void test_float_accumulate(float *f1, float *f2, double *d1, double *d2, int n) {
    float acc1 = f1[0], acc2 = f2[0];
    double dacc1 = d1[0], dacc2 = d2[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        acc1 = acc1 * 1.5f + f1[i];
        acc2 = acc2 - f2[i] * 0.7f;
        
        dacc1 = dacc1 + d1[i] * 2.3;
        dacc2 = dacc2 * 0.9 + d2[i];
        
        /* Cross-chain operations to create register pressure */
        f1[i-1] = acc1 + (float)dacc1;
        f2[i-1] = acc2 * (float)dacc2;
        
        d1[i-1] = dacc1 - (double)acc1;
        d2[i-1] = dacc2 + (double)acc2;
    }
    
    g_result += (long)(acc1 + acc2 + dacc1 + dacc2);
}

/* Test 3: Pointer chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *ptr4 = data + 3 * stride;
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    
    /* Loop with pointer arithmetic and carried dependencies */
    for (int i = 0; i < n - 4 * stride; i++) {
        /* Each chain has carried dependency through tmp variables */
        tmp1 = tmp1 * 2 + *ptr1;
        sum1 += tmp1;
        
        tmp2 = (tmp2 >> 1) ^ *ptr2;
        sum2 ^= tmp2;
        
        tmp3 = tmp3 + *ptr3 * 3;
        sum3 -= tmp3;
        
        tmp4 = (*ptr4 & tmp4) | 0xAA;
        sum4 |= tmp4;
        
        /* Update pointers - creates address generation dependencies */
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        ptr4 += stride;
        
        /* Additional computation to increase pressure */
        data[i] = sum1 + sum2 + sum3 + sum4;
    }
    
    g_result += sum1 + sum2 + sum3 + sum4;
}

/* Test 4: Mixed types and operations with manual unrolling */
#pragma GCC unroll 4
void test_mixed_unrolled(short *s, int *i, long *l, float *f, int n) {
    long lacc = l[0];
    int iacc = i[0];
    float facc = f[0];
    short sacc = s[0];
    
    /* Manually unrolled to increase operations per iteration */
    for (int j = 0; j < n - 4; j += 4) {
        /* Multiple operations with carried dependencies */
        lacc = lacc + l[j] * 3;
        iacc = iacc ^ (i[j] + 1);
        facc = facc * 1.1f + f[j];
        sacc = sacc | (s[j] & 0x7F);
        
        lacc = lacc - l[j+1] / 2;
        iacc = iacc * 5 - i[j+1];
        facc = facc - f[j+1] * 0.5f;
        sacc = (sacc << 1) ^ s[j+1];
        
        lacc = lacc | (l[j+2] & 0xFFFFFF);
        iacc = iacc + i[j+2] * 7;
        facc = facc + f[j+2] / 3.0f;
        sacc = sacc + s[j+2] * 3;
        
        lacc = lacc ^ l[j+3];
        iacc = iacc & (i[j+3] | 0x5555);
        facc = facc * 0.8f - f[j+3];
        sacc = sacc - s[j+3];
        
        /* Store results to create anti-dependencies */
        l[j] = lacc;
        i[j] = iacc;
        f[j] = facc;
        s[j] = sacc;
    }
    
    g_result += lacc + iacc + (long)facc + sacc;
}

/* Test 5: PowerPC specific - double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *d1, double *d2, double *d3, double *d4, int n) {
    double acc1 = d1[0], acc2 = d2[0], acc3 = d3[0], acc4 = d4[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple double precision chains */
        acc1 = acc1 * 1.234567 + d1[i];
        acc2 = acc2 - d2[i] * 0.987654;
        acc3 = acc3 + d3[i] / 1.414213;
        acc4 = acc4 * 0.707106 - d4[i];
        
        /* Cross dependencies */
        d1[i-1] = acc1 + acc2;
        d2[i-1] = acc2 * acc3;
        d3[i-1] = acc3 - acc4;
        d4[i-1] = acc4 / (acc1 + 1.0);
    }
    
    g_result += (long)(acc1 + acc2 + acc3 + acc4);
}
#endif

/* Test 6: Array accumulation with varying distances */
void test_varying_distance(int *arr, int n) {
    int v1 = arr[0], v2 = arr[1], v3 = arr[2];
    int t1 = 0, t2 = 0, t3 = 0;
    
    for (int i = 3; i < n; i++) {
        /* Different dependency distances */
        t1 = v1 + arr[i];          /* Uses v1 from previous iteration */
        t2 = v2 * arr[i-1];        /* Uses v2 from previous iteration */
        t3 = v3 ^ arr[i-2];        /* Uses v3 from 2 iterations ago */
        
        /* Update with carried dependencies */
        v1 = t1 * 3 - v1;
        v2 = t2 + v2 / 2;
        v3 = t3 | (v3 << 1);
        
        /* Store to create register pressure */
        arr[i-3] = v1 + v2 + v3;
    }
    
    g_result += v1 + v2 + v3;
}

int main() {
    /* Allocate and initialize arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    
    short *sarr = malloc(SIZE * sizeof(short));
    long *larr = malloc(SIZE * sizeof(long));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
        
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
        darr1[i] = i * 1.234;
        darr2[i] = i * 3.456;
        
        sarr[i] = i & 0x7FFF;
        larr[i] = i * 1000L;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multi_int_recurrence(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(farr1, farr2, darr1, darr2, SIZE);
        test_pointer_chasing(arr1, 4, SIZE);
        test_mixed_unrolled(sarr, arr2, larr, farr1, SIZE);
        test_varying_distance(arr3, SIZE);
        
#ifdef __powerpc__
        test_powerpc_double(darr1, darr2, darr1, darr2, SIZE);
#endif
    }
    
    /* Output result to prevent dead code elimination */
    printf("Result: %ld\n", g_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(farr1);
    free(farr2);
    free(darr1);
    free(darr2);
    free(sarr);
    free(larr);
    
    return 0;
}

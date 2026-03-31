/* test_modulo_sched.c
 * 
 * This program creates loops that trigger GCC's modulo scheduling
 * optimization, specifically targeting the register move scheduling
 * logic in schedule_reg_move() function.
 *
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x1 = a[0];
    int x2 = b[0] + b[1];
    int x3 = c[0] * 2;
    int x4 = d[0] | 0x5555;
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple accumulation with dependency distance 1 */
        x1 = x1 + a[i] * b[i];
        
        /* Chain 2: More complex recurrence with multiple operations */
        x2 = (x2 * 3) + (c[i] << 2) - (d[i] & 0xFF);
        
        /* Chain 3: Alternating operations to increase register pressure */
        x3 = x3 ^ (x3 >> 1) + (a[i] * c[i]);
        
        /* Chain 4: Mixed arithmetic/logical operations */
        x4 = (x4 + x3) | (x2 & x1);
        
        /* Additional operations to create more register pressure */
        a[i] = x1 + x4;
        b[i] = x2 * x3;
        c[i] = x3 ^ x4;
        d[i] = x1 | x2;
    }
    
    /* Store results to prevent optimization */
    global_acc += x1 + x2 + x3 + x4;
}

/* Test 2: Floating point accumulation with mixed operations */
void test_float_recurrence(float *f1, float *f2, double *d1, double *d2, int n) {
    int i;
    float f_acc = f1[0];
    double d_acc = d1[0];
    float f_mul = 1.1f;
    double d_mul = 1.01;
    
    for (i = 1; i < n; i++) {
        /* Float recurrence chain */
        f_acc = f_acc * f_mul + f1[i] * f2[i];
        f_mul = f_mul * 0.99f + f_acc * 0.01f;
        
        /* Double recurrence chain */
        d_acc = d_acc * d_mul + d1[i] + d2[i];
        d_mul = d_mul * 0.995 + d_acc * 0.001;
        
        /* Cross-chain operations to increase pressure */
        f1[i] = f_acc + (float)d_acc;
        d1[i] = d_acc * (double)f_acc;
        
        /* Additional operations */
        f2[i] = f_mul * f2[i-1];
        d2[i] = d_mul + d2[i-1];
    }
    
    global_acc += (long long)(f_acc + d_acc);
}

/* Test 3: Pointer chasing with strided access patterns */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + stride * 2;
    int *end = data + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer-based recurrence chains */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 + (*ptr2 << 1);
        sum3 = sum3 ^ *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional operations to create register moves */
        *ptr1 = sum1 + sum2;
        *ptr2 = sum2 * sum3;
        *ptr3 = sum1 | sum3;
        
        /* More operations to increase pressure */
        sum1 = sum1 ^ sum2;
        sum2 = sum2 + sum3;
        sum3 = sum3 * sum1;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed types and operations with manual unrolling */
#pragma GCC unroll 4
void test_mixed_unrolled(short *s, int *i, long *l, int n) {
    int j;
    short s_acc = s[0];
    int i_acc = i[0];
    long l_acc = l[0];
    
    for (j = 1; j < n - 3; j += 4) {
        /* Unrolled operations with carried dependencies */
        s_acc = s_acc + s[j] - s[j-1];
        i_acc = i_acc * 3 + i[j] * i[j+1];
        l_acc = l_acc ^ (l[j] << 2);
        
        s_acc = s_acc * 2 + s[j+1];
        i_acc = i_acc + i[j+1] * 5;
        l_acc = l_acc | l[j+1];
        
        s_acc = s_acc - s[j+2] / 2;
        i_acc = i_acc ^ (i[j+2] << 1);
        l_acc = l_acc + l[j+2] * 7;
        
        s_acc = s_acc & 0x7FFF + s[j+3];
        i_acc = i_acc * 11 - i[j+3];
        l_acc = l_acc & 0xFFFFFFFF + l[j+3];
        
        /* Store results to arrays */
        s[j] = s_acc;
        i[j] = i_acc;
        l[j] = l_acc;
    }
    
    global_acc += s_acc + i_acc + l_acc;
}

/* Test 5: PowerPC-specific double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0];
    double acc2 = b[0];
    double acc3 = c[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple double precision recurrence chains */
        acc1 = acc1 * 1.01 + a[i] * b[i];
        acc2 = acc2 * 0.99 - c[i] * a[i];
        acc3 = acc3 * 1.001 + b[i] / c[i];
        
        /* Cross dependencies */
        a[i] = acc1 + acc2;
        b[i] = acc2 * acc3;
        c[i] = acc3 - acc1;
        
        /* More operations for register pressure */
        acc1 = acc1 * acc2;
        acc2 = acc2 + acc3;
        acc3 = acc3 * 2.0;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3);
}
#endif

/* Test 6: Vector-style operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
void test_vector_style(int *a, int *b, int *c, int n) {
    int i;
    int v1 = a[0];
    int v2 = b[0];
    int v3 = c[0];
    
    for (i = 1; i < n; i++) {
        /* Strided access patterns that encourage vectorization */
        v1 = v1 + a[i] + a[i-1];
        v2 = v2 * b[i] - b[i-1];
        v3 = v3 ^ c[i] | c[i-1];
        
        /* Additional vector-style operations */
        a[i] = v1 << (i & 3);
        b[i] = v2 >> (i & 1);
        c[i] = v3 + (i & 0xFF);
        
        /* More recurrence chains */
        v1 = v1 * 3 + v2;
        v2 = v2 + v3 * 5;
        v3 = v3 ^ v1;
    }
    
    global_acc += v1 + v2 + v3;
}
#endif

/* Main test driver */
int main() {
    int i;
    clock_t start, end;
    
    /* Allocate and initialize arrays */
    int *a_int = malloc(SIZE * sizeof(int));
    int *b_int = malloc(SIZE * sizeof(int));
    int *c_int = malloc(SIZE * sizeof(int));
    int *d_int = malloc(SIZE * sizeof(int));
    
    float *f_arr1 = malloc(SIZE * sizeof(float));
    float *f_arr2 = malloc(SIZE * sizeof(float));
    double *d_arr1 = malloc(SIZE * sizeof(double));
    double *d_arr2 = malloc(SIZE * sizeof(double));
    
    short *s_arr = malloc(SIZE * sizeof(short));
    int *i_arr = malloc(SIZE * sizeof(int));
    long *l_arr = malloc(SIZE * sizeof(long));
    
    int *data = malloc(SIZE * 4 * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        c_int[i] = rand() % 1000;
        d_int[i] = rand() % 1000;
        
        f_arr1[i] = (float)(rand() % 1000) / 10.0f;
        f_arr2[i] = (float)(rand() % 1000) / 10.0f;
        d_arr1[i] = (double)(rand() % 1000) / 10.0;
        d_arr2[i] = (double)(rand() % 1000) / 10.0;
        
        s_arr[i] = (short)(rand() % 1000);
        i_arr[i] = rand() % 1000;
        l_arr[i] = rand() % 1000;
    }
    
    for (i = 0; i < SIZE * 4; i++) {
        data[i] = rand() % 1000;
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        test_multi_recurrence_int(a_int, b_int, c_int, d_int, SIZE);
        test_float_recurrence(f_arr1, f_arr2, d_arr1, d_arr2, SIZE);
        test_pointer_chasing(data, 8, SIZE);
        test_mixed_unrolled(s_arr, i_arr, l_arr, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double(d_arr1, d_arr2, (double*)f_arr1, SIZE);
        #endif
        
        #if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_style(a_int, b_int, c_int, SIZE);
        #endif
    }
    
    end = clock();
    
    printf("Tests completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(a_int);
    free(b_int);
    free(c_int);
    free(d_int);
    free(f_arr1);
    free(f_arr2);
    free(d_arr1);
    free(d_arr2);
    free(s_arr);
    free(i_arr);
    free(l_arr);
    free(data);
    
    return 0;
}

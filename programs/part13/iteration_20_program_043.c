/* Test program for GCC modulo scheduling register move coverage */
/* Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* For PowerPC: add -mtune=powerpc -mcpu=power8 */
/* For ARM SVE: add -march=armv8-a+sve -ftree-vectorize */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

volatile long long g_result = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    /* Multiple independent dependency chains with distance 1 */
    for (int i = 2; i < n; i++) {
        /* Chain 1: a[i] depends on a[i-1] and a[i-2] */
        x1 = x1 * 3 + y1 * 7;
        y1 = a[i] + x1 * 5;
        a[i] = y1 - x1;
        
        /* Chain 2: b[i] depends on b[i-1] with different ops */
        x2 = (x2 << 3) | (x2 >> 5);
        y2 = b[i] ^ x2;
        b[i] = y2 + (x2 & 0xFF);
        
        /* Chain 3: c[i] depends on c[i-1] with multiplication */
        x3 = x3 * 13 + 17;
        y3 = c[i] * x3;
        c[i] = y3 >> 4;
        
        /* Chain 4: d[i] depends on d[i-1] with complex expression */
        x4 = (x4 * 19) ^ (x4 >> 2);
        y4 = d[i] + x4 * 11;
        d[i] = y4 & 0xFFFF;
        
        /* Cross-chain operations to increase register pressure */
        a[i] ^= b[i] & c[i];
        b[i] += d[i] | a[i];
        c[i] *= d[i] ^ b[i];
        d[i] -= a[i] + c[i];
    }
    
    g_result += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = a[1], prod2 = b[1], prod3 = c[1];
    
    /* Multiple FP dependency chains */
    for (int i = 2; i < n; i++) {
        /* Chain 1: Linear recurrence */
        sum1 = sum1 * 1.01 + a[i];
        prod1 = prod1 * 0.99 * a[i];
        a[i] = sum1 + prod1;
        
        /* Chain 2: Different coefficients */
        sum2 = sum2 * 1.02 + b[i];
        prod2 = prod2 * 0.98 * b[i];
        b[i] = sum2 - prod2;
        
        /* Chain 3: More complex FP ops */
        sum3 = sum3 * 1.03 + c[i];
        prod3 = prod3 * 0.97 * c[i];
        c[i] = sum3 * prod3;
        
        /* Cross dependencies */
        double t1 = a[i] * b[i];
        double t2 = b[i] * c[i];
        double t3 = c[i] * a[i];
        a[i] += t1 - t3;
        b[i] += t2 - t1;
        c[i] += t3 - t2;
    }
    
    g_result += (long long)(sum1 + sum2 + sum3 + prod1 + prod2 + prod3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer chains with dependencies */
        tmp1 = *ptr1 + acc1;
        acc1 = (acc1 << 1) | (tmp1 & 1);
        *ptr1 = tmp1 ^ acc1;
        
        tmp2 = *ptr2 * acc2;
        acc2 = acc2 + (tmp2 >> 4);
        *ptr2 = tmp2 - acc2;
        
        tmp3 = *ptr3 ^ acc3;
        acc3 = acc3 * 3 + tmp3;
        *ptr3 = tmp3 & acc3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computations to increase register pressure */
        int cross1 = tmp1 * tmp2;
        int cross2 = tmp2 * tmp3;
        int cross3 = tmp3 * tmp1;
        acc1 ^= cross1;
        acc2 += cross2;
        acc3 &= cross3;
    }
    
    g_result += acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3;
}

/* Test 4: Mixed types and operations for maximum pressure */
void test_mixed_types(short *s, int *i, long long *ll, float *f, double *d, int n) {
    short s_acc = s[0];
    int i_acc = i[0];
    long long ll_acc = ll[0];
    float f_acc = f[0];
    double d_acc = d[0];
    
    for (int j = 1; j < n; j++) {
        /* Type 1: short with promotion */
        s_acc = (s_acc * 3 + s[j]) & 0x7FFF;
        s[j] = s_acc ^ j;
        
        /* Type 2: integer with shifts */
        i_acc = (i_acc << 2) + i[j];
        i[j] = i_acc | (j * 7);
        
        /* Type 3: long long with 64-bit ops */
        ll_acc = ll_acc * 5 + ll[j];
        ll[j] = ll_acc ^ (ll_acc >> 32);
        
        /* Type 4: float */
        f_acc = f_acc * 1.5f + f[j];
        f[j] = f_acc * 0.9f;
        
        /* Type 5: double */
        d_acc = d_acc * 2.0 + d[j];
        d[j] = d_acc / 1.1;
        
        /* Cross-type operations forcing register moves */
        i_acc += (int)s_acc;
        ll_acc += (long long)i_acc;
        f_acc += (float)(d_acc * 0.5);
        d_acc += (double)f_acc;
        s_acc += (short)(i_acc & 0xFFFF);
    }
    
    g_result += s_acc + i_acc + (int)(ll_acc >> 32) + (int)f_acc + (int)d_acc;
}

/* Test 5: Nested loops with inner loop being modulo scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    for (int i = 1; i < n; i++) {
        int acc1 = a[i-1];
        int acc2 = b[i-1];
        int acc3 = c[i-1];
        
        /* Inner loop with carried dependencies */
        for (int j = 0; j < m; j++) {
            /* Multiple dependency chains in inner loop */
            acc1 = acc1 * 11 + a[j] * 13;
            acc2 = acc2 * 17 + b[j] * 19;
            acc3 = acc3 * 23 + c[j] * 29;
            
            /* Cross dependencies */
            int t1 = acc1 ^ acc2;
            int t2 = acc2 & acc3;
            int t3 = acc3 | acc1;
            
            a[j] = t1 + i;
            b[j] = t2 - j;
            c[j] = t3 * (i + j);
            
            /* More operations to increase pressure */
            acc1 += t2 * 3;
            acc2 += t3 * 5;
            acc3 += t1 * 7;
        }
        
        a[i] = acc1;
        b[i] = acc2;
        c[i] = acc3;
    }
    
    g_result += a[n-1] + b[n-1] + c[n-1];
}

/* PowerPC-specific test with potential for FP/vector register moves */
#ifdef __powerpc__
void test_powerpc_specific(double *a, double *b, int n) {
    double sum_even = 0.0, sum_odd = 0.0;
    double prod_even = 1.0, prod_odd = 1.0;
    
    /* Loop designed to use many FP registers */
    for (int i = 0; i < n; i += 2) {
        double t1 = a[i] * 1.1;
        double t2 = b[i] * 0.9;
        double t3 = a[i+1] * 1.2;
        double t4 = b[i+1] * 0.8;
        
        sum_even = sum_even + t1 + t2;
        sum_odd = sum_odd + t3 + t4;
        prod_even = prod_even * t1 * t2;
        prod_odd = prod_odd * t3 * t4;
        
        /* Cross dependencies */
        double cross1 = sum_even * prod_odd;
        double cross2 = sum_odd * prod_even;
        double cross3 = t1 * t3 + t2 * t4;
        double cross4 = t1 * t4 - t2 * t3;
        
        a[i] = cross1 + cross3;
        b[i] = cross2 - cross4;
        a[i+1] = cross1 * 0.5 + cross4;
        b[i+1] = cross2 * 0.5 - cross3;
    }
    
    g_result += (long long)(sum_even + sum_odd + prod_even + prod_odd);
}
#endif

/* ARM SVE-style test with unknown bounds at compile time */
#ifdef __ARM_FEATURE_SVE
void test_sve_style(int *data, int n) {
    int sum1 = data[0], sum2 = data[1];
    int prod1 = 1, prod2 = 1;
    
    #pragma GCC unroll 4
    for (int i = 2; i < n; i++) {
        /* Multiple chains with varying dependency distances */
        int tmp1 = data[i] + sum1;
        sum1 = (sum1 * 3 + tmp1) & 0xFFFFFF;
        prod1 = prod1 * (tmp1 | 1);
        
        int tmp2 = data[n-i] ^ sum2;
        sum2 = sum2 + (tmp2 << 2);
        prod2 = prod2 * (tmp2 & 0xFF);
        
        /* Data swapping to create anti-dependencies */
        data[i] = tmp1 ^ tmp2;
        data[n-i] = tmp1 & tmp2;
        
        /* Additional computations */
        int cross = (tmp1 * sum2) + (tmp2 * sum1);
        sum1 += cross >> 8;
        sum2 += cross & 0xFF;
    }
    
    g_result += sum1 + sum2 + prod1 + prod2;
}
#endif

int main() {
    /* Allocate and initialize test data */
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *data4 = malloc(SIZE * sizeof(int));
    short *sdata = malloc(SIZE * sizeof(short));
    long long *lldata = malloc(SIZE * sizeof(long long));
    float *fdata = malloc(SIZE * sizeof(float));
    double *ddata1 = malloc(SIZE * sizeof(double));
    double *ddata2 = malloc(SIZE * sizeof(double));
    double *ddata3 = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        data3[i] = rand() % 1000;
        data4[i] = rand() % 1000;
        sdata[i] = rand() % 1000;
        lldata[i] = rand() % 1000;
        fdata[i] = (float)(rand() % 1000) / 10.0f;
        ddata1[i] = (double)(rand() % 1000) / 10.0;
        ddata2[i] = (double)(rand() % 1000) / 10.0;
        ddata3[i] = (double)(rand() % 1000) / 10.0;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multi_recurrence_int(data1, data2, data3, data4, SIZE);
        test_float_accumulate(ddata1, ddata2, ddata3, SIZE);
        test_pointer_chasing(data1, SIZE, 4);
        test_mixed_types(sdata, data2, lldata, fdata, ddata1, SIZE);
        test_nested_loops(data1, data2, data3, 64, 16);
        
        #ifdef __powerpc__
        test_powerpc_specific(ddata1, ddata2, SIZE);
        #endif
        
        #ifdef __ARM_FEATURE_SVE
        test_sve_style(data1, SIZE);
        #endif
    }
    
    /* Output result to prevent optimization */
    printf("Result: %lld\n", g_result);
    
    /* Cleanup */
    free(data1); free(data2); free(data3); free(data4);
    free(sdata); free(lldata); free(fdata);
    free(ddata1); free(ddata2); free(ddata3);
    
    return 0;
}

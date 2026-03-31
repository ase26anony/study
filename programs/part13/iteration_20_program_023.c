/* Test program to trigger modulo scheduling register move coverage in GCC */
/* Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* For PowerPC: add -mtune=powerpc -mcpu=power8 */
/* For ARM SVE: add -march=armv8-a+sve -ftree-vectorize */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

volatile int g_result = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x = a[0];
    int y = b[0];
    int z = c[0];
    int w = d[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent dependency chains */
        x = x * 3 + a[i];          /* Chain 1: distance 1 */
        y = y + b[i] * 7;          /* Chain 2: distance 1 */
        z = z << 2 | c[i];         /* Chain 3: distance 1 */
        w = (w & 0xFF) + d[i] * 5; /* Chain 4: distance 1 */
        
        /* Additional operations to increase register pressure */
        a[i] = x + y;
        b[i] = y ^ z;
        c[i] = z * w;
        d[i] = w - x;
    }
    
    g_result += x + y + z + w;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = c[0];
    double prod = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];      /* Distance 1 */
        sum2 = sum2 + b[i] * sum1;      /* Distance 1 with cross-chain */
        sum3 = sum3 - c[i] / (i+1);     /* Distance 1 */
        prod = prod * (1.0 + a[i]*b[i]); /* Distance 1 */
        
        /* More operations for register pressure */
        a[i] = sum1 * sum2;
        b[i] = sum2 - sum3;
        c[i] = sum3 * prod;
    }
    
    g_result += (int)(sum1 + sum2 + sum3 + prod);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chase(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2*stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer-based dependency chains */
        acc1 = acc1 * 2 + *ptr1;    /* Distance 1 */
        acc2 = acc2 + *ptr2 * 3;    /* Distance 1 */
        acc3 = acc3 ^ *ptr3;        /* Distance 1 */
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation */
        *ptr1 = acc1 & acc2;
        *ptr2 = acc2 | acc3;
        *ptr3 = acc3 ^ acc1;
    }
    
    g_result += acc1 + acc2 + acc3;
}

/* Test 4: Mixed types and operations for maximum pressure */
void test_mixed_pressure(short *s, int *i, long *l, float *f, int n) {
    short s_acc = s[0];
    int i_acc = i[0];
    long l_acc = l[0];
    float f_acc = f[0];
    
    for (int j = 1; j < n; j++) {
        /* Type-mixed dependency chains */
        s_acc = (s_acc * 3 + s[j]) & 0x7FFF;
        i_acc = i_acc + i[j] * 7 - s_acc;
        l_acc = l_acc ^ (l[j] << 2);
        f_acc = f_acc * 1.5f + f[j];
        
        /* Cross-type operations */
        i[j] = s_acc + (int)f_acc;
        l[j] = i_acc * l_acc;
        f[j] = (float)(s_acc * i_acc) / (j+1);
        s[j] = (short)(l_acc & 0xFFFF);
    }
    
    g_result += s_acc + i_acc + (int)l_acc + (int)f_acc;
}

/* Test 5: Nested loops with inner loop being modulo scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    for (int i = 1; i < n; i++) {
        int acc1 = a[i-1];
        int acc2 = b[i-1];
        int acc3 = c[i-1];
        
        /* Inner loop designed for modulo scheduling */
        for (int j = 0; j < m; j++) {
            /* High register pressure inner loop */
            acc1 = acc1 * 2 + a[j];
            acc2 = acc2 + b[j] * acc1;
            acc3 = acc3 ^ (c[j] << 1);
            
            /* Additional operations */
            a[j] = acc1 & acc2;
            b[j] = acc2 | acc3;
            c[j] = acc3 ^ acc1;
        }
        
        a[i] = acc1;
        b[i] = acc2;
        c[i] = acc3;
    }
    
    g_result += a[n-1] + b[n-1] + c[n-1];
}

/* Test 6: PowerPC-specific patterns with double operations */
#ifdef __powerpc__ || __PPC__
void test_powerpc_double(double *a, double *b, int n) {
    double d1 = a[0];
    double d2 = b[0];
    double d3 = 1.0;
    double d4 = 0.5;
    
    for (int i = 1; i < n; i++) {
        /* Multiple double-precision dependency chains */
        d1 = d1 * 1.01 + a[i];
        d2 = d2 + b[i] * d1;
        d3 = d3 * (1.0 + a[i] * b[i]);
        d4 = d4 - (a[i] / b[i]) * 0.1;
        
        /* Cross dependencies */
        a[i] = d1 * d2 + d3;
        b[i] = d2 - d4 * d3;
    }
    
    g_result += (int)(d1 + d2 + d3 + d4);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
void test_unrolled_loop(int *a, int *b, int n) {
    int x0 = a[0], x1 = a[1], x2 = a[2], x3 = a[3];
    int y0 = b[0], y1 = b[1], y2 = b[2], y3 = b[3];
    
    for (int i = 4; i < n; i += 4) {
        /* Unrolled dependency chains */
        x0 = x0 * 3 + a[i];
        y0 = y0 + b[i] * x0;
        
        x1 = x1 * 5 + a[i+1];
        y1 = y1 + b[i+1] * x1;
        
        x2 = x2 * 7 + a[i+2];
        y2 = y2 + b[i+2] * x2;
        
        x3 = x3 * 11 + a[i+3];
        y3 = y3 + b[i+3] * x3;
        
        /* Results back to arrays */
        a[i] = x0 ^ y0;
        b[i] = y0 & x1;
        a[i+1] = x1 | y1;
        b[i+1] = y1 ^ x2;
        a[i+2] = x2 + y2;
        b[i+2] = y2 - x3;
        a[i+3] = x3 * y3;
        b[i+3] = y3 / (x0 + 1);
    }
    
    g_result += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
}

int main() {
    /* Allocate and initialize test arrays */
    int *a_int = malloc(SIZE * sizeof(int));
    int *b_int = malloc(SIZE * sizeof(int));
    int *c_int = malloc(SIZE * sizeof(int));
    int *d_int = malloc(SIZE * sizeof(int));
    
    double *a_dbl = malloc(SIZE * sizeof(double));
    double *b_dbl = malloc(SIZE * sizeof(double));
    double *c_dbl = malloc(SIZE * sizeof(double));
    
    short *s_arr = malloc(SIZE * sizeof(short));
    long *l_arr = malloc(SIZE * sizeof(long));
    float *f_arr = malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        a_int[i] = i % 100;
        b_int[i] = (i * 3) % 100;
        c_int[i] = (i * 5) % 100;
        d_int[i] = (i * 7) % 100;
        
        a_dbl[i] = i * 0.1;
        b_dbl[i] = i * 0.3;
        c_dbl[i] = i * 0.5;
        
        s_arr[i] = i % 32767;
        l_arr[i] = i * 1000L;
        f_arr[i] = i * 0.7f;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multi_recurrence_int(a_int, b_int, c_int, d_int, SIZE);
        test_float_accumulate(a_dbl, b_dbl, c_dbl, SIZE);
        test_pointer_chase(a_int, 4, SIZE/4);
        test_mixed_pressure(s_arr, a_int, l_arr, f_arr, SIZE);
        test_nested_loops(a_int, b_int, c_int, SIZE/8, 8);
        test_unrolled_loop(a_int, b_int, SIZE);
        
        #ifdef __powerpc__ || __PPC__
        test_powerpc_double(a_dbl, b_dbl, SIZE);
        #endif
    }
    
    /* Output result to prevent optimization */
    printf("Result: %d\n", g_result);
    
    /* Cleanup */
    free(a_int);
    free(b_int);
    free(c_int);
    free(d_int);
    free(a_dbl);
    free(b_dbl);
    free(c_dbl);
    free(s_arr);
    free(l_arr);
    free(f_arr);
    
    return 0;
}

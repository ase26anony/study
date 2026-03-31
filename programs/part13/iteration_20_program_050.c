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

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multiple_int_chains(int *a, int *b, int *c, int *d, int n) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Four independent recurrence chains with different distances */
        x1 = x1 * 3 + a[i];          /* Distance 1 */
        x2 = x2 * 5 + b[i] - x1;     /* Distance 1 with cross-chain dep */
        x3 = x3 * 7 + c[i] + x2;     /* Distance 1 */
        x4 = x4 * 11 + d[i] - x3;    /* Distance 1 */
        
        /* Additional chains with arithmetic operations */
        y1 = (y1 << 1) ^ a[i];
        y2 = (y2 * 13) | b[i];
        y3 = y3 + (c[i] * y2);
        y4 = y4 - (d[i] ^ y3);
        
        /* Mix results to create more dependencies */
        a[i] = x1 + y1;
        b[i] = x2 ^ y2;
        c[i] = x3 * y3;
        d[i] = x4 - y4;
    }
    
    global_sum += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *fa, float *fb, float *fc, int n) {
    float sum1 = 0.1f, sum2 = 0.2f, sum3 = 0.3f;
    float prod1 = 1.0f, prod2 = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01f + fa[i];
        sum2 = sum2 * 1.02f + fb[i] - sum1;
        sum3 = sum3 * 1.03f + fc[i] * sum2;
        
        prod1 = prod1 * (fa[i] + 0.5f);
        prod2 = prod2 * (fb[i] - 0.3f) + sum3;
        
        /* Cross dependencies to force register moves */
        fa[i] = sum1 * prod1;
        fb[i] = sum2 + prod2;
        fc[i] = sum3 - prod1 * prod2;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer chasing with strided access */
void test_pointer_chasing(int *arr, int stride, int n) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2 * stride;
    int *end = arr + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer-based recurrence chains */
        acc1 = acc1 * 2 + *ptr1;
        acc2 = acc2 * 3 + *ptr2 - acc1;
        acc3 = acc3 * 5 + *ptr3 ^ acc2;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional operations to increase register pressure */
        *ptr1 = acc1 >> 1;
        *ptr2 = acc2 & 0xFF;
        *ptr3 = acc3 | 0xAA;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with manual unrolling */
void test_mixed_ops_unrolled(short *sa, int *ia, long long *la, int n) {
    long long ll1 = 1, ll2 = 2, ll3 = 3;
    int i1 = 4, i2 = 5, i3 = 6;
    short s1 = 7, s2 = 8, s3 = 9;
    
    /* Manual unrolling to increase operations per iteration */
    for (int i = 0; i < n - 3; i += 4) {
        /* Chain 1: long long operations */
        ll1 = ll1 * 17 + la[i];
        ll2 = ll2 * 19 + la[i+1] - ll1;
        ll3 = ll3 * 23 + la[i+2] ^ ll2;
        
        /* Chain 2: integer operations dependent on long long chain */
        i1 = i1 * 29 + ia[i] + (int)(ll1 & 0xFFFFFFFF);
        i2 = i2 * 31 + ia[i+1] - (int)(ll2 >> 32);
        i3 = i3 * 37 + ia[i+2] | (int)ll3;
        
        /* Chain 3: short operations dependent on integer chain */
        s1 = (s1 * 41 + sa[i]) & (short)i1;
        s2 = (s2 * 43 + sa[i+1]) | (short)i2;
        s3 = (s3 * 47 + sa[i+2]) ^ (short)i3;
        
        /* Store results creating anti-dependencies */
        la[i] = ll1 + i1;
        la[i+1] = ll2 - i2;
        la[i+2] = ll3 * i3;
        
        ia[i] = i1 ^ s1;
        ia[i+1] = i2 + s2;
        ia[i+2] = i3 - s3;
        
        sa[i] = s1 * 2;
        sa[i+1] = s2 / 3;
        sa[i+2] = s3 << 1;
    }
    
    global_sum += ll1 + ll2 + ll3 + i1 + i2 + i3 + s1 + s2 + s3;
}

/* Test 5: PowerPC specific - double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *da, double *db, double *dc, int n) {
    double d1 = 0.1, d2 = 0.2, d3 = 0.3;
    double d4 = 0.4, d5 = 0.5, d6 = 0.6;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double precision dependency chains */
        d1 = d1 * 1.1 + da[i];
        d2 = d2 * 1.2 + db[i] * d1;
        d3 = d3 * 1.3 + dc[i] - d2;
        
        d4 = d4 / 1.4 + da[i] * d3;
        d5 = d5 * 1.5 - db[i] + d4;
        d6 = d6 / 1.6 + dc[i] * d5;
        
        /* Create register pressure with many live values */
        da[i] = d1 + d4;
        db[i] = d2 * d5;
        dc[i] = d3 - d6;
    }
    
    global_sum += (long long)(d1 + d2 + d3 + d4 + d5 + d6);
}
#endif

/* Test 6: Compile-time unknown bounds to trigger vectorization */
void test_variable_bound(int *a, int *b, int *c, int start, int end) {
    int x = 1, y = 2, z = 3;
    
    /* Loop with runtime bounds - harder to optimize away */
    for (int i = start; i < end; i++) {
        x = x * 3 + a[i];
        y = y * 5 + b[i] - x;
        z = z * 7 + c[i] ^ y;
        
        a[i] = x >> 1;
        b[i] = y & 0xFF;
        c[i] = z | 0xAA;
    }
    
    global_sum += x + y + z;
}

/* Main test driver */
int main() {
    /* Initialize arrays with pattern */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    float *fc = malloc(SIZE * sizeof(float));
    short *sa = malloc(SIZE * sizeof(short));
    int *ia = malloc(SIZE * sizeof(int));
    long long *la = malloc(SIZE * sizeof(long long));
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = i * 4;
        fa[i] = i * 0.1f;
        fb[i] = i * 0.2f;
        fc[i] = i * 0.3f;
        sa[i] = i % 100;
        ia[i] = i * 5;
        la[i] = i * 100LL;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multiple_int_chains(a, b, c, d, SIZE);
        test_float_accumulate(fa, fb, fc, SIZE);
        test_pointer_chasing(a, 4, SIZE);
        test_mixed_ops_unrolled(sa, ia, la, SIZE);
        
        /* Variable bound test with different ranges */
        int start = iter % 100;
        test_variable_bound(a, b, c, start, SIZE - start);
        
        #ifdef __powerpc__
        double *da = malloc(SIZE * sizeof(double));
        double *db = malloc(SIZE * sizeof(double));
        double *dc = malloc(SIZE * sizeof(double));
        for (int i = 0; i < SIZE; i++) {
            da[i] = i * 0.01;
            db[i] = i * 0.02;
            dc[i] = i * 0.03;
        }
        test_powerpc_double(da, db, dc, SIZE);
        free(da); free(db); free(dc);
        #endif
    }
    
    printf("Final global sum: %lld\n", global_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc);
    free(sa); free(ia); free(la);
    
    return 0;
}

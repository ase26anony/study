/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multiple_int_chains(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent dependency chains */
    int acc1 = a[0];
    int acc2 = a[1];
    int acc3 = b[0];
    int acc4 = c[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        acc1 = acc1 * 3 + b[i];
        a[i] = acc1;
        
        /* Chain 2: distance-1 with different operation */
        acc2 = (acc2 << 2) - c[i];
        b[i] = acc2;
        
        /* Chain 3: distance-1 with multiply */
        acc3 = acc3 * 7 + d[i];
        c[i] = acc3;
        
        /* Chain 4: distance-1 with bit operations */
        acc4 = (acc4 ^ d[i]) + a[i-1];
        d[i] = acc4;
        
        /* Additional operations to increase register pressure */
        int temp1 = a[i] & b[i];
        int temp2 = c[i] | d[i];
        int temp3 = temp1 * temp2;
        int temp4 = temp3 << (i & 3);
        
        global_acc += temp4;
    }
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(float *fa, float *fb, float *fc, int n) {
    int i;
    float f1 = fa[0];
    float f2 = fb[0];
    float f3 = fc[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        f1 = f1 * 1.5f + fa[i];
        fa[i] = f1;
        
        f2 = f2 * 2.0f - fb[i];
        fb[i] = f2;
        
        f3 = f3 * 0.5f + fc[i];
        fc[i] = f3;
        
        /* Cross-chain operations to create register pressure */
        float t1 = f1 * f2;
        float t2 = f2 * f3;
        float t3 = f3 * f1;
        float t4 = t1 + t2 + t3;
        
        global_acc += (long long)t4;
    }
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *base, int stride, int n) {
    int *ptr1 = base;
    int *ptr2 = base + stride;
    int *ptr3 = base + 2*stride;
    int *end = base + n*stride;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    while (ptr1 < end) {
        /* Multiple pointer chains with carried dependencies */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 * 3 + *ptr2;
        sum3 = sum3 * 5 + *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Register pressure operations */
        int mix = (sum1 & sum2) | (sum3 << 1);
        global_acc += mix;
    }
}

/* Test 4: Nested loops with inner loop having carried dependency */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = a[i];
        
        /* Inner loop with carried dependency */
        for (j = 0; j < m; j++) {
            acc = acc * 11 + b[j] * c[j];
            a[i] += acc;
            
            /* Additional operations for register pressure */
            int t1 = b[j] << (j & 3);
            int t2 = c[j] >> (j & 1);
            int t3 = t1 * t2;
            global_acc += t3;
        }
    }
}

/* Test 5: Mixed types and operations for maximum register pressure */
void test_mixed_operations(short *sa, int *ia, long long *lla, float *fa, int n) {
    int i;
    short s_acc = sa[0];
    int i_acc = ia[0];
    long long ll_acc = lla[0];
    float f_acc = fa[0];
    
    for (i = 1; i < n; i++) {
        /* Type 1: 16-bit operations */
        s_acc = (s_acc * 3 + sa[i]) & 0x7FFF;
        sa[i] = s_acc;
        
        /* Type 2: 32-bit operations */
        i_acc = (i_acc << 1) + ia[i];
        ia[i] = i_acc;
        
        /* Type 3: 64-bit operations */
        ll_acc = ll_acc * 7 + lla[i];
        lla[i] = ll_acc;
        
        /* Type 4: Floating-point operations */
        f_acc = f_acc * 1.25f + fa[i];
        fa[i] = f_acc;
        
        /* Cross-type mixing for register pressure */
        long long mix = (long long)s_acc * i_acc + ll_acc + (long long)f_acc;
        global_acc += mix;
    }
}

/* Test 6: PowerPC-specific patterns with double precision */
#ifdef __powerpc__
void test_powerpc_double(double *da, double *db, double *dc, int n) {
    int i;
    double d1 = da[0];
    double d2 = db[0];
    
    for (i = 1; i < n; i++) {
        /* Double precision recurrence chains */
        d1 = d1 * 1.75 + da[i] * db[i];
        da[i] = d1;
        
        d2 = d2 * 2.5 - dc[i] * da[i-1];
        db[i] = d2;
        
        /* Additional FP operations */
        double t1 = d1 * d2;
        double t2 = da[i] * dc[i];
        double t3 = t1 + t2;
        
        global_acc += (long long)t3;
    }
}
#endif

/* Test 7: Loop with compile-time unrolling hint */
void test_unrolled_loop(int *a, int *b, int *c, int n) {
    int i;
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    
    #pragma GCC unroll 4
    for (i = 1; i < n; i++) {
        /* Multiple operations to encourage unrolling */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 * 5 + b[i];
        acc3 = acc3 * 7 + c[i];
        
        a[i] = acc1;
        b[i] = acc2;
        c[i] = acc3;
        
        /* Complex expression for register pressure */
        int t1 = (acc1 & acc2) | (acc3 << 2);
        int t2 = (acc1 ^ acc2) + (acc3 >> 1);
        int t3 = t1 * t2;
        
        global_acc += t3;
    }
}

/* Main test driver */
int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize test arrays */
    int *a1 = malloc(SIZE * sizeof(int));
    int *b1 = malloc(SIZE * sizeof(int));
    int *c1 = malloc(SIZE * sizeof(int));
    int *d1 = malloc(SIZE * sizeof(int));
    
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    float *fc = malloc(SIZE * sizeof(float));
    
    short *sa = malloc(SIZE * sizeof(short));
    int *ia = malloc(SIZE * sizeof(int));
    long long *lla = malloc(SIZE * sizeof(long long));
    float *fa2 = malloc(SIZE * sizeof(float));
    
    #ifdef __powerpc__
    double *da = malloc(SIZE * sizeof(double));
    double *db = malloc(SIZE * sizeof(double));
    double *dc = malloc(SIZE * sizeof(double));
    #endif
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        a1[i] = rand() % 100;
        b1[i] = rand() % 100;
        c1[i] = rand() % 100;
        d1[i] = rand() % 100;
        
        fa[i] = (float)(rand() % 100) / 10.0f;
        fb[i] = (float)(rand() % 100) / 10.0f;
        fc[i] = (float)(rand() % 100) / 10.0f;
        
        sa[i] = (short)(rand() % 100);
        ia[i] = rand() % 100;
        lla[i] = rand() % 100;
        fa2[i] = (float)(rand() % 100) / 10.0f;
        
        #ifdef __powerpc__
        da[i] = (double)(rand() % 100) / 10.0;
        db[i] = (double)(rand() % 100) / 10.0;
        dc[i] = (double)(rand() % 100) / 10.0;
        #endif
    }
    
    start = clock();
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        /* Alternate between different test patterns */
        switch (i % 7) {
            case 0:
                test_multiple_int_chains(a1, b1, c1, d1, SIZE);
                break;
            case 1:
                test_float_recurrence(fa, fb, fc, SIZE);
                break;
            case 2:
                test_pointer_chasing(a1, 4, SIZE/4);
                break;
            case 3:
                test_nested_loops(a1, b1, c1, 16, 64);
                break;
            case 4:
                test_mixed_operations(sa, ia, lla, fa2, SIZE);
                break;
            case 5:
                #ifdef __powerpc__
                test_powerpc_double(da, db, dc, SIZE);
                #else
                test_unrolled_loop(a1, b1, c1, SIZE);
                #endif
                break;
            case 6:
                test_unrolled_loop(a1, b1, c1, SIZE);
                break;
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Output results to ensure computations aren't optimized away */
    printf("Final accumulator value: %lld\n", global_acc);
    printf("Total execution time: %f seconds\n", cpu_time_used);
    printf("Average iteration time: %f microseconds\n", 
           cpu_time_used * 1e6 / ITERATIONS);
    
    /* Cleanup */
    free(a1);
    free(b1);
    free(c1);
    free(d1);
    free(fa);
    free(fb);
    free(fc);
    free(sa);
    free(ia);
    free(lla);
    free(fa2);
    
    #ifdef __powerpc__
    free(da);
    free(db);
    free(dc);
    #endif
    
    return 0;
}

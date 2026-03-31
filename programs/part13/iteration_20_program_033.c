/* test_modulo_sched.c
 * 
 * Test program to trigger GCC's modulo scheduling register move logic
 * Specifically targets lines 596-606 in modulo-sched.cc
 * 
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * 
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multiple_recurrence_chains(int *a, int *b, int *c, int *d, int n) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Four independent recurrence chains with different distances */
        x1 = x1 * 13 + b[i];          /* Distance 1 chain */
        x2 = x2 * 17 + c[i] + x1;     /* Depends on x1 from same iteration */
        x3 = x3 * 19 + d[i] + x2;     /* Depends on x2 from same iteration */
        x4 = x4 * 23 + a[i] + x3;     /* Depends on x3 from same iteration */
        
        /* Additional chains with different operations */
        y1 = (y1 << 3) ^ b[i];
        y2 = (y2 << 2) | c[i] + y1;
        y3 = (y3 << 1) & d[i] * y2;
        y4 = (y4 >> 1) + a[i] ^ y3;
        
        /* Mix results to create anti-dependencies */
        a[i] = x1 + y1;
        b[i] = x2 + y2;
        c[i] = x3 + y3;
        d[i] = x4 + y4;
    }
    
    /* Accumulate to prevent optimization */
    global_sum += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(float *fa, float *fb, float *fc, int n) {
    float sum1 = 0.1f, sum2 = 0.2f, sum3 = 0.3f, sum4 = 0.4f;
    float prod1 = 1.0f, prod2 = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01f + fa[i];          /* Distance 1 */
        sum2 = sum2 * 1.02f + fb[i] + sum1;   /* Depends on sum1 */
        sum3 = sum3 * 1.03f + fc[i] + sum2;   /* Depends on sum2 */
        sum4 = sum4 * 1.04f + fa[i] * sum3;   /* Depends on sum3 */
        
        /* Parallel product chains */
        prod1 = prod1 * (0.99f + fb[i] * 0.01f);
        prod2 = prod2 * (0.98f + fc[i] * 0.02f) + prod1;
        
        /* Create anti-dependencies by reusing arrays */
        fa[i] = sum1 + prod1;
        fb[i] = sum2 + prod2;
        fc[i] = sum3 * sum4;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + sum4 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access (triggers vectorization) */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *ptr4 = data + 3 * stride;
    
    int acc1 = 0, acc2 = 0, acc3 = 0, acc4 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer-chasing chains */
        acc1 = acc1 * 3 + *ptr1;
        acc2 = acc2 * 5 + *ptr2 + acc1;
        acc3 = acc3 * 7 + *ptr3 + acc2;
        acc4 = acc4 * 11 + *ptr4 + acc3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        ptr4 += 4;
        
        /* Create register pressure with temporary calculations */
        int t1 = acc1 & 0xFF;
        int t2 = acc2 | 0x55;
        int t3 = acc3 ^ t1;
        int t4 = acc4 + t2 * t3;
        
        /* Store back to create dependencies */
        if (i % 2 == 0) {
            *ptr1 = t1;
            *ptr2 = t2;
        } else {
            *ptr3 = t3;
            *ptr4 = t4;
        }
    }
    
    global_sum += acc1 + acc2 + acc3 + acc4;
}

/* Test 4: Mixed types and operations for maximum register pressure */
void test_mixed_operations(short *sa, int *ia, float *fa, double *da, int n) {
    int int_acc = 1;
    float float_acc = 1.0f;
    double double_acc = 1.0;
    long long long_acc = 1;
    
    for (int i = 0; i < n; i++) {
        /* Interleaved dependency chains across different types */
        int_acc = int_acc * 2 + sa[i];                /* Uses short */
        float_acc = float_acc * 1.5f + ia[i];         /* Uses int */
        double_acc = double_acc * 1.25 + fa[i];       /* Uses float */
        long_acc = long_acc * 3 + (long long)da[i];   /* Uses double */
        
        /* Cross-type dependencies to force register moves */
        sa[i] = (short)(int_acc & 0xFFFF);
        ia[i] = (int)(float_acc * 100);
        fa[i] = (float)(double_acc / 2.0);
        da[i] = (double)(long_acc % 1000);
        
        /* Additional operations to increase register pressure */
        int t1 = int_acc << 2;
        float t2 = float_acc * 0.75f;
        double t3 = double_acc + t2;
        long long t4 = long_acc ^ t1;
        
        /* More mixing */
        int_acc = t1 + (int)t4;
        float_acc = t2 + (float)t3;
        double_acc = t3 - t2;
        long_acc = t4 | (long long)t1;
    }
    
    global_sum += int_acc + (long long)float_acc + (long long)double_acc + long_acc;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *a, int *b, int *c, int outer, int inner) {
    for (int o = 0; o < outer; o++) {
        int acc1 = a[o];
        int acc2 = b[o];
        int acc3 = c[o];
        
        /* Innermost loop - this is where modulo scheduling happens */
        for (int i = 0; i < inner; i++) {
            /* Multiple recurrence relations with different distances */
            acc1 = acc1 * 11 + (i % 16);
            acc2 = acc2 * 13 + acc1 + (i % 8);
            acc3 = acc3 * 17 + acc2 + (i % 4);
            
            /* Additional operations to increase pressure */
            int t1 = acc1 & 0xFF;
            int t2 = acc2 | 0xAA;
            int t3 = acc3 ^ t1;
            int t4 = t1 + t2 * t3;
            
            /* Create anti-dependencies */
            if (i % 3 == 0) {
                acc1 = t4;
            } else if (i % 3 == 1) {
                acc2 = t4 + acc1;
            } else {
                acc3 = t4 + acc2;
            }
        }
        
        a[o] = acc1;
        b[o] = acc2;
        c[o] = acc3;
    }
    
    global_sum += a[outer-1] + b[outer-1] + c[outer-1];
}

/* Test 6: PowerPC specific patterns (if compiled for PowerPC) */
#ifdef __powerpc__
void test_powerpc_specific(double *da, double *db, double *dc, int n) {
    double acc1 = 1.0, acc2 = 2.0, acc3 = 3.0, acc4 = 4.0;
    double mul1 = 1.0, mul2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double-precision chains */
        acc1 = acc1 * 1.1 + da[i];
        acc2 = acc2 * 1.2 + db[i] * acc1;
        acc3 = acc3 * 1.3 + dc[i] * acc2;
        acc4 = acc4 * 1.4 + da[i] * acc3;
        
        /* Parallel multiplicative chains */
        mul1 = mul1 * (0.9 + db[i] * 0.1);
        mul2 = mul2 * (0.8 + dc[i] * 0.2) * mul1;
        
        /* Store results creating anti-deps */
        da[i] = acc1 + mul1;
        db[i] = acc2 + mul2;
        dc[i] = acc3 * acc4;
        
        /* Additional FP operations */
        double t1 = acc1 * 2.0;
        double t2 = acc2 / 1.5;
        double t3 = t1 + t2;
        double t4 = t3 * mul1;
        
        /* Mix them back */
        acc1 = t4 - t1;
        acc2 = t3 + t2;
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3 + acc4 + mul1 + mul2);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_manual_unroll(int *a, int *b, int n) {
    int acc1 = 1, acc2 = 2, acc3 = 3, acc4 = 4;
    
    for (int i = 0; i < n; i += 4) {
        /* Unrolled operations creating more register pressure */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 * 5 + b[i] + acc1;
        
        acc3 = acc3 * 7 + a[i+1];
        acc4 = acc4 * 11 + b[i+1] + acc3;
        
        acc1 = acc1 * 13 + a[i+2] + acc4;
        acc2 = acc2 * 17 + b[i+2] + acc1;
        
        acc3 = acc3 * 19 + a[i+3];
        acc4 = acc4 * 23 + b[i+3] + acc3;
        
        /* Store results */
        a[i] = acc1;
        b[i] = acc2;
        a[i+1] = acc3;
        b[i+1] = acc4;
        
        /* Swap chains to create more dependencies */
        int tmp = acc1;
        acc1 = acc3;
        acc3 = tmp;
        tmp = acc2;
        acc2 = acc4;
        acc4 = tmp;
    }
    
    global_sum += acc1 + acc2 + acc3 + acc4;
}

/* Main driver that runs all tests repeatedly */
int main() {
    /* Initialize data arrays */
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *data4 = malloc(SIZE * sizeof(int));
    float *fdata1 = malloc(SIZE * sizeof(float));
    float *fdata2 = malloc(SIZE * sizeof(float));
    float *fdata3 = malloc(SIZE * sizeof(float));
    short *sdata = malloc(SIZE * sizeof(short));
    double *ddata1 = malloc(SIZE * sizeof(double));
    double *ddata2 = malloc(SIZE * sizeof(double));
    double *ddata3 = malloc(SIZE * sizeof(double));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = i % 100;
        data2[i] = (i * 3) % 100;
        data3[i] = (i * 5) % 100;
        data4[i] = (i * 7) % 100;
        fdata1[i] = (i % 50) * 0.1f;
        fdata2[i] = (i % 30) * 0.2f;
        fdata3[i] = (i % 20) * 0.3f;
        sdata[i] = (short)(i % 256);
        ddata1[i] = (i % 60) * 0.05;
        ddata2[i] = (i % 40) * 0.1;
        ddata3[i] = (i % 25) * 0.2;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_multiple_recurrence_chains(data1, data2, data3, data4, SIZE);
        test_float_accumulation(fdata1, fdata2, fdata3, SIZE);
        test_pointer_chasing(data1, 8, SIZE/2);
        test_mixed_operations(sdata, data2, fdata1, ddata1, SIZE);
        test_nested_loops(data1, data2, data3, 16, 64);
        test_manual_unroll(data3, data4, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_specific(ddata1, ddata2, ddata3, SIZE);
        #endif
        
        /* Occasionally reset data to prevent overflow */
        if (iter % 1000 == 0) {
            for (int i = 0; i < SIZE; i++) {
                data1[i] = i % 100;
                data2[i] = (i * 3) % 100;
            }
        }
    }
    
    printf("Final global sum: %lld\n", global_sum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(sdata);
    free(ddata1);
    free(ddata2);
    free(ddata3);
    
    return 0;
}

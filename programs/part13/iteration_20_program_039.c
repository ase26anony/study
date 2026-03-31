/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    int acc4 = d[0];
    int acc5 = a[1];
    int acc6 = b[1];
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains */
        acc1 = acc1 + b[i] * 3;      /* Chain 1: distance 1 */
        acc2 = acc2 * 2 + c[i];      /* Chain 2: distance 1 */
        acc3 = acc3 ^ (d[i] << 2);   /* Chain 3: distance 1 */
        acc4 = acc4 - (a[i] >> 1);   /* Chain 4: distance 1 */
        acc5 = (acc5 & 0xFF) + acc1; /* Chain 5: depends on chain 1 */
        acc6 = acc6 * 7 + acc2;      /* Chain 6: depends on chain 2 */
        
        /* Additional operations to increase register pressure */
        int temp1 = acc1 + acc3;
        int temp2 = acc2 + acc4;
        int temp3 = acc5 ^ acc6;
        
        /* Store results to prevent optimization */
        a[i] = acc1 + temp1;
        b[i] = acc2 + temp2;
        c[i] = acc3 + temp3;
        d[i] = acc4 + temp1 + temp2;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4 + acc5 + acc6;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(float *fa, float *fb, float *fc, int n) {
    float sum1 = fa[0];
    float sum2 = fb[0];
    float prod1 = fc[0];
    float prod2 = fa[1];
    
    for (int i = 1; i < n; i++) {
        /* Floating-point recurrence chains */
        sum1 = sum1 + fa[i] * 1.5f;    /* Distance 1 */
        sum2 = sum2 - fb[i] * 0.5f;    /* Distance 1 */
        prod1 = prod1 * 1.1f + fc[i];  /* Distance 1 */
        prod2 = prod2 * 0.9f + fa[i];  /* Distance 1 */
        
        /* Cross-chain dependencies */
        float mix1 = sum1 * prod1;
        float mix2 = sum2 * prod2;
        float mix3 = mix1 + mix2;
        
        /* Additional FP operations */
        fa[i] = sum1 + mix3;
        fb[i] = sum2 - mix3;
        fc[i] = prod1 * mix3;
    }
    
    global_acc += (long long)(sum1 + sum2 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + 1;
    int *ptr3 = arr + 2;
    int *end = arr + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    while (ptr1 < end - 3) {
        /* Multiple pointer-chasing chains */
        sum1 = sum1 * 3 + *ptr1;    /* Distance through ptr1 advancement */
        sum2 = sum2 ^ (*ptr2 << 1); /* Different operation for variety */
        sum3 = sum3 + (*ptr3 >> 2);
        
        /* Update pointers with different strides */
        ptr1 += stride;
        ptr2 += stride + 1;
        ptr3 += stride + 2;
        
        /* Additional computation to increase register pressure */
        int temp = sum1 + sum2;
        sum1 = sum2 + sum3;
        sum2 = sum3 + temp;
        sum3 = temp ^ sum1;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with variable dependency distances */
void test_mixed_ops_variable_distance(short *s1, short *s2, int *i1, int *i2, int n) {
    int acc1 = i1[0];
    int acc2 = i2[0];
    short acc3 = s1[0];
    short acc4 = s2[0];
    
    for (int i = 1; i < n; i++) {
        /* Operations with different latencies and register requirements */
        acc1 = (acc1 << 3) | (i1[i] & 0xFF);      /* Shift + bitwise */
        acc2 = acc2 * 13 + i2[i];                 /* Multiplication */
        acc3 = acc3 + s1[i] * 5;                  /* 16-bit operations */
        acc4 = acc4 - s2[i] / 2;                  /* Different operation */
        
        /* Cross-type mixing */
        int mix1 = acc1 + acc3;
        int mix2 = acc2 + acc4;
        int mix3 = mix1 * mix2;
        
        /* Store back with different patterns */
        i1[i] = mix1 ^ mix3;
        i2[i] = mix2 + mix3;
        s1[i] = (mix1 >> 8) & 0xFFFF;
        s2[i] = (mix2 >> 8) & 0xFFFF;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int r = 0; r < rows; r++) {
        int *row = mat + r * cols;
        int prev = row[0];
        
        /* Innermost loop with carried dependency */
        for (int c = 1; c < cols; c++) {
            /* Multiple operations to increase register pressure */
            int curr = row[c];
            int diff = curr - prev;
            int prod = diff * prev;
            int sum = prod + curr;
            int shifted = sum << 2;
            
            row[c] = shifted + prev;
            prev = curr + diff;
            
            /* Additional independent chains */
            static int chain = 0;
            chain = chain * 3 + row[c];
        }
        
        global_acc += prev;
    }
}

/* Test 6: PowerPC-specific patterns using double precision */
#ifdef __powerpc__
void test_powerpc_double(double *da, double *db, double *dc, int n) {
    double sum1 = da[0];
    double sum2 = db[0];
    double prod1 = dc[0];
    
    for (int i = 1; i < n; i++) {
        /* Double precision operations that use FP registers */
        sum1 = sum1 + da[i] * 1.41421356237;
        sum2 = sum2 - db[i] * 0.70710678118;
        prod1 = prod1 * 1.61803398875 + dc[i];
        
        /* Cross operations */
        double temp1 = sum1 * prod1;
        double temp2 = sum2 / prod1;
        double temp3 = temp1 + temp2;
        
        da[i] = temp1;
        db[i] = temp2;
        dc[i] = temp3;
    }
    
    global_acc += (long long)(sum1 + sum2 + prod1);
}
#endif

/* Test 7: Loop with manual unrolling hint */
#pragma GCC unroll 4
void test_unrolled_loop(int *a, int *b, int *c, int n) {
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    
    for (int i = 1; i < n; i++) {
        /* Manually unrolled style operations */
        acc1 = acc1 * 2 + a[i];
        acc2 = acc2 + b[i] * 3;
        acc3 = acc3 ^ (c[i] << 1);
        
        /* Interleaved operations */
        int t1 = acc1 + acc2;
        int t2 = acc2 + acc3;
        int t3 = acc3 + acc1;
        
        a[i] = t1;
        b[i] = t2;
        c[i] = t3;
        
        /* Additional independent operation */
        static int counter = 0;
        counter = counter * 5 + i;
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Main test driver */
int main() {
    /* Initialize data arrays */
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *data4 = malloc(SIZE * sizeof(int));
    float *fdata1 = malloc(SIZE * sizeof(float));
    float *fdata2 = malloc(SIZE * sizeof(float));
    float *fdata3 = malloc(SIZE * sizeof(float));
    short *sdata1 = malloc(SIZE * sizeof(short));
    short *sdata2 = malloc(SIZE * sizeof(short));
    int *idata1 = malloc(SIZE * sizeof(int));
    int *idata2 = malloc(SIZE * sizeof(int));
    int *matrix = malloc(100 * 100 * sizeof(int));
    
#ifdef __powerpc__
    double *ddata1 = malloc(SIZE * sizeof(double));
    double *ddata2 = malloc(SIZE * sizeof(double));
    double *ddata3 = malloc(SIZE * sizeof(double));
#endif
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        data4[i] = rand() % 100;
        fdata1[i] = (float)(rand() % 100) / 10.0f;
        fdata2[i] = (float)(rand() % 100) / 10.0f;
        fdata3[i] = (float)(rand() % 100) / 10.0f;
        sdata1[i] = (short)(rand() % 100);
        sdata2[i] = (short)(rand() % 100);
        idata1[i] = rand() % 100;
        idata2[i] = rand() % 100;
    }
    
    for (int i = 0; i < 100 * 100; i++) {
        matrix[i] = rand() % 100;
    }
    
#ifdef __powerpc__
    for (int i = 0; i < SIZE; i++) {
        ddata1[i] = (double)(rand() % 100) / 10.0;
        ddata2[i] = (double)(rand() % 100) / 10.0;
        ddata3[i] = (double)(rand() % 100) / 10.0;
    }
#endif
    
    /* Run tests multiple times to ensure hot loop execution */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multi_recurrence_int(data1, data2, data3, data4, SIZE);
        test_float_recurrence(fdata1, fdata2, fdata3, SIZE);
        test_pointer_chasing(data1, SIZE, 2);
        test_mixed_ops_variable_distance(sdata1, sdata2, idata1, idata2, SIZE);
        test_nested_loops(matrix, 100, 100);
        test_unrolled_loop(data1, data2, data3, SIZE);
        
#ifdef __powerpc__
        test_powerpc_double(ddata1, ddata2, ddata3, SIZE);
#endif
        
        /* Modify inputs slightly each iteration */
        data1[0] += iter;
        fdata1[0] += (float)iter / 100.0f;
    }
    
    /* Output result to prevent optimization */
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(sdata1);
    free(sdata2);
    free(idata1);
    free(idata2);
    free(matrix);
    
#ifdef __powerpc__
    free(ddata1);
    free(ddata2);
    free(ddata3);
#endif
    
    return 0;
}

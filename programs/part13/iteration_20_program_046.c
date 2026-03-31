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
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains */
        x = x * 3 + a[i];          /* Chain 1: distance 1 */
        y = y + x * 2 - b[i];      /* Chain 2: depends on x */
        z = z * 5 + y + c[i];      /* Chain 3: depends on y */
        w = w * 7 + z - d[i];      /* Chain 4: depends on z */
        v = v * 11 + w + a[i] * b[i]; /* Chain 5: depends on w */
        
        /* Cross-chain operations to increase pressure */
        a[i] = x + y;
        b[i] = z - w;
        c[i] = v * 2;
        d[i] = (x << 2) | (y & 0xFF);
    }
    
    global_sum += x + y + z + w + v;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *fa, float *fb, float *fc, int n) {
    float sum1 = 0.1f, sum2 = 0.2f, sum3 = 0.3f;
    float prod1 = 1.0f, prod2 = 1.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01f + fa[i];           /* Distance 1 */
        sum2 = sum2 * 1.02f + fb[i] - sum1;    /* Depends on sum1 */
        sum3 = sum3 * 1.03f + fc[i] + sum2;    /* Depends on sum2 */
        
        prod1 = prod1 * (1.0f + fa[i] * 0.001f);
        prod2 = prod2 * (1.0f - fb[i] * 0.001f) + sum3;
        
        /* Store results to create anti-dependencies */
        fa[i] = sum1 + sum2;
        fb[i] = sum3 * prod1;
        fc[i] = prod2 - sum1;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer-based recurrences */
        acc1 = acc1 * 3 + *ptr1;
        acc2 = acc2 * 5 + *ptr2 - acc1;
        acc3 = acc3 * 7 + *ptr3 + acc2;
        
        /* Update pointers with different strides */
        *ptr1 = acc1 & 0xFF;
        *ptr2 = acc2 | 0x100;
        *ptr3 = acc3 ^ 0x200;
        
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with manual unrolling */
void test_mixed_ops_unrolled(short *s1, short *s2, int *i1, int *i2, int n) {
    int sum_h = 0, sum_l = 0;
    int prod_h = 1, prod_l = 1;
    
    /* Manual unrolling to increase register pressure */
    for (int i = 0; i < n - 3; i += 4) {
        /* First unrolled iteration */
        sum_h = sum_h * 2 + s1[i];
        sum_l = sum_l * 3 + s2[i];
        prod_h = prod_h * (1 + (sum_h & 0xF));
        prod_l = prod_l * (1 + (sum_l >> 4));
        
        /* Second iteration with cross dependencies */
        sum_h = sum_h + i1[i] - prod_l;
        sum_l = sum_l + i2[i] ^ prod_h;
        prod_h = (prod_h << 1) | (sum_h & 1);
        prod_l = (prod_l >> 1) ^ (sum_l & 1);
        
        /* Third iteration */
        sum_h = sum_h * 5 + s1[i+2];
        sum_l = sum_l * 7 + s2[i+2];
        prod_h = prod_h + (sum_h % 256);
        prod_l = prod_l - (sum_l % 128);
        
        /* Fourth iteration */
        sum_h = (sum_h & i1[i+3]) | sum_l;
        sum_l = (sum_l ^ i2[i+3]) & sum_h;
        prod_h = prod_h * 11 + (sum_h >> 8);
        prod_l = prod_l * 13 + (sum_l << 8);
        
        /* Store results creating anti-deps */
        s1[i] = sum_h & 0xFFFF;
        s2[i] = sum_l & 0xFFFF;
        i1[i] = prod_h;
        i2[i] = prod_l;
    }
    
    global_sum += sum_h + sum_l + prod_h + prod_l;
}

/* Test 5: Double precision operations for PowerPC targeting */
void test_double_accumulate(double *da, double *db, double *dc, int n) {
    double acc1 = 0.1, acc2 = 0.2, acc3 = 0.3;
    double tmp1 = 1.0, tmp2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double precision dependency chains */
        acc1 = acc1 * 1.0001 + da[i];
        acc2 = acc2 * 1.0002 + db[i] * acc1;
        acc3 = acc3 * 1.0003 + dc[i] + acc2;
        
        tmp1 = tmp1 * (1.0 + da[i] * 0.00001);
        tmp2 = tmp2 / (1.0 + db[i] * 0.00002) - acc3;
        
        /* FMA-like operations */
        da[i] = acc1 * 2.0 + tmp1;
        db[i] = acc2 * 3.0 - tmp2;
        dc[i] = acc3 * 4.0 * tmp1;
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3 + tmp1 + tmp2);
}

/* Test 6: Vector-style access pattern for RISC-V V extension */
void test_vector_style(int *in1, int *in2, int *out1, int *out2, int n, int stride) {
    int sum1 = 0, sum2 = 0;
    int prod1 = 1, prod2 = 1;
    
    for (int i = 0; i < n; i += stride) {
        for (int j = 0; j < stride && (i + j) < n; j++) {
            /* Nested loops to encourage software pipelining */
            int idx = i + j;
            sum1 = sum1 * 2 + in1[idx];
            sum2 = sum2 * 3 + in2[idx] ^ sum1;
            
            prod1 = prod1 * (1 + (sum1 & 0xFF));
            prod2 = prod2 * (1 + (sum2 >> 8)) - prod1;
            
            out1[idx] = sum1 + prod2;
            out2[idx] = sum2 * prod1;
        }
    }
    
    global_sum += sum1 + sum2 + prod1 + prod2;
}

/* Test 7: Complex recurrence with varying distances */
void test_variable_distance(int *arr, int *brr, int n) {
    int hist[4] = {0, 0, 0, 0};  /* History buffer */
    
    for (int i = 0; i < n; i++) {
        /* Use history with different distances */
        int val = arr[i] + brr[i];
        
        /* Recurrence with distance 1 */
        hist[0] = hist[0] * 3 + val;
        
        /* Recurrence with effective distance 2 */
        hist[1] = hist[1] + hist[0] * 2 - val;
        
        /* Recurrence with distance 3 */
        hist[2] = hist[2] ^ hist[1] + val;
        
        /* Recurrence with distance 4 */
        hist[3] = (hist[3] & hist[2]) | val;
        
        /* Rotate history */
        arr[i] = hist[0];
        brr[i] = hist[1] + hist[2] + hist[3];
        
        /* Shift history */
        hist[3] = hist[2];
        hist[2] = hist[1];
        hist[1] = hist[0];
    }
    
    for (int i = 0; i < 4; i++) {
        global_sum += hist[i];
    }
}

int main() {
    /* Allocate and initialize test arrays */
    int *a1 = malloc(SIZE * sizeof(int));
    int *a2 = malloc(SIZE * sizeof(int));
    int *a3 = malloc(SIZE * sizeof(int));
    int *a4 = malloc(SIZE * sizeof(int));
    
    float *f1 = malloc(SIZE * sizeof(float));
    float *f2 = malloc(SIZE * sizeof(float));
    float *f3 = malloc(SIZE * sizeof(float));
    
    short *s1 = malloc(SIZE * sizeof(short));
    short *s2 = malloc(SIZE * sizeof(short));
    
    double *d1 = malloc(SIZE * sizeof(double));
    double *d2 = malloc(SIZE * sizeof(double));
    double *d3 = malloc(SIZE * sizeof(double));
    
    int *v1 = malloc(SIZE * sizeof(int));
    int *v2 = malloc(SIZE * sizeof(int));
    int *v3 = malloc(SIZE * sizeof(int));
    int *v4 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pattern data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a1[i] = rand() % 100;
        a2[i] = rand() % 100;
        a3[i] = rand() % 100;
        a4[i] = rand() % 100;
        
        f1[i] = (rand() % 100) / 100.0f;
        f2[i] = (rand() % 100) / 100.0f;
        f3[i] = (rand() % 100) / 100.0f;
        
        s1[i] = rand() % 32768;
        s2[i] = rand() % 32768;
        
        d1[i] = (rand() % 100) / 100.0;
        d2[i] = (rand() % 100) / 100.0;
        d3[i] = (rand() % 100) / 100.0;
        
        v1[i] = rand() % 1000;
        v2[i] = rand() % 1000;
        v3[i] = rand() % 1000;
        v4[i] = rand() % 1000;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_multi_recurrence_int(a1, a2, a3, a4, SIZE);
        test_float_accumulate(f1, f2, f3, SIZE);
        test_pointer_chasing(a1, SIZE / 4, 4);
        test_mixed_ops_unrolled(s1, s2, a3, a4, SIZE);
        test_double_accumulate(d1, d2, d3, SIZE);
        test_vector_style(v1, v2, v3, v4, SIZE, 4);
        test_variable_distance(a1, a2, SIZE);
        
        /* Modify inputs slightly each iteration */
        if (iter % 100 == 0) {
            a1[rand() % SIZE] = rand() % 100;
            f1[rand() % SIZE] = (rand() % 100) / 100.0f;
        }
    }
    
    /* Output result to prevent optimization */
    printf("Final result: %lld\n", global_sum);
    
    /* Cleanup */
    free(a1); free(a2); free(a3); free(a4);
    free(f1); free(f2); free(f3);
    free(s1); free(s2);
    free(d1); free(d2); free(d3);
    free(v1); free(v2); free(v3); free(v4);
    
    return 0;
}

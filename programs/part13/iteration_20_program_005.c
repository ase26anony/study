/* test_modulo_sched.c
 * Test program to cover register move scheduling in GCC's modulo scheduler
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve -ftree-vectorize test_modulo_sched.c -o test_modulo_sched
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_int_recurrence_chains(int *a, int *b, int *c, int *d, int n) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains to create register pressure */
        x1 = x1 * 13 + b[i] * 17;
        x2 = x2 * 19 + c[i] * 23;
        x3 = x3 * 29 + d[i] * 31;
        x4 = x4 * 37 + a[i] * 41;
        
        y1 = y1 * 43 + x1 * 47;
        y2 = y2 * 53 + x2 * 59;
        y3 = y3 * 61 + x3 * 67;
        y4 = y4 * 71 + x4 * 73;
        
        a[i] = x1 + y1;
        b[i] = x2 + y2;
        c[i] = x3 + y3;
        d[i] = x4 + y4;
    }
    
    global_sum += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(float *fa, float *fb, float *fc, float *fd, int n) {
    float acc1 = 1.0f, acc2 = 2.0f, acc3 = 3.0f, acc4 = 4.0f;
    float tmp1 = 0.5f, tmp2 = 1.5f, tmp3 = 2.5f, tmp4 = 3.5f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        acc1 = acc1 * 1.01f + fa[i] * tmp1;
        acc2 = acc2 * 1.02f + fb[i] * tmp2;
        acc3 = acc3 * 1.03f + fc[i] * tmp3;
        acc4 = acc4 * 1.04f + fd[i] * tmp4;
        
        /* Cross-chain dependencies to increase pressure */
        tmp1 = tmp1 * 0.99f + acc2 * 0.1f;
        tmp2 = tmp2 * 0.98f + acc3 * 0.2f;
        tmp3 = tmp3 * 0.97f + acc4 * 0.3f;
        tmp4 = tmp4 * 0.96f + acc1 * 0.4f;
        
        fa[i] = acc1 + tmp1;
        fb[i] = acc2 + tmp2;
        fc[i] = acc3 + tmp3;
        fd[i] = acc4 + tmp4;
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3 + acc4 + tmp1 + tmp2 + tmp3 + tmp4);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2 * stride;
    int *ptr4 = arr + 3 * stride;
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int prod1 = 1, prod2 = 1, prod3 = 1, prod4 = 1;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer-based recurrence chains */
        sum1 = sum1 + *ptr1 * prod1;
        sum2 = sum2 + *ptr2 * prod2;
        sum3 = sum3 + *ptr3 * prod3;
        sum4 = sum4 + *ptr4 * prod4;
        
        prod1 = prod1 * 3 + sum2;
        prod2 = prod2 * 5 + sum3;
        prod3 = prod3 * 7 + sum4;
        prod4 = prod4 * 11 + sum1;
        
        *ptr1 = sum1;
        *ptr2 = sum2;
        *ptr3 = sum3;
        *ptr4 = sum4;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        ptr4 += stride;
    }
    
    global_sum += sum1 + sum2 + sum3 + sum4 + prod1 + prod2 + prod3 + prod4;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_operations(short *sa, int *ia, long long *lla, int n) {
    short s1 = 1, s2 = 2;
    int i1 = 3, i2 = 4;
    long long ll1 = 5, ll2 = 6;
    
    for (int i = 0; i < n; i++) {
        /* Operations on different-sized types to use multiple register types */
        s1 = s1 * 2 + sa[i];
        s2 = s2 * 3 + s1;
        
        i1 = i1 * 5 + ia[i];
        i2 = i2 * 7 + i1;
        
        ll1 = ll1 * 11 + lla[i];
        ll2 = ll2 * 13 + ll1;
        
        /* Cross-type dependencies */
        sa[i] = s1 + (short)i1;
        ia[i] = i2 + (int)ll1;
        lla[i] = ll2 + (long long)s2;
    }
    
    global_sum += s1 + s2 + i1 + i2 + ll1 + ll2;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int r = 0; r < rows; r++) {
        int *row = mat + r * cols;
        int prev = r;
        int curr = 0;
        
        /* Innermost loop with carried dependency */
        for (int c = 0; c < cols; c++) {
            curr = prev * 17 + row[c] * 19;
            row[c] = curr;
            prev = curr;
            
            /* Additional operations to increase register pressure */
            int tmp1 = curr * 23;
            int tmp2 = tmp1 * 29;
            int tmp3 = tmp2 * 31;
            row[c] += tmp3;
        }
    }
}

/* Test 6: PowerPC-specific double operations */
#ifdef __powerpc__
void test_powerpc_double(double *da, double *db, double *dc, int n) {
    double d1 = 1.0, d2 = 2.0, d3 = 3.0, d4 = 4.0;
    double t1 = 0.1, t2 = 0.2, t3 = 0.3, t4 = 0.4;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double-precision FP chains */
        d1 = d1 * 1.1 + da[i] * t1;
        d2 = d2 * 1.2 + db[i] * t2;
        d3 = d3 * 1.3 + dc[i] * t3;
        d4 = d4 * 1.4 + da[i] * t4;
        
        t1 = t1 * 0.9 + d2 * 0.01;
        t2 = t2 * 0.8 + d3 * 0.02;
        t3 = t3 * 0.7 + d4 * 0.03;
        t4 = t4 * 0.6 + d1 * 0.04;
        
        da[i] = d1 + t1;
        db[i] = d2 + t2;
        dc[i] = d3 + t3;
    }
    
    global_sum += (long long)(d1 + d2 + d3 + d4 + t1 + t2 + t3 + t4);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_manual_unroll(int *a, int *b, int n) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4;
    int y1 = 5, y2 = 6, y3 = 7, y4 = 8;
    
    for (int i = 0; i < n; i++) {
        /* Manually unrolled operations */
        x1 = x1 * 2 + a[i];
        y1 = y1 * 3 + x1;
        a[i] = x1 + y1;
        
        x2 = x2 * 5 + b[i];
        y2 = y2 * 7 + x2;
        b[i] = x2 + y2;
        
        x3 = x3 * 11 + a[i];
        y3 = y3 * 13 + x3;
        a[i] += x3 + y3;
        
        x4 = x4 * 17 + b[i];
        y4 = y4 * 19 + x4;
        b[i] += x4 + y4;
    }
    
    global_sum += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

int main() {
    /* Initialize arrays with test data */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    float *fc = malloc(SIZE * sizeof(float));
    float *fd = malloc(SIZE * sizeof(float));
    
    short *sa = malloc(SIZE * sizeof(short));
    int *ia = malloc(SIZE * sizeof(int));
    long long *lla = malloc(SIZE * sizeof(long long));
    
    int *mat = malloc(100 * 100 * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        
        fa[i] = (float)(rand() % 100) / 10.0f;
        fb[i] = (float)(rand() % 100) / 10.0f;
        fc[i] = (float)(rand() % 100) / 10.0f;
        fd[i] = (float)(rand() % 100) / 10.0f;
        
        sa[i] = (short)(rand() % 100);
        ia[i] = rand() % 100;
        lla[i] = rand() % 100;
    }
    
    for (int i = 0; i < 100 * 100; i++) {
        mat[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_recurrence_chains(a, b, c, d, SIZE);
        test_float_accumulation(fa, fb, fc, fd, SIZE);
        test_pointer_chasing(a, SIZE, 4);
        test_mixed_operations(sa, ia, lla, SIZE);
        test_nested_loops(mat, 100, 100);
        test_manual_unroll(a, b, SIZE);
        
        #ifdef __powerpc__
        double *da = malloc(SIZE * sizeof(double));
        double *db = malloc(SIZE * sizeof(double));
        double *dc = malloc(SIZE * sizeof(double));
        for (int i = 0; i < SIZE; i++) {
            da[i] = (double)(rand() % 100) / 10.0;
            db[i] = (double)(rand() % 100) / 10.0;
            dc[i] = (double)(rand() % 100) / 10.0;
        }
        test_powerpc_double(da, db, dc, SIZE);
        free(da); free(db); free(dc);
        #endif
    }
    
    printf("Tests completed. Global sum: %lld\n", global_sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(fa); free(fb); free(fc); free(fd);
    free(sa); free(ia); free(lla);
    free(mat);
    
    return 0;
}

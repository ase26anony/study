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
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    /* Multiple independent dependency chains with distance 1 */
    for (int i = 2; i < n; i++) {
        /* Chain 1: a[i] depends on a[i-1] and a[i-2] */
        x1 = x1 + y1 * 3;
        y1 = a[i] + x1;
        a[i] = y1 - x1 / 2;
        
        /* Chain 2: b[i] depends on b[i-1] with different operations */
        x2 = x2 ^ (y2 << 2);
        y2 = b[i] | x2;
        b[i] = y2 & 0x7FFFFFFF;
        
        /* Chain 3: c[i] depends on c[i-1] with arithmetic */
        x3 = x3 * 5 + y3;
        y3 = c[i] - x3;
        c[i] = y3 * 3;
        
        /* Chain 4: d[i] depends on d[i-1] with shifts */
        x4 = (x4 >> 3) | (y4 << 5);
        y4 = d[i] ^ x4;
        d[i] = y4 + i;
    }
    
    /* Use results to prevent optimization */
    global_acc += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(double *f1, double *f2, double *f3, int n) {
    double acc1 = f1[0], acc2 = f2[0], acc3 = f3[0];
    double tmp1 = f1[1], tmp2 = f2[1], tmp3 = f3[1];
    
    /* Multiple FP dependency chains */
    for (int i = 2; i < n; i++) {
        /* FP chain 1: linear recurrence */
        acc1 = acc1 * 1.01 + tmp1;
        tmp1 = f1[i] * 0.5;
        f1[i] = acc1 + tmp1;
        
        /* FP chain 2: different coefficients */
        acc2 = acc2 * 0.99 - tmp2;
        tmp2 = f2[i] * 1.5;
        f2[i] = acc2 * tmp2;
        
        /* FP chain 3: mixed operations */
        acc3 = acc3 / 1.03 + tmp3 * 2.0;
        tmp3 = f3[i] + 1.0;
        f3[i] = acc3 - tmp3;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int val1 = *ptr1, val2 = *ptr2, val3 = *ptr3;
    
    while (ptr3 < end) {
        /* Multiple pointer chasing chains */
        sum1 = sum1 + val1 * 2;
        val1 = *ptr1;
        ptr1 += stride;
        
        sum2 = sum2 ^ (val2 << 1);
        val2 = *ptr2;
        ptr2 += stride;
        
        sum3 = sum3 | (val3 >> 2);
        val3 = *ptr3;
        ptr3 += stride;
        
        /* Cross-chain dependencies to increase pressure */
        sum1 = sum1 - sum3;
        sum2 = sum2 ^ sum1;
        sum3 = sum3 + sum2;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer/float with unrolling hint */
void test_mixed_unrolled(float *fa, int *ia, double *da, int n) {
    float f_acc = fa[0];
    int i_acc = ia[0];
    double d_acc = da[0];
    
    /* Manual unrolling to increase operations per iteration */
    for (int i = 1; i < n - 3; i += 4) {
        /* Unrolled iteration 1 */
        f_acc = f_acc * 1.1f + fa[i];
        i_acc = i_acc + (ia[i] << 1);
        d_acc = d_acc * 0.9 + da[i];
        
        /* Unrolled iteration 2 */
        f_acc = f_acc - fa[i+1] * 0.5f;
        i_acc = i_acc ^ (ia[i+1] >> 2);
        d_acc = d_acc + da[i+1] * 1.1;
        
        /* Unrolled iteration 3 */
        f_acc = f_acc + fa[i+2] * 2.0f;
        i_acc = i_acc | (ia[i+2] << 3);
        d_acc = d_acc - da[i+2] / 1.5;
        
        /* Unrolled iteration 4 */
        f_acc = f_acc / 1.05f - fa[i+3];
        i_acc = i_acc & (ia[i+3] | 0xFF);
        d_acc = d_acc * 1.01 + da[i+3];
        
        /* Store results to create dependencies */
        fa[i] = f_acc;
        ia[i] = i_acc;
        da[i] = d_acc;
    }
    
    global_acc += (long long)(f_acc + d_acc) + i_acc;
}

/* Test 5: PowerPC-specific double operations */
#ifdef __powerpc__
void test_powerpc_double(double *d1, double *d2, double *d3, int n) {
    double acc1 = d1[0], acc2 = d2[0], acc3 = d3[0];
    double tmp1 = d1[1], tmp2 = d2[1], tmp3 = d3[1];
    
    /* PowerPC often has more FP registers, use many double ops */
    for (int i = 2; i < n; i++) {
        /* Multiple double precision chains */
        acc1 = __builtin_fma(acc1, 1.01, tmp1);
        tmp1 = d1[i] * 3.14159;
        d1[i] = acc1 + tmp1;
        
        acc2 = __builtin_fma(acc2, 0.99, tmp2 * 2.0);
        tmp2 = d2[i] / 2.71828;
        d2[i] = acc2 - tmp2;
        
        acc3 = acc3 * 0.95 + tmp3;
        tmp3 = d3[i] * d3[i-1];
        d3[i] = acc3 * tmp3;
        
        /* Additional operations to increase register pressure */
        double t1 = acc1 * acc2;
        double t2 = acc2 * acc3;
        double t3 = acc3 * acc1;
        acc1 += t1;
        acc2 += t2;
        acc3 += t3;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3);
}
#endif

/* Test 6: Variable distance dependencies */
void test_variable_distance(int *arr, int *idx, int n) {
    int sum1 = arr[0], sum2 = arr[1], sum3 = arr[2];
    int pos1 = 0, pos2 = 1, pos3 = 2;
    
    for (int i = 3; i < n; i++) {
        /* Variable dependency distances based on index array */
        int dist1 = idx[i] % 3 + 1;
        int dist2 = idx[i+1] % 3 + 1;
        int dist3 = idx[i+2] % 3 + 1;
        
        sum1 = sum1 + arr[pos1] * dist1;
        pos1 = (pos1 + dist1) % n;
        
        sum2 = sum2 ^ (arr[pos2] << dist2);
        pos2 = (pos2 + dist2) % n;
        
        sum3 = sum3 | (arr[pos3] >> dist3);
        pos3 = (pos3 + dist3) % n;
        
        /* Cross dependencies */
        arr[i] = sum1 + sum2 + sum3;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, int *c, int *d, 
                 double *f1, double *f2, double *f3,
                 float *fa, int *ia, double *da,
                 int *idx, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        c[i] = rand() % 1000;
        d[i] = rand() % 1000;
        
        f1[i] = (double)(rand() % 1000) / 10.0;
        f2[i] = (double)(rand() % 1000) / 10.0;
        f3[i] = (double)(rand() % 1000) / 10.0;
        
        fa[i] = (float)(rand() % 1000) / 10.0f;
        ia[i] = rand() % 1000;
        da[i] = (double)(rand() % 1000) / 10.0;
        
        idx[i] = rand() % n;
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    
    double *f1 = malloc(SIZE * sizeof(double));
    double *f2 = malloc(SIZE * sizeof(double));
    double *f3 = malloc(SIZE * sizeof(double));
    
    float *fa = malloc(SIZE * sizeof(float));
    int *ia = malloc(SIZE * sizeof(int));
    double *da = malloc(SIZE * sizeof(double));
    
    int *idx = malloc(SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        init_arrays(a, b, c, d, f1, f2, f3, fa, ia, da, idx, SIZE);
        
        /* Test 1: Multiple integer recurrence chains */
        test_multi_recurrence_int(a, b, c, d, SIZE);
        
        /* Test 2: Floating-point accumulation */
        test_float_recurrence(f1, f2, f3, SIZE);
        
        /* Test 3: Pointer chasing */
        test_pointer_chasing(a, SIZE, 4);
        
        /* Test 4: Mixed with unrolling */
        test_mixed_unrolled(fa, ia, da, SIZE);
        
        /* Test 5: PowerPC-specific if enabled */
        #ifdef __powerpc__
        test_powerpc_double(f1, f2, f3, SIZE);
        #endif
        
        /* Test 6: Variable distance */
        test_variable_distance(a, idx, SIZE);
    }
    
    /* Output result to prevent optimization */
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(f1); free(f2); free(f3);
    free(fa); free(ia); free(da);
    free(idx);
    
    return 0;
}

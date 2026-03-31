/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Targets lines 596-606 in modulo-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        x = x * 3 + a[i];          /* Chain 1: distance 1 dependency */
        y = y + x * 2 - b[i];      /* Chain 2: uses x from current iteration */
        z = z * 5 + y + c[i];      /* Chain 3: uses y from current iteration */
        w = w * 7 - z + a[i];      /* Chain 4: uses z from current iteration */
        v = v * 11 + w * b[i];     /* Chain 5: uses w from current iteration */
        u = u * 13 + v - c[i];     /* Chain 6: uses v from current iteration */
        
        /* Store results to create memory pressure */
        a[i] = x;
        b[i] = y;
        c[i] = z;
    }
    
    global_acc += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = 0.1, sum2 = 0.2, sum3 = 0.3;
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i] * b[i];      /* Distance 1 */
        sum2 = sum2 * 0.99 + b[i] * c[i];      /* Distance 1 */
        sum3 = sum3 * 1.02 + a[i] * c[i];      /* Distance 1 */
        
        /* Cross-iteration dependencies with distance 2 */
        if (i >= 2) {
            prod1 = prod1 * (1.0 + a[i] * a[i-2]);
            prod2 = prod2 * (1.0 + b[i] * b[i-2]);
        }
        
        /* Additional operations to increase register pressure */
        a[i] = sum1 + sum2 + sum3;
        b[i] = prod1 - prod2;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer-based recurrence chains */
        acc1 = acc1 * 2 + *ptr1;
        acc2 = acc2 * 3 + *ptr2;
        acc3 = acc3 * 5 + *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation to prevent optimization */
        *ptr1 = acc1 & 0xFF;
        *ptr2 = acc2 & 0xFF;
        *ptr3 = acc3 & 0xFF;
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with manual unrolling */
#pragma GCC unroll 4
void test_mixed_unrolled(int *a, int *b, int *c, int n) {
    int x0 = 1, x1 = 2, x2 = 3, x3 = 4;
    int y0 = 5, y1 = 6, y2 = 7, y3 = 8;
    
    for (int i = 0; i < n; i += 4) {
        /* Unrolled operations creating complex dependency web */
        x0 = x0 * 3 + a[i]   - y3;  /* Uses y3 from previous iteration group */
        y0 = y0 * 7 + b[i]   ^ x0;
        
        x1 = x1 * 5 + a[i+1] - y0;
        y1 = y1 * 11 + b[i+1] ^ x1;
        
        x2 = x2 * 13 + a[i+2] - y1;
        y2 = y2 * 17 + b[i+2] ^ x2;
        
        x3 = x3 * 19 + a[i+3] - y2;
        y3 = y3 * 23 + b[i+3] ^ x3;
        
        /* Store with dependencies */
        c[i]   = x0 + y0;
        c[i+1] = x1 + y1;
        c[i+2] = x2 + y2;
        c[i+3] = x3 + y3;
    }
    
    global_acc += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
}

/* Test 5: PowerPC-specific double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    double d1 = 0.0, d2 = 0.0, d3 = 0.0, d4 = 0.0;
    double d5 = 0.0, d6 = 0.0, d7 = 0.0, d8 = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Many double operations to use FP registers */
        d1 = d1 * 1.1 + a[i];
        d2 = d2 * 1.2 + b[i];
        d3 = d3 * 1.3 + d1 * d2;
        d4 = d4 * 1.4 + d3 / (a[i] + 1.0);
        d5 = d5 * 1.5 + d4 * d3;
        d6 = d6 * 1.6 + d5 - d2;
        d7 = d7 * 1.7 + d6 * d4;
        d8 = d8 * 1.8 + d7 / d5;
        
        /* Store results */
        a[i] = d1 + d3 + d5 + d7;
        b[i] = d2 + d4 + d6 + d8;
    }
    
    global_acc += (long long)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
}
#endif

/* Test 6: Compile-time unknown bounds to encourage vectorization */
void test_variable_bounds(int *a, int *b, int *c, int n) {
    int x = 1, y = 1, z = 1;
    
    /* Loop with runtime-determined bounds */
    for (int i = 0; i < n; i++) {
        /* Complex dependency chain */
        x = (x * 1103515245 + 12345) & 0x7fffffff;
        y = y ^ (x + a[i]);
        z = z * 3 + (y >> 16);
        
        /* Cross-iteration store/load */
        b[i] = z;
        if (i > 0) {
            c[i] = c[i-1] + b[i] * 2;
        } else {
            c[i] = b[i];
        }
        
        /* Additional dependency */
        a[i] = a[i] + x - y + z;
    }
    
    global_acc += x + y + z;
}

/* Test 7: Nested loops with hot inner loop */
void test_nested_loops(int *a, int *b, int n, int m) {
    for (int j = 0; j < m; j++) {
        int acc = j;
        /* Hot inner loop with carried dependency */
        for (int i = 0; i < n; i++) {
            acc = acc * 3 + a[i] - b[i];
            a[i] = acc & 0xFF;
            b[i] = (acc >> 8) & 0xFF;
        }
        global_acc += acc;
    }
}

/* Main driver with multiple test iterations */
int main() {
    const int N = 1024;
    const int M = 100;
    const int ITERS = 10000;
    
    /* Allocate and initialize arrays */
    int *a_int = malloc(N * sizeof(int));
    int *b_int = malloc(N * sizeof(int));
    int *c_int = malloc(N * sizeof(int));
    double *a_dbl = malloc(N * sizeof(double));
    double *b_dbl = malloc(N * sizeof(double));
    double *c_dbl = malloc(N * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < N; i++) {
        a_int[i] = rand() % 100;
        b_int[i] = rand() % 100;
        c_int[i] = rand() % 100;
        a_dbl[i] = (double)(rand() % 100) / 10.0;
        b_dbl[i] = (double)(rand() % 100) / 10.0;
        c_dbl[i] = (double)(rand() % 100) / 10.0;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_int_recurrence_multi_chain(a_int, b_int, c_int, N);
        test_float_accumulate(a_dbl, b_dbl, c_dbl, N);
        test_pointer_chasing(a_int, N, 8);
        test_mixed_unrolled(a_int, b_int, c_int, N);
        
        #ifdef __powerpc__
        test_powerpc_double(a_dbl, b_dbl, N);
        #endif
        
        test_variable_bounds(a_int, b_int, c_int, N);
        test_nested_loops(a_int, b_int, N, 4);
        
        /* Modify inputs slightly each iteration */
        a_int[iter % N] ^= iter;
        b_dbl[iter % N] += 0.1;
    }
    
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(a_int);
    free(b_int);
    free(c_int);
    free(a_dbl);
    free(b_dbl);
    free(c_dbl);
    
    return 0;
}

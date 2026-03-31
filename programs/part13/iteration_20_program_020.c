/* test-modulo-sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Volatile to prevent optimization */
volatile long global_sum = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5;
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains */
        x = x * 3 + a[i];      /* Chain 1: distance 1 */
        y = y + b[i] * x;      /* Chain 2: depends on x from same iteration */
        z = z * 2 - c[i] + y;  /* Chain 3: depends on y */
        w = w + x * y - z;     /* Chain 4: complex dependency */
        v = v * 7 + w;         /* Chain 5: distance 1 */
        
        /* Store results to create register pressure */
        a[i] = x;
        b[i] = y;
        c[i] = z;
    }
    
    global_sum += x + y + z + w + v;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];          /* Distance 1 recurrence */
        sum2 = sum2 + b[i] * sum1;          /* Depends on sum1 */
        sum3 = sum3 - c[i] / (sum2 + 1.0);  /* Complex FP chain */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 * (1.0 - b[i] * 0.001);
        
        /* Cross-chain dependencies */
        sum1 = sum1 + prod1 * 0.5;
        sum2 = sum2 - prod2 * 0.3;
    }
    
    global_sum += (long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    while (ptr1 < end && ptr2 < end && ptr3 < end) {
        /* Multiple pointer chains with carried dependencies */
        acc1 = acc1 + *ptr1 + tmp1;
        tmp1 = acc1 >> 1;
        
        acc2 = acc2 * 2 + *ptr2 - tmp2;
        tmp2 = acc2 & 0xFF;
        
        acc3 = acc3 + (*ptr3 ^ tmp3);
        tmp3 = acc3 * 3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Additional operations to increase register pressure */
        acc1 = acc1 ^ (acc2 & acc3);
        acc2 = acc2 | (acc1 ^ acc3);
        acc3 = acc3 & (acc1 | acc2);
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with manual unrolling */
#pragma GCC unroll 4
void test_mixed_ops_unrolled(int *a, int *b, int *c, int n) {
    int x0 = a[0], x1 = b[0], x2 = c[0];
    int y0 = 1, y1 = 2, y2 = 3, y3 = 4;
    
    for (int i = 1; i < n - 3; i += 4) {
        /* Unrolled operations with inter-iteration dependencies */
        x0 = x0 * 3 + a[i];
        x1 = x1 + b[i] * x0;
        x2 = x2 - c[i] + x1;
        y0 = y0 + x0 * x1 - x2;
        
        x0 = x0 * 5 + a[i+1];
        x1 = x1 + b[i+1] * x0;
        x2 = x2 - c[i+1] + x1;
        y1 = y1 + x0 * x1 - x2;
        
        x0 = x0 * 7 + a[i+2];
        x1 = x1 + b[i+2] * x0;
        x2 = x2 - c[i+2] + x1;
        y2 = y2 + x0 * x1 - x2;
        
        x0 = x0 * 11 + a[i+3];
        x1 = x1 + b[i+3] * x0;
        x2 = x2 - c[i+3] + x1;
        y3 = y3 + x0 * x1 - x2;
        
        /* Additional register pressure */
        a[i] = x0 + y0;
        b[i] = x1 + y1;
        c[i] = x2 + y2;
    }
    
    global_sum += x0 + x1 + x2 + y0 + y1 + y2 + y3;
}

/* Test 5: PowerPC-specific double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    double sum0 = a[0], sum1 = b[0];
    double prod0 = 1.0, prod1 = 1.0;
    double tmp0 = 0.0, tmp1 = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple double precision chains */
        tmp0 = sum0 * 1.5;
        sum0 = tmp0 + a[i] * prod0;
        prod0 = prod0 * (1.0 + a[i] * 0.01);
        
        tmp1 = sum1 * 0.75;
        sum1 = tmp1 + b[i] * prod1;
        prod1 = prod1 * (1.0 - b[i] * 0.01);
        
        /* Cross dependencies */
        sum0 = sum0 + sum1 * 0.25;
        sum1 = sum1 - sum0 * 0.125;
    }
    
    global_sum += (long)(sum0 + sum1 + prod0 + prod1);
}
#endif

/* Test 6: Array accumulation with variable distance */
void test_variable_distance(int *a, int *b, int *c, int n, int dist) {
    int acc[5] = {0};
    
    for (int i = dist; i < n; i++) {
        /* Dependency with variable distance */
        acc[0] = acc[0] + a[i] * a[i-dist];
        acc[1] = acc[1] + b[i] * b[i-dist] - acc[0];
        acc[2] = acc[2] * 2 + c[i] / (acc[1] + 1);
        acc[3] = acc[3] + acc[0] * acc[1] - acc[2];
        acc[4] = acc[4] ^ (acc[3] * acc[2]);
        
        /* Additional operations */
        a[i] = acc[0] + acc[4];
        b[i] = acc[1] - acc[3];
        c[i] = acc[2] * acc[4];
    }
    
    for (int i = 0; i < 5; i++) {
        global_sum += acc[i];
    }
}

/* Main test driver */
int main() {
    /* Allocate and initialize arrays */
    int *int_data1 = malloc(SIZE * sizeof(int));
    int *int_data2 = malloc(SIZE * sizeof(int));
    int *int_data3 = malloc(SIZE * sizeof(int));
    double *double_data1 = malloc(SIZE * sizeof(double));
    double *double_data2 = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        int_data1[i] = rand() % 100;
        int_data2[i] = rand() % 100;
        int_data3[i] = rand() % 100;
        double_data1[i] = (double)(rand() % 100) / 10.0;
        double_data2[i] = (double)(rand() % 100) / 10.0;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_int_recurrence_multi_chain(int_data1, int_data2, int_data3, SIZE);
        test_float_accumulate(double_data1, double_data2, double_data1, SIZE);
        test_pointer_chasing(int_data1, SIZE, 4);
        test_mixed_ops_unrolled(int_data1, int_data2, int_data3, SIZE);
        test_variable_distance(int_data1, int_data2, int_data3, SIZE, 2);
        
        #ifdef __powerpc__
        test_powerpc_double(double_data1, double_data2, SIZE);
        #endif
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %ld\n", global_sum);
    
    /* Cleanup */
    free(int_data1);
    free(int_data2);
    free(int_data3);
    free(double_data1);
    free(double_data2);
    
    return 0;
}

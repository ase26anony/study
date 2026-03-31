/* test_modulo_sched.c
 * 
 * Test program to trigger GCC's modulo scheduler register move logic.
 * Targets uncovered lines in modulo-sched.cc: schedule_reg_move() function.
 * 
 * Compilation options for coverage:
 *   PowerPC: -O3 -mtune=powerpc -mcpu=power8 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 *   ARM SVE: -O3 -march=armv8-a+sve -fdump-rtl-sms -fmodulo-sched -ftree-vectorize
 *   Generic: -O2 -fdump-rtl-sms -fmodulo-sched -da
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        a[i] = a[i-1] + b[i] * c[i];          /* Chain 1: distance 1 */
        x = x * 13 + b[i];                    /* Chain 2: scalar recurrence */
        y = (y << 3) ^ (y >> 5) + c[i];       /* Chain 3: shift recurrence */
        z = z * 7 + a[i-1] * 3;               /* Chain 4: mixed recurrence */
        w = w * 11 - b[i] * 5;                /* Chain 5: another recurrence */
        v = (v & 0xFF) * 19 + c[i];           /* Chain 6: bitwise recurrence */
        u = u * 23 + (a[i-1] & b[i]);         /* Chain 7: complex recurrence */
        
        /* Additional operations to increase register pressure */
        a[i] += x + y - z + w - v + u;
    }
    
    /* Use results to prevent optimization */
    global_acc += a[n-1] + x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i] * b[i];     /* Chain 1: FP recurrence */
        sum2 = sum2 * 0.99 + c[i] * a[i-1];   /* Chain 2: FP with distance 1 */
        sum3 = sum3 * 1.02 - b[i] * c[i-1];   /* Chain 3: Another FP chain */
        prod1 = prod1 * (1.0 + a[i] * 0.001); /* Chain 4: Product recurrence */
        prod2 = prod2 * (1.0 - b[i] * 0.001); /* Chain 5: Another product */
        
        /* Additional mixed operations */
        a[i] = sum1 + sum2 * 2.0 - sum3 / 3.0;
        b[i] = prod1 * prod2 + a[i-1] * 0.5;
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
    int tmp1 = 1, tmp2 = 2, tmp3 = 3;
    
    while (ptr3 < end) {
        /* Multiple pointer-chasing chains */
        acc1 = acc1 * 17 + *ptr1;             /* Chain 1 */
        acc2 = acc2 * 19 + *ptr2;             /* Chain 2 */
        acc3 = acc3 * 23 + *ptr3;             /* Chain 3 */
        
        /* Cross-chain dependencies */
        tmp1 = tmp1 * 29 + acc1 & 0xFF;
        tmp2 = tmp2 * 31 ^ acc2;
        tmp3 = tmp3 * 37 | acc3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional operations */
        *ptr1 = (*ptr1 + tmp1) & 0xFFFF;
        *ptr2 = (*ptr2 ^ tmp2) & 0xFFFF;
        *ptr3 = (*ptr3 | tmp3) & 0xFFFF;
    }
    
    global_acc += acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3;
}

/* Test 4: Mixed integer operations with varying distances */
void test_mixed_distance(int *a, int *b, int *c, int n) {
    int x1 = a[0], x2 = a[1], x3 = a[2];
    int y1 = b[0], y2 = b[1], y3 = b[2];
    
    for (int i = 3; i < n; i++) {
        /* Different dependency distances */
        a[i] = a[i-1] + b[i] * 3;            /* Distance 1 */
        b[i] = b[i-2] * 5 - c[i];            /* Distance 2 */
        c[i] = c[i-3] + a[i-1] * 7;          /* Distance 3 */
        
        /* Scalar recurrences with different latencies */
        x1 = (x1 << 1) + (x1 >> 31) ^ a[i];
        x2 = x2 * 13 + b[i-1];
        x3 = x3 * 17 - c[i-2];
        
        y1 = y1 & 0xFFFFFF + x1;
        y2 = y2 | 0xFFFF00 + x2;
        y3 = y3 ^ 0xFF00FF + x3;
        
        /* Additional operations to increase pressure */
        a[i] += x1 + x2 + x3;
        b[i] += y1 + y2 + y3;
        c[i] += (x1 * y1) + (x2 * y2) + (x3 * y3);
    }
    
    global_acc += a[n-1] + b[n-1] + c[n-1] + x1 + x2 + x3;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    for (int j = 0; j < m; j++) {
        int acc = a[j];
        int prod = 1;
        
        /* Innermost loop - target for modulo scheduling */
        for (int i = 1; i < n; i++) {
            /* High register pressure with multiple chains */
            int idx = i + j * n;
            a[idx] = a[idx-1] * 3 + b[idx];
            b[idx] = b[idx-n] * 5 - c[idx];
            c[idx] = c[idx-1] + a[idx-n] * 7;
            
            /* Scalar recurrences */
            acc = acc * 11 + a[idx];
            prod = prod * 13 ^ b[idx];
            
            /* Additional operations */
            a[idx] += (acc & 0xFF) * prod;
            b[idx] ^= (prod >> 8) + acc;
        }
        
        global_acc += acc + prod;
    }
}

/* Test 6: PowerPC specific - double precision operations */
#ifdef __powerpc__ || __PPC__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0, prod3 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple double-precision dependency chains */
        sum1 = sum1 * 1.0001 + a[i] * b[i];
        sum2 = sum2 * 0.9999 + b[i] * c[i];
        sum3 = sum3 * 1.0002 + c[i] * a[i];
        
        prod1 = prod1 * (1.0 + a[i-1] * 0.0001);
        prod2 = prod2 * (1.0 + b[i-1] * 0.0002);
        prod3 = prod3 * (1.0 + c[i-1] * 0.0003);
        
        /* Cross dependencies */
        a[i] = sum1 + prod1 * 0.5;
        b[i] = sum2 + prod2 * 0.3;
        c[i] = sum3 + prod3 * 0.7;
        
        /* Additional operations */
        sum1 = sum1 * 0.99 + a[i-1];
        sum2 = sum2 * 1.01 - b[i-1];
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2 + prod3);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
void test_manual_unroll(int *a, int *b, int *c, int n) {
    int x0 = a[0], x1 = a[1], x2 = a[2], x3 = a[3];
    int y0 = b[0], y1 = b[1], y2 = b[2], y3 = b[3];
    
    for (int i = 4; i < n; i += 4) {
        /* Unrolled operations creating multiple live values */
        x0 = x0 * 3 + a[i-4] * b[i];
        y0 = y0 * 5 - c[i] ^ x0;
        a[i] = x0 + y0;
        
        x1 = x1 * 7 + a[i-3] * b[i+1];
        y1 = y1 * 11 - c[i+1] ^ x1;
        a[i+1] = x1 + y1;
        
        x2 = x2 * 13 + a[i-2] * b[i+2];
        y2 = y2 * 17 - c[i+2] ^ x2;
        a[i+2] = x2 + y2;
        
        x3 = x3 * 19 + a[i-1] * b[i+3];
        y3 = y3 * 23 - c[i+3] ^ x3;
        a[i+3] = x3 + y3;
        
        /* Cross-iteration dependencies */
        int tmp = x0 + x1 + x2 + x3;
        y0 = y0 + tmp;
        y1 = y1 + tmp;
        y2 = y2 + tmp;
        y3 = y3 + tmp;
    }
    
    global_acc += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
}

/* Main test driver */
int main() {
    /* Initialize data arrays */
    int *int_data1 = malloc(SIZE * sizeof(int));
    int *int_data2 = malloc(SIZE * sizeof(int));
    int *int_data3 = malloc(SIZE * sizeof(int));
    double *double_data1 = malloc(SIZE * sizeof(double));
    double *double_data2 = malloc(SIZE * sizeof(double));
    double *double_data3 = malloc(SIZE * sizeof(double));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        int_data1[i] = rand() % 1000;
        int_data2[i] = rand() % 1000;
        int_data3[i] = rand() % 1000;
        double_data1[i] = (double)(rand() % 1000) / 1000.0;
        double_data2[i] = (double)(rand() % 1000) / 1000.0;
        double_data3[i] = (double)(rand() % 1000) / 1000.0;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_recurrence_multi_chain(int_data1, int_data2, int_data3, SIZE);
        test_float_accumulate(double_data1, double_data2, double_data3, SIZE);
        test_pointer_chasing(int_data1, SIZE, 4);
        test_mixed_distance(int_data1, int_data2, int_data3, SIZE);
        test_nested_loops(int_data1, int_data2, int_data3, 64, 16);
        test_manual_unroll(int_data1, int_data2, int_data3, SIZE);
        
#ifdef __powerpc__ || __PPC__
        test_powerpc_double(double_data1, double_data2, double_data3, SIZE);
#endif
    }
    
    printf("Tests completed. Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(int_data1);
    free(int_data2);
    free(int_data3);
    free(double_data1);
    free(double_data2);
    free(double_data3);
    
    return 0;
}

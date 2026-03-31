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
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        x = x * 3 + a[i];          /* Chain 1: distance 1 */
        y = y * 5 + b[i] - x;      /* Chain 2: depends on x from same iteration */
        z = z * 7 + c[i] + y;      /* Chain 3: depends on y from same iteration */
        
        w = w * 11 + a[i-1] * 2;   /* Chain 4: distance 1, different operation */
        v = v * 13 + b[i] ^ w;     /* Chain 5: XOR operation for variety */
        u = u * 17 + c[i] & v;     /* Chain 6: AND operation */
        
        /* Cross-chain dependencies to create anti-dependencies */
        a[i] = x + y - z;
        b[i] = w ^ v;
        c[i] = u & 0xFF;
    }
    
    global_acc += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0, prod3 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains with different latencies */
        sum1 = sum1 * 1.01 + a[i];         /* Multiplication then addition */
        sum2 = sum2 * 1.02 - b[i] * sum1;  /* Depends on sum1 from same iteration */
        sum3 = sum3 * 1.03 + c[i] / sum2;  /* Division for longer latency */
        
        prod1 = prod1 * (a[i] + 0.5);      /* Different computation pattern */
        prod2 = prod2 * (b[i] - 0.3) * sum1;
        prod3 = prod3 * (c[i] * 0.7) + prod2;
        
        /* Store results creating anti-dependencies */
        a[i] = sum1 + prod1;
        b[i] = sum2 * prod2;
        c[i] = sum3 - prod3;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2 + prod3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    for (int i = 0; i < n - 3 * stride; i++) {
        /* Multiple pointer chasing chains */
        acc1 = acc1 * 2 + *ptr1;
        acc2 = acc2 * 3 + *ptr2 ^ acc1;
        acc3 = acc3 * 5 + *ptr3 & acc2;
        
        tmp1 = tmp1 + (*ptr1 << 2);
        tmp2 = tmp2 ^ (*ptr2 >> 1);
        tmp3 = tmp3 | (*ptr3 & 0xF);
        
        /* Update pointers creating memory dependencies */
        *ptr1 = acc1 + tmp1;
        *ptr2 = acc2 - tmp2;
        *ptr3 = acc3 ^ tmp3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
    }
    
    global_acc += acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops_varying_distance(short *a, short *b, char *c, int n) {
    int sum1 = a[0], sum2 = b[0], sum3 = c[0];
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (int i = 2; i < n; i++) {
        /* Different dependency distances */
        sum1 = sum1 + a[i] * 2;            /* Distance 0 (within iteration) */
        sum2 = sum2 + b[i-1] - sum1;       /* Distance 1 */
        sum3 = sum3 + c[i-2] ^ sum2;       /* Distance 2 */
        
        /* More operations to increase register pressure */
        acc1 = (acc1 << 3) | (a[i] & 0x7);
        acc2 = (acc2 >> 2) ^ (b[i-1] << 1);
        acc3 = (acc3 + c[i-2]) * 3;
        
        /* Create anti-dependencies through stores */
        a[i] = (short)(sum1 & 0xFFFF);
        b[i] = (short)(sum2 | 0x8000);
        c[i] = (char)(sum3 & 0xFF);
    }
    
    global_acc += sum1 + sum2 + sum3 + acc1 + acc2 + acc3;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *matrix, int rows, int cols) {
    for (int r = 1; r < rows - 1; r++) {
        int *prev = matrix + (r - 1) * cols;
        int *curr = matrix + r * cols;
        int *next = matrix + (r + 1) * cols;
        
        /* Innermost loop with carried dependencies */
        for (int c = 1; c < cols - 1; c++) {
            /* 5-point stencil-like computation with multiple chains */
            int top = prev[c];
            int left = curr[c-1];
            int center = curr[c];
            int right = curr[c+1];
            int bottom = next[c];
            
            /* Multiple computation chains */
            int diff1 = top - center;
            int diff2 = left - right;
            int diff3 = center - bottom;
            
            int sum1 = diff1 * diff1;
            int sum2 = diff2 * diff2 + sum1;
            int sum3 = diff3 * diff3 + sum2;
            
            int prod1 = diff1 * diff2;
            int prod2 = diff2 * diff3 * sum1;
            int prod3 = diff3 * diff1 + prod2;
            
            /* Update creating anti-dependencies */
            curr[c] = sum3 + prod1 + prod3;
        }
    }
    
    /* Use result to prevent elimination */
    for (int i = 0; i < rows * cols; i += 64) {
        global_acc += matrix[i];
    }
}

/* Test 6: PowerPC-specific patterns using double and long long */
#ifdef __powerpc__
void test_powerpc_specific(double *a, double *b, int n) {
    double sum_even = 0.0, sum_odd = 0.0;
    long long int_acc_even = 0, int_acc_odd = 0;
    
    for (int i = 0; i < n; i += 2) {
        /* Mixed double and long long operations */
        sum_even = sum_even * 1.5 + a[i];
        sum_odd = sum_odd * 2.0 + b[i+1];
        
        int_acc_even = int_acc_even * 3 + (long long)a[i];
        int_acc_odd = int_acc_odd * 5 ^ (long long)b[i+1];
        
        /* Cross-type dependencies */
        a[i] = sum_even + (double)int_acc_even;
        b[i+1] = sum_odd * (double)int_acc_odd;
    }
    
    global_acc += (long long)(sum_even + sum_odd) + int_acc_even + int_acc_odd;
}
#endif

/* Test 7: Loop with manual unrolling hint */
#pragma GCC unroll 4
void test_manual_unroll(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4;
    
    for (int i = 1; i < n; i++) {
        /* Manually unrolled pattern */
        x = x * 3 + a[i];
        y = y * 5 + b[i] - x;
        z = z * 7 + c[i] + y;
        w = w * 11 + a[i] ^ b[i];
        
        /* Additional ops to fill issue slots */
        int t1 = x << 1;
        int t2 = y >> 2;
        int t3 = z & 0xFF;
        int t4 = w | 0x80;
        
        a[i] = t1 + t2;
        b[i] = t3 * t4;
        c[i] = x ^ y ^ z ^ w;
    }
    
    global_acc += x + y + z + w;
}

/* Main test driver */
int main() {
    /* Initialize data arrays */
    int *data_int = malloc(SIZE * sizeof(int) * 3);
    double *data_double = malloc(SIZE * sizeof(double) * 3);
    short *data_short = malloc(SIZE * sizeof(short) * 2);
    char *data_char = malloc(SIZE * sizeof(char));
    int *matrix = malloc(256 * 256 * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE * 3; i++) {
        data_int[i] = rand() % 100;
    }
    for (int i = 0; i < SIZE * 3; i++) {
        data_double[i] = (double)(rand() % 100) / 10.0;
    }
    for (int i = 0; i < SIZE * 2; i++) {
        data_short[i] = (short)(rand() % 1000);
    }
    for (int i = 0; i < SIZE; i++) {
        data_char[i] = (char)(rand() % 100);
    }
    for (int i = 0; i < 256 * 256; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_recurrence_multi_chain(data_int, 
                                       data_int + SIZE, 
                                       data_int + 2 * SIZE, 
                                       SIZE);
        
        test_float_accumulate(data_double,
                             data_double + SIZE,
                             data_double + 2 * SIZE,
                             SIZE);
        
        test_pointer_chasing(data_int, SIZE, 4);
        
        test_mixed_ops_varying_distance(data_short,
                                       data_short + SIZE,
                                       data_char,
                                       SIZE);
        
        test_nested_loops(matrix, 256, 256);
        
        test_manual_unroll(data_int,
                          data_int + SIZE,
                          data_int + 2 * SIZE,
                          SIZE);
        
        #ifdef __powerpc__
        test_powerpc_specific(data_double,
                             data_double + SIZE,
                             SIZE);
        #endif
    }
    
    /* Output result to prevent dead code elimination */
    printf("Result: %lld\n", global_acc);
    
    /* Cleanup */
    free(data_int);
    free(data_double);
    free(data_short);
    free(data_char);
    free(matrix);
    
    return 0;
}

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

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        x = x * 13 + b[i];          /* Chain 1 */
        y = y * 17 + c[i];          /* Chain 2 */
        z = z * 19 + a[i];          /* Chain 3 */
        w = w * 23 + b[i] * c[i];   /* Chain 4 */
        v = v * 29 + a[i] * x;      /* Chain 5 (depends on Chain 1) */
        u = u * 31 + y * z;         /* Chain 6 (depends on Chain 2 & 3) */
        
        a[i] = x + y + z + w + v + u;
    }
    
    global_acc += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + a[i] * b[i];      /* Chain 1: multiply-accumulate */
        sum2 = sum2 + b[i] * c[i];      /* Chain 2: independent MAC */
        sum3 = sum3 + a[i] * c[i];      /* Chain 3: another MAC */
        prod1 = prod1 * (1.0 + a[i]);   /* Chain 4: product recurrence */
        prod2 = prod2 * (1.0 + b[i]);   /* Chain 5: another product */
        
        /* Cross-chain dependencies to create anti-dependencies */
        a[i] = sum1 * prod1 + sum2 * prod2 + sum3;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer chasing with strided access */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2 * stride;
    int *end = arr + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer chasing chains */
        acc1 = acc1 * 7 + *ptr1;
        acc2 = acc2 * 11 + *ptr2;
        acc3 = acc3 * 13 + *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Cross-store to create register pressure */
        *(ptr1 - 1) = acc1;
        *(ptr2 - 2) = acc2;
        *(ptr3 - 3) = acc3;
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops_varying_distance(short *a, short *b, short *c, int n) {
    int acc[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    for (int i = 0; i < n; i++) {
        /* Multiple operations with different latencies */
        acc[0] = (acc[0] << 3) + a[i];          /* Shift-add */
        acc[1] = (acc[1] * 5) + b[i];           /* Multiply-add */
        acc[2] = (acc[2] & 0xFFF) + c[i];       /* And-add */
        acc[3] = (acc[3] | 0x100) + a[i] * b[i];/* Or-multiply-add */
        
        /* Dependencies with distance 2 */
        if (i >= 2) {
            acc[4] = acc[0] + acc[2] + a[i-2];
            acc[5] = acc[1] * acc[3] + b[i-2];
        }
        
        /* More chains for register pressure */
        acc[6] = acc[6] + (a[i] * c[i]) >> 2;
        acc[7] = acc[7] ^ (b[i] * acc[0]);
        
        /* Store result creating anti-dependencies */
        a[i] = (short)((acc[0] + acc[1] + acc[2] + acc[3] + 
                       acc[4] + acc[5] + acc[6] + acc[7]) & 0xFFFF);
    }
    
    for (int i = 0; i < 8; i++) {
        global_acc += acc[i];
    }
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int i = 1; i < rows - 1; i++) {
        int *prev = mat + (i - 1) * cols;
        int *curr = mat + i * cols;
        int *next = mat + (i + 1) * cols;
        
        /* Innermost loop with carried dependencies */
        for (int j = 1; j < cols - 1; j++) {
            /* Stencil computation with multiple dependencies */
            int north = prev[j];
            int south = next[j];
            int west = curr[j - 1];
            int east = curr[j + 1];
            int center = curr[j];
            
            /* Multiple computation chains */
            int diff1 = north - south;
            int diff2 = east - west;
            int sum1 = center * 2;
            int sum2 = diff1 * diff2;
            int result = (sum1 + sum2) / 4;
            
            /* Anti-dependency: read curr[j], write to next iteration */
            curr[j] = result + west;  /* west is curr[j-1] from this iteration */
        }
    }
    
    /* Use result to prevent elimination */
    global_acc += mat[rows * cols / 2];
}

/* Test 6: PowerPC specific patterns with double operations */
#ifdef __powerpc__
void test_powerpc_double_ops(double *a, double *b, double *c, int n) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    double prod1 = 1.0, prod2 = 1.0, prod3 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double-precision chains */
        sum1 = sum1 + a[i] * 3.14159;
        sum2 = sum2 + b[i] * 2.71828;
        sum3 = sum3 + c[i] * 1.41421;
        
        prod1 = prod1 * (a[i] + 1.0);
        prod2 = prod2 * (b[i] + 2.0);
        prod3 = prod3 * (c[i] + 3.0);
        
        /* Cross dependencies */
        a[i] = sum1 * prod1;
        b[i] = sum2 * prod2;
        c[i] = sum3 * prod3;
        
        /* More operations for register pressure */
        sum1 = sum1 * 0.99;
        sum2 = sum2 * 0.98;
        sum3 = sum3 * 0.97;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2 + prod3);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_manual_unroll(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int u = 4, v = 5, w = 6;
    
    for (int i = 0; i < n; i += 4) {
        /* Unrolled operations creating more parallel chains */
        x = x * 11 + a[i];
        y = y * 13 + b[i];
        z = z * 17 + c[i];
        u = u * 19 + a[i] * b[i];
        v = v * 23 + b[i] * c[i];
        w = w * 29 + c[i] * a[i];
        
        if (i + 1 < n) {
            x = x * 7 + a[i + 1];
            y = y * 11 + b[i + 1];
            z = z * 13 + c[i + 1];
            u = u * 17 + a[i + 1] * b[i + 1];
            v = v * 19 + b[i + 1] * c[i + 1];
            w = w * 23 + c[i + 1] * a[i + 1];
        }
        
        if (i + 2 < n) {
            x = x * 5 + a[i + 2];
            y = y * 7 + b[i + 2];
            z = z * 11 + c[i + 2];
            u = u * 13 + a[i + 2] * b[i + 2];
            v = v * 17 + b[i + 2] * c[i + 2];
            w = w * 19 + c[i + 2] * a[i + 2];
        }
        
        if (i + 3 < n) {
            x = x * 3 + a[i + 3];
            y = y * 5 + b[i + 3];
            z = z * 7 + c[i + 3];
            u = u * 11 + a[i + 3] * b[i + 3];
            v = v * 13 + b[i + 3] * c[i + 3];
            w = w * 17 + c[i + 3] * a[i + 3];
        }
        
        /* Store results creating anti-dependencies */
        a[i] = x + y;
        if (i + 1 < n) b[i + 1] = z + u;
        if (i + 2 < n) c[i + 2] = v + w;
    }
    
    global_acc += x + y + z + u + v + w;
}

int main() {
    /* Initialize data arrays */
    int *data_int1 = malloc(SIZE * sizeof(int));
    int *data_int2 = malloc(SIZE * sizeof(int));
    int *data_int3 = malloc(SIZE * sizeof(int));
    short *data_short1 = malloc(SIZE * sizeof(short));
    short *data_short2 = malloc(SIZE * sizeof(short));
    short *data_short3 = malloc(SIZE * sizeof(short));
    double *data_double1 = malloc(SIZE * sizeof(double));
    double *data_double2 = malloc(SIZE * sizeof(double));
    double *data_double3 = malloc(SIZE * sizeof(double));
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        data_int1[i] = rand() % 100;
        data_int2[i] = rand() % 100;
        data_int3[i] = rand() % 100;
        data_short1[i] = rand() % 100;
        data_short2[i] = rand() % 100;
        data_short3[i] = rand() % 100;
        data_double1[i] = (double)rand() / RAND_MAX;
        data_double2[i] = (double)rand() / RAND_MAX;
        data_double3[i] = (double)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_int_recurrence_multi_chain(data_int1, data_int2, data_int3, SIZE);
        test_float_accumulate(data_double1, data_double2, data_double3, SIZE);
        test_pointer_chasing(data_int1, SIZE, 8);
        test_mixed_ops_varying_distance(data_short1, data_short2, data_short3, SIZE);
        test_nested_loops(matrix, 32, 32);
        test_manual_unroll(data_int1, data_int2, data_int3, SIZE);
        
#ifdef __powerpc__
        test_powerpc_double_ops(data_double1, data_double2, data_double3, SIZE);
#endif
    }
    
    printf("Tests completed. Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(data_int1);
    free(data_int2);
    free(data_int3);
    free(data_short1);
    free(data_short2);
    free(data_short3);
    free(data_double1);
    free(data_double2);
    free(data_double3);
    free(matrix);
    
    return 0;
}

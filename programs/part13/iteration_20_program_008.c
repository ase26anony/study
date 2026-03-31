/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

volatile int global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x = a[0];
    int y = b[0] + b[1];
    int z = c[0] * 2;
    int w = d[0] ^ 0x55;
    
    for (i = 1; i < n; i++) {
        /* Chain 1: linear recurrence with multiplication */
        x = x * 3 + a[i];
        
        /* Chain 2: recurrence with bit operations */
        y = (y << 1) ^ b[i];
        
        /* Chain 3: alternating recurrence */
        z = z + (i % 2 ? c[i] : -c[i]);
        
        /* Chain 4: complex recurrence */
        w = (w & 0xFF) | ((w >> 8) + d[i]);
        
        /* Additional operations to increase register pressure */
        a[i] = x + y;
        b[i] = z - w;
        c[i] = x * z;
        d[i] = y | w;
    }
    
    global_sum += x + y + z + w;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(float *f1, float *f2, float *f3, float *f4, int n) {
    int i;
    float acc1 = f1[0];
    float acc2 = f2[0];
    float acc3 = f3[0];
    float acc4 = f4[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        acc1 = acc1 * 1.01f + f1[i];
        acc2 = acc2 - 0.5f * f2[i];
        acc3 = acc3 + f3[i] * f3[i-1];
        acc4 = (acc4 + f4[i]) * 0.99f;
        
        /* Cross-chain dependencies to create anti-dependencies */
        f1[i] = acc1 + acc2;
        f2[i] = acc3 - acc4;
        f3[i] = acc1 * acc3;
        f4[i] = acc2 / (acc4 + 1.0f);
    }
    
    global_sum += (int)(acc1 + acc2 + acc3 + acc4);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int i;
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (i = 0; i < n - 2 * stride; i++) {
        /* Multiple pointer-chasing chains */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 + (*ptr2 << 1);
        sum3 = sum3 ^ *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Create anti-dependencies through memory */
        *(ptr1 - 1) = sum1;
        *(ptr2 - 2) = sum2;
        *(ptr3 - 3) = sum3;
    }
    
    global_sum += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops_variable_distance(short *s1, short *s2, int *i1, int *i2, int n) {
    int i;
    int acc_int = i1[0];
    short acc_short = s1[0];
    int temp[4] = {0};
    
    for (i = 1; i < n; i++) {
        /* Varying dependency distances */
        temp[i % 4] = acc_int + i1[i];
        acc_int = temp[(i - 1) % 4] * 3;
        
        /* Short operations with different latencies */
        acc_short = (acc_short + s1[i]) >> 1;
        s2[i] = acc_short ^ s2[i-1];
        
        /* Additional operations for register pressure */
        i2[i] = acc_int + acc_short;
        
        /* Complex expression with multiple uses */
        temp[(i + 1) % 4] = (acc_int << 2) | (acc_short & 0xFF);
    }
    
    global_sum += acc_int + acc_short + temp[0] + temp[1];
}

/* Test 5: Nested loops with inner loop being modulo-scheduled */
void test_nested_loops(int *mat, int rows, int cols) {
    int i, j;
    int row_acc = 0;
    int col_acc = 0;
    
    for (i = 1; i < rows; i++) {
        int prev_row = mat[(i-1) * cols];
        int diag_acc = mat[(i-1) * cols + (i-1)];
        
        /* Inner loop with carried dependencies */
        for (j = 1; j < cols; j++) {
            /* Multiple dependency chains */
            int left = mat[i * cols + (j-1)];
            int up = mat[(i-1) * cols + j];
            int diag = mat[(i-1) * cols + (j-1)];
            
            /* Recurrence relations */
            int val = left + up - diag;
            val = val * 2 + (j % 3);
            
            mat[i * cols + j] = val;
            
            /* Accumulate for anti-dependencies */
            row_acc += val;
            col_acc ^= val;
            
            /* Update diagonal accumulator */
            diag_acc = diag_acc + val;
        }
        
        global_sum += row_acc + col_acc + diag_acc;
    }
}

/* Test 6: PowerPC specific - double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *d1, double *d2, double *d3, int n) {
    int i;
    double acc1 = d1[0];
    double acc2 = d2[0];
    double acc3 = d3[0];
    
    for (i = 1; i < n; i++) {
        /* Double precision operations that use FP registers */
        acc1 = acc1 * 1.0001 + d1[i];
        acc2 = acc2 - d2[i] * 0.5;
        acc3 = (acc3 + d3[i]) * 0.9999;
        
        /* Cross dependencies */
        d1[i] = acc1 + acc2;
        d2[i] = acc2 * acc3;
        d3[i] = acc1 - acc3;
        
        /* Additional operations for register pressure */
        acc1 = acc1 * 1.000001;
        acc2 = acc2 / 1.000001;
    }
    
    global_sum += (int)(acc1 + acc2 + acc3);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_manual_unroll(int *arr, int n) {
    int i;
    int acc1 = arr[0], acc2 = arr[1], acc3 = arr[2], acc4 = arr[3];
    
    for (i = 4; i < n; i += 4) {
        /* Unrolled operations creating multiple dependency chains */
        acc1 = acc1 * 3 + arr[i];
        acc2 = acc2 + arr[i+1] * 2;
        acc3 = acc3 ^ arr[i+2];
        acc4 = (acc4 << 1) | (arr[i+3] & 1);
        
        /* Anti-dependencies through array */
        arr[i] = acc1;
        arr[i+1] = acc2;
        arr[i+2] = acc3;
        arr[i+3] = acc4;
        
        /* Cross-chain operations */
        int temp = acc1 + acc2;
        acc1 = acc3 - acc4;
        acc2 = temp * acc4;
        acc3 = acc2 ^ acc1;
        acc4 = temp + acc3;
    }
    
    global_sum += acc1 + acc2 + acc3 + acc4;
}

int main() {
    int i;
    clock_t start, end;
    
    /* Allocate and initialize arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    float *farr3 = malloc(SIZE * sizeof(float));
    float *farr4 = malloc(SIZE * sizeof(float));
    short *sarr1 = malloc(SIZE * sizeof(short));
    short *sarr2 = malloc(SIZE * sizeof(short));
    int *mat = malloc(100 * 100 * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || 
        !farr1 || !farr2 || !farr3 || !farr4 ||
        !sarr1 || !sarr2 || !mat) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
        farr1[i] = i * 0.1f;
        farr2[i] = i * 0.2f;
        farr3[i] = i * 0.3f;
        farr4[i] = i * 0.4f;
        sarr1[i] = i % 100;
        sarr2[i] = (i * 7) % 100;
    }
    
    for (i = 0; i < 10000; i++) {
        mat[i] = i % 256;
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run multiple test functions to trigger different modulo scheduling scenarios */
    for (i = 0; i < ITERATIONS; i++) {
        test_multi_recurrence_int(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(farr1, farr2, farr3, farr4, SIZE);
        test_pointer_chasing(arr1, SIZE, 8);
        test_mixed_ops_variable_distance(sarr1, sarr2, arr1, arr2, SIZE);
        test_nested_loops(mat, 100, 100);
        test_manual_unroll(arr3, SIZE);
        
        #ifdef __powerpc__
        double *darr1 = (double*)farr1;
        double *darr2 = (double*)farr2;
        double *darr3 = (double*)farr3;
        test_powerpc_double(darr1, darr2, darr3, SIZE/2);
        #endif
    }
    
    end = clock();
    printf("Tests completed in %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    printf("Global sum: %d\n", global_sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(farr1);
    free(farr2);
    free(farr3);
    free(farr4);
    free(sarr1);
    free(sarr2);
    free(mat);
    
    return 0;
}

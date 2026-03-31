/* test_modulo_sched.c
 * 
 * Test program to trigger GCC's modulo scheduling register move logic
 * Specifically targets lines 596-606 in modulo-sched.cc
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
volatile long long global_sum = 0;

/* Test 1: Integer recurrence with multiple dependency chains
 * Creates high register pressure with 3 independent chains
 */
void test_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Chain 1: a[i] depends on a[i-1] */
    /* Chain 2: b[i] depends on b[i-2] */
    /* Chain 3: c[i] depends on c[i-1] and d[i] */
    for (i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i] * 3;
        b[i] = b[i-2] + a[i] >> 2;
        c[i] = c[i-1] + d[i] * 7 - a[i];
        d[i] = d[i-1] ^ (b[i] & 0xFF);
    }
    
    /* Accumulate results to prevent optimization */
    for (i = 0; i < n; i++) {
        global_sum += a[i] + b[i] + c[i] + d[i];
    }
}

/* Test 2: Floating-point accumulation with mixed operations
 * Uses double precision to increase register pressure on PowerPC
 */
void test_float_accumulate(double *arr1, double *arr2, double *arr3, int n) {
    int i;
    double acc1 = arr1[0];
    double acc2 = arr2[0];
    double acc3 = arr3[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple carried dependencies */
        acc1 = acc1 * 1.01 + arr1[i];
        acc2 = acc2 * 0.99 - arr2[i];
        acc3 = acc3 + acc1 * acc2;
        
        /* Store results to create register pressure */
        arr1[i] = acc1;
        arr2[i] = acc2;
        arr3[i] = acc3;
    }
    
    /* Use results */
    global_sum += (long long)(acc1 + acc2 + acc3);
}

/* Test 3: Pointer-chasing with strided access
 * Creates anti-dependencies that require register moves
 */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2*stride;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 0; i < n; i++) {
        sum1 = sum1 * 3 + *ptr1;
        sum2 = sum2 * 5 + *ptr2;
        sum3 = sum3 * 7 + *ptr3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Cross-iteration dependencies */
        *ptr1 = sum1 & sum2;
        *ptr2 = sum2 | sum3;
        *ptr3 = sum1 ^ sum3;
    }
    
    global_sum += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with varying dependency distances
 * Uses #pragma to encourage unrolling
 */
void test_mixed_ops(short *s_arr, int *i_arr, long long *ll_arr, int n) {
    int i;
    short s_acc = s_arr[0];
    int i_acc = i_arr[0];
    long long ll_acc = ll_arr[0];
    
    /* Try to encourage software pipelining */
    #pragma GCC unroll 4
    for (i = 1; i < n; i++) {
        /* Different dependency distances */
        s_acc = (s_acc + s_arr[i]) & 0x7FFF;
        i_acc = i_acc * 2 + i_arr[i-1];  /* Distance 1 */
        ll_acc = ll_acc + (long long)s_acc * i_acc;
        
        /* Store with anti-dependencies */
        s_arr[i] = s_acc;
        i_arr[i] = i_acc;
        ll_arr[i] = ll_acc;
    }
    
    global_sum += ll_acc;
}

/* Test 5: Nested loops with innermost hot loop
 * The inner loop should trigger modulo scheduling
 */
void test_nested_loops(int *matrix, int rows, int cols) {
    int i, j;
    int prev_row = 0;
    int prev_col = 0;
    
    for (i = 0; i < rows; i++) {
        int row_acc = 0;
        for (j = 0; j < cols; j++) {
            /* Innermost loop with carried dependencies */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Multiple operations to increase register pressure */
            row_acc = row_acc + val * prev_col;
            prev_col = val;
            
            matrix[idx] = row_acc + prev_row;
            
            /* Additional operations to prevent simplification */
            row_acc ^= (val << 3);
            row_acc += j * 7;
        }
        prev_row = row_acc;
    }
    
    /* Accumulate results */
    for (i = 0; i < rows * cols; i++) {
        global_sum += matrix[i];
    }
}

/* Test 6: PowerPC-specific double operations
 * Uses multiple FP registers to trigger register moves
 */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0];
    double acc2 = b[0];
    double acc3 = c[0];
    double acc4 = a[0] * b[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP operations with carried dependencies */
        acc1 = acc1 * 1.5 + a[i];
        acc2 = acc2 * 2.0 - b[i];
        acc3 = acc3 + acc1 * acc2;
        acc4 = acc4 * 0.5 + acc3;
        
        /* Store results creating anti-dependencies */
        a[i] = acc1;
        b[i] = acc2;
        c[i] = acc3;
        
        /* Use PowerPC-specific operation if available */
        #ifdef __FP_FAST_FMA
        acc4 = __builtin_fma(acc1, acc2, acc4);
        #endif
    }
    
    global_sum += (long long)(acc1 + acc2 + acc3 + acc4);
}
#endif

/* Test 7: Vector-style operations for ARM SVE/RISC-V V
 * Uses compile-time unknown bounds
 */
#ifndef SIZE
#define UNKNOWN_SIZE 512
#endif

void test_vector_style(int *in1, int *in2, int *out, int n) {
    int i;
    int acc1 = in1[0];
    int acc2 = in2[0];
    
    /* Loop with unknown bound encourages vectorization */
    for (i = 1; i < n; i++) {
        /* Strided access pattern */
        int val1 = in1[i];
        int val2 = in2[i];
        
        /* Carried dependencies */
        acc1 = (acc1 + val1) * 3;
        acc2 = (acc2 - val2) * 5;
        
        /* Output with anti-dependency */
        out[i] = acc1 ^ acc2;
        
        /* Additional operations for register pressure */
        acc1 = acc1 & 0xFFFF;
        acc2 = acc2 | 0xFF00;
    }
    
    global_sum += acc1 + acc2;
}

/* Main driver that runs all tests repeatedly */
int main() {
    int i, iter;
    
    /* Allocate test arrays */
    int *arr_int1 = malloc(SIZE * sizeof(int));
    int *arr_int2 = malloc(SIZE * sizeof(int));
    int *arr_int3 = malloc(SIZE * sizeof(int));
    int *arr_int4 = malloc(SIZE * sizeof(int));
    double *arr_double1 = malloc(SIZE * sizeof(double));
    double *arr_double2 = malloc(SIZE * sizeof(double));
    double *arr_double3 = malloc(SIZE * sizeof(double));
    short *arr_short = malloc(SIZE * sizeof(short));
    long long *arr_ll = malloc(SIZE * sizeof(long long));
    int *matrix = malloc(SIZE * SIZE/4 * sizeof(int));
    int *vec_in1 = malloc(SIZE * sizeof(int));
    int *vec_in2 = malloc(SIZE * sizeof(int));
    int *vec_out = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        arr_int1[i] = rand() % 100;
        arr_int2[i] = rand() % 100;
        arr_int3[i] = rand() % 100;
        arr_int4[i] = rand() % 100;
        arr_double1[i] = (double)(rand() % 100) / 3.0;
        arr_double2[i] = (double)(rand() % 100) / 5.0;
        arr_double3[i] = (double)(rand() % 100) / 7.0;
        arr_short[i] = rand() % 32767;
        arr_ll[i] = rand() % 1000;
        vec_in1[i] = rand() % 255;
        vec_in2[i] = rand() % 255;
    }
    
    for (i = 0; i < SIZE * SIZE/4; i++) {
        matrix[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (iter = 0; iter < ITERATIONS; iter++) {
        test_recurrence_int(arr_int1, arr_int2, arr_int3, arr_int4, SIZE);
        test_float_accumulate(arr_double1, arr_double2, arr_double3, SIZE);
        test_pointer_chasing(arr_int1, 4, SIZE/4);
        test_mixed_ops(arr_short, arr_int1, arr_ll, SIZE);
        test_nested_loops(matrix, SIZE/32, 32);
        test_vector_style(vec_in1, vec_in2, vec_out, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double(arr_double1, arr_double2, arr_double3, SIZE);
        #endif
        
        /* Modify inputs slightly each iteration */
        arr_int1[0] += iter;
        arr_double1[0] += (double)iter / 100.0;
    }
    
    printf("Final global sum: %lld\n", global_sum);
    
    /* Free allocated memory */
    free(arr_int1);
    free(arr_int2);
    free(arr_int3);
    free(arr_int4);
    free(arr_double1);
    free(arr_double2);
    free(arr_double3);
    free(arr_short);
    free(arr_ll);
    free(matrix);
    free(vec_in1);
    free(vec_in2);
    free(vec_out);
    
    return 0;
}

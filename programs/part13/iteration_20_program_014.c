/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
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

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x = a[0];
    int y = b[0];
    int z = c[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: distance 1 dependency */
        x = x * 3 + a[i] * 7;
        
        /* Chain 2: distance 1 dependency with different operation */
        y = (y << 2) ^ b[i];
        
        /* Chain 3: more complex recurrence */
        z = (z & 0xFFFF) * 5 + c[i] * 11;
        
        /* Independent computation to increase register pressure */
        d[i] = x + y + z + d[i-1] * 2;
    }
    
    global_sum += x + y + z;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = c[0];
    double prod1 = 1.0;
    double prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i] * 2.5;
        sum2 = sum2 * 0.99 + b[i] * 1.5;
        sum3 = sum3 * 1.02 + c[i] * 3.0;
        
        /* Independent product chains */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 * (1.0 - b[i] * 0.001);
        
        /* Cross-chain operations to increase pressure */
        a[i] = sum1 + prod1;
        b[i] = sum2 + prod2;
        c[i] = sum3 * 0.5;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int i;
    int *ptr1 = data;
    int *ptr2 = data + 1;
    int *ptr3 = data + 2;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    for (i = 0; i < n; i++) {
        /* Multiple pointer-based dependency chains */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 * 3 + *ptr2;
        sum3 = sum3 * 4 + *ptr3;
        
        /* Additional accumulation chains */
        acc1 = acc1 + (sum1 & 0xFF);
        acc2 = acc2 ^ (sum2 << 1);
        acc3 = acc3 | (sum3 >> 1);
        
        /* Update pointers with different strides */
        ptr1 += stride;
        ptr2 += stride * 2;
        ptr3 += stride * 3;
        
        /* Prevent pointer wrap-around */
        if (ptr1 >= data + SIZE) ptr1 = data;
        if (ptr2 >= data + SIZE) ptr2 = data + 1;
        if (ptr3 >= data + SIZE) ptr3 = data + 2;
    }
    
    global_sum += sum1 + sum2 + sum3 + acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops_varying_distance(int *a, int *b, int *c, int n) {
    int i;
    int x1 = a[0], x2 = a[1];
    int y1 = b[0], y2 = b[1];
    int z1 = c[0], z2 = c[1];
    
    for (i = 2; i < n; i++) {
        /* Distance 2 dependencies */
        int new_x = x1 * 5 + x2 * 3 + a[i] * 7;
        int new_y = y1 ^ (y2 << 1) ^ b[i];
        int new_z = (z1 & z2) * 11 + c[i];
        
        /* Distance 1 dependencies for some values */
        x1 = x2;
        x2 = new_x;
        
        y1 = y2;
        y2 = new_y;
        
        z1 = z2;
        z2 = new_z;
        
        /* Additional computations to increase register pressure */
        a[i-1] = x1 + y1 + z1;
        b[i-1] = x2 * y2;
        c[i-1] = z2 & 0xFFFFFF;
    }
    
    global_sum += x1 + x2 + y1 + y2 + z1 + z2;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *matrix, int rows, int cols) {
    int i, j;
    int sum = 0;
    int prod = 1;
    
    for (i = 1; i < rows; i++) {
        int row_sum = 0;
        int row_prod = 1;
        
        /* Innermost loop - target for modulo scheduling */
        for (j = 1; j < cols; j++) {
            /* Carried dependencies across j iterations */
            int idx = i * cols + j;
            int prev_idx = i * cols + (j - 1);
            int above_idx = (i - 1) * cols + j;
            
            /* Multiple dependency chains */
            int val1 = matrix[prev_idx] * 3 + matrix[idx];
            int val2 = matrix[above_idx] * 2 + matrix[idx] * 5;
            
            matrix[idx] = val1 + val2;
            
            /* Accumulations with dependencies */
            row_sum = row_sum * 2 + matrix[idx];
            row_prod = row_prod * (1 + (matrix[idx] & 0xF));
        }
        
        sum += row_sum;
        prod *= (row_prod & 0xFFFF);
    }
    
    global_sum += sum + prod;
}

/* Test 6: PowerPC-specific double precision operations */
#ifdef __powerpc__

void test_powerpc_double_ops(double *a, double *b, int n) {
    int i;
    double sum1 = a[0], sum2 = b[0];
    double prod1 = 1.0, prod2 = 1.0;
    double acc1 = 0.0, acc2 = 0.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple double-precision dependency chains */
        sum1 = sum1 * 1.1 + a[i] * 2.2;
        sum2 = sum2 * 0.9 + b[i] * 3.3;
        
        prod1 = prod1 * (1.0 + a[i] * 0.01);
        prod2 = prod2 * (1.0 - b[i] * 0.01);
        
        acc1 = acc1 + sum1 * prod1;
        acc2 = acc2 + sum2 * prod2;
        
        /* Cross assignments to force register moves */
        a[i-1] = sum1 + acc1;
        b[i-1] = sum2 + acc2;
    }
    
    global_sum += (long long)(sum1 + sum2 + prod1 + prod2 + acc1 + acc2);
}

#endif

/* Test 7: Vector-style operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)

void test_vector_style_ops(int *a, int *b, int *c, int n) {
    int i;
    int sum1 = a[0], sum2 = b[0], sum3 = c[0];
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Use varying strides to create complex access patterns */
    for (i = 1; i < n; i++) {
        /* Multiple independent chains with different operations */
        sum1 = (sum1 << 1) + a[i] * 3;
        sum2 = (sum2 >> 1) + b[i] * 5;
        sum3 = sum3 ^ (c[i] * 7);
        
        /* Accumulators with different update patterns */
        acc1 += sum1 & 0xFF;
        acc2 ^= sum2;
        acc3 |= sum3;
        
        /* Store results with offset to create anti-dependencies */
        if (i + 1 < n) {
            a[i+1] = sum1 + acc1;
            b[i+1] = sum2 + acc2;
            c[i+1] = sum3 + acc3;
        }
    }
    
    global_sum += sum1 + sum2 + sum3 + acc1 + acc2 + acc3;
}

#endif

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, int *c, int *d, double *fa, double *fb, double *fc) {
    int i;
    for (i = 0; i < SIZE; i++) {
        a[i] = (i * 3) & 0xFF;
        b[i] = (i * 5) & 0xFF;
        c[i] = (i * 7) & 0xFF;
        d[i] = (i * 11) & 0xFF;
        fa[i] = (i * 0.1);
        fb[i] = (i * 0.2);
        fc[i] = (i * 0.3);
    }
}

int main() {
    int i;
    
    /* Allocate and initialize arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    double *darr3 = malloc(SIZE * sizeof(double));
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !darr1 || !darr2 || !darr3 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    srand(time(NULL));
    init_arrays(arr1, arr2, arr3, arr4, darr1, darr2, darr3);
    
    /* Initialize matrix */
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = rand() & 0xFF;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        /* Cycle through different tests to exercise various patterns */
        test_int_recurrence_multi_chain(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(darr1, darr2, darr3, SIZE);
        test_pointer_chasing(arr1, 3, SIZE);
        test_mixed_ops_varying_distance(arr2, arr3, arr4, SIZE);
        test_nested_loops(matrix, 32, 32);  /* Smaller matrix for nested test */
        
        #ifdef __powerpc__
        test_powerpc_double_ops(darr1, darr2, SIZE);
        #endif
        
        #if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_style_ops(arr1, arr2, arr3, SIZE);
        #endif
        
        /* Occasionally reinitialize to prevent pattern optimization */
        if (i % 1000 == 0) {
            init_arrays(arr1, arr2, arr3, arr4, darr1, darr2, darr3);
        }
    }
    
    printf("Final global sum: %lld\n", global_sum);
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(darr1);
    free(darr2);
    free(darr3);
    free(matrix);
    
    return 0;
}

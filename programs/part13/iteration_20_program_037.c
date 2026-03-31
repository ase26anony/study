/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduler register move coverage
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int acc1 = a[0];
    int acc2 = b[0] + b[1];
    int acc3 = c[0];
    int acc4 = d[0];
    
    /* Multiple independent dependency chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: distance 1 recurrence */
        acc1 = acc1 * 3 + a[i] * 7;
        
        /* Chain 2: distance 1 recurrence with different operation */
        acc2 = (acc2 << 2) - b[i] * 5;
        
        /* Chain 3: distance 1 recurrence with bit operations */
        acc3 = (acc3 & 0xFFFF) * 11 + c[i];
        
        /* Chain 4: distance 1 recurrence with mixed operations */
        acc4 = (acc4 * 13) ^ (d[i] + i);
        
        /* Additional operations to increase register pressure */
        a[i] = acc1 + acc2;
        b[i] = acc3 ^ acc4;
        c[i] = acc1 * acc3;
        d[i] = acc2 + acc4;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *x, double *y, double *z, int n) {
    double sum1 = x[0];
    double sum2 = y[0];
    double sum3 = z[0];
    double prod1 = 1.0;
    double prod2 = 1.0;
    
    /* Multiple FP dependency chains */
    for (int i = 1; i < n; i++) {
        /* Chain 1: Linear recurrence */
        sum1 = sum1 * 1.01 + x[i] * 2.5;
        
        /* Chain 2: Another recurrence */
        sum2 = sum2 * 0.99 + y[i] * 3.7;
        
        /* Chain 3: Distance 2 recurrence */
        if (i >= 2) {
            sum3 = sum3 * 1.05 + z[i-2] * z[i];
        }
        
        /* Chain 4: Product accumulation */
        prod1 = prod1 * (1.0 + x[i] * 0.001);
        
        /* Chain 5: Another product */
        prod2 = prod2 * (1.0 - y[i] * 0.0005);
        
        /* Cross-chain dependencies to increase pressure */
        x[i] = sum1 + prod1;
        y[i] = sum2 - prod2;
        z[i] = sum3 * 0.5;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Mixed integer operations with strided access */
void test_strided_access(int *arr, int stride, int n) {
    int acc1 = arr[0];
    int acc2 = arr[1];
    int acc3 = arr[2];
    int acc4 = arr[3];
    
    /* Strided access pattern to prevent simple optimization */
    for (int i = 4; i < n; i += stride) {
        /* Multiple recurrence chains with strided dependencies */
        acc1 = acc1 * 17 + arr[i];
        acc2 = acc2 * 19 + arr[i + 1];
        acc3 = acc3 * 23 + arr[i + 2];
        acc4 = acc4 * 29 + arr[i + 3];
        
        /* Write back with different strides */
        arr[i] = acc1;
        arr[i + 1] = acc2;
        arr[i + 2] = acc3;
        arr[i + 3] = acc4;
        
        /* Additional operations for register pressure */
        int temp1 = acc1 ^ acc2;
        int temp2 = acc3 & acc4;
        int temp3 = acc1 | acc3;
        int temp4 = acc2 ^ acc4;
        
        arr[i] ^= temp1;
        arr[i + 1] &= temp2;
        arr[i + 2] |= temp3;
        arr[i + 3] ^= temp4;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 4: Nested loops with inner loop having carried dependencies */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int r = 1; r < rows; r++) {
        int prev_row = (r - 1) * cols;
        int curr_row = r * cols;
        
        /* Inner loop with multiple dependency chains */
        for (int c = 1; c < cols; c++) {
            /* Horizontal recurrence (within same row) */
            int left = mat[curr_row + c - 1];
            
            /* Vertical recurrence (from previous row) */
            int top = mat[prev_row + c];
            
            /* Diagonal recurrence */
            int diag = mat[prev_row + c - 1];
            
            /* Multiple computations with carried dependencies */
            int val1 = left * 3 + top * 5;
            int val2 = diag * 7 + c * 11;
            int val3 = (left ^ top) * 13;
            int val4 = (diag & c) * 17;
            
            /* Combined result with more operations */
            mat[curr_row + c] = (val1 + val2) * (val3 - val4) + r;
            
            /* Additional operations to increase register usage */
            int temp = mat[curr_row + c];
            mat[curr_row + c] = (temp << 3) | (temp >> 29);
            mat[curr_row + c] ^= 0xAAAAAAAA;
        }
    }
    
    /* Accumulate some values to prevent elimination */
    for (int i = 0; i < rows * cols; i += 32) {
        global_acc += mat[i];
    }
}

/* Test 5: Pointer-chasing loop with arithmetic */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = &data[0];
    int *ptr2 = &data[1];
    int *ptr3 = &data[2];
    int *ptr4 = &data[3];
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    for (int i = 0; i < n - 4; i++) {
        /* Pointer chasing with different strides */
        sum1 = sum1 * 31 + *ptr1;
        sum2 = sum2 * 37 + *ptr2;
        sum3 = sum3 * 41 + *ptr3;
        sum4 = sum4 * 43 + *ptr4;
        
        /* Update pointers with different increments */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        ptr4 += 4;
        
        /* Additional computations */
        int mix1 = sum1 ^ sum2;
        int mix2 = sum3 & sum4;
        int mix3 = sum1 | sum3;
        int mix4 = sum2 ^ sum4;
        
        data[i] = mix1 + mix2 + mix3 + mix4;
    }
    
    global_acc += sum1 + sum2 + sum3 + sum4;
}

/* Test 6: Loop with manual unrolling hint */
void test_unrolled_loop(int *a, int *b, int *c, int n) {
    int acc1 = a[0], acc2 = b[0], acc3 = c[0];
    
    /* Manual unrolling to increase operations per iteration */
    for (int i = 1; i < n - 3; i += 4) {
        /* Unrolled iteration 1 */
        acc1 = acc1 * 2 + a[i];
        acc2 = acc2 * 3 + b[i];
        acc3 = acc3 * 5 + c[i];
        a[i] = acc1 ^ acc2 ^ acc3;
        
        /* Unrolled iteration 2 */
        acc1 = acc1 * 7 + a[i+1];
        acc2 = acc2 * 11 + b[i+1];
        acc3 = acc3 * 13 + c[i+1];
        a[i+1] = acc1 & acc2 & acc3;
        
        /* Unrolled iteration 3 */
        acc1 = acc1 * 17 + a[i+2];
        acc2 = acc2 * 19 + b[i+2];
        acc3 = acc3 * 23 + c[i+2];
        a[i+2] = acc1 | acc2 | acc3;
        
        /* Unrolled iteration 4 */
        acc1 = acc1 * 29 + a[i+3];
        acc2 = acc2 * 31 + b[i+3];
        acc3 = acc3 * 37 + c[i+3];
        a[i+3] = acc1 ^ (acc2 << 1) ^ (acc3 << 2);
        
        /* Cross-iteration dependencies */
        b[i] = a[i] + a[i+1];
        b[i+1] = a[i+1] + a[i+2];
        b[i+2] = a[i+2] + a[i+3];
        b[i+3] = a[i+3] + a[i];
        
        c[i] = b[i] * b[i+1];
        c[i+1] = b[i+1] * b[i+2];
        c[i+2] = b[i+2] * b[i+3];
        c[i+3] = b[i+3] * b[i];
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 7: Mixed data types for architecture-specific testing */
#ifdef __powerpc__
void test_powerpc_specific(double *dbl_arr, long long *ll_arr, int n) {
    double dbl_acc = dbl_arr[0];
    long long ll_acc = ll_arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Double precision operations for FP register pressure */
        dbl_acc = dbl_acc * 1.5 + dbl_arr[i] * 2.0;
        
        /* 64-bit integer operations */
        ll_acc = ll_acc * 3 + ll_arr[i] * 5;
        
        /* Mix them to prevent optimization */
        dbl_arr[i] = dbl_acc + (double)ll_acc;
        ll_arr[i] = (long long)dbl_acc ^ ll_acc;
    }
    
    global_acc += (long long)dbl_acc + ll_acc;
}
#endif

#ifdef __ARM_FEATURE_SVE
/* Simple vector-like pattern for ARM SVE */
void test_sve_pattern(int *data, int n) {
    int acc1 = data[0], acc2 = data[1], acc3 = data[2], acc4 = data[3];
    
    for (int i = 4; i < n; i++) {
        /* Multiple independent chains */
        acc1 = acc1 * 2 + data[i];
        acc2 = acc2 * 3 + data[i-1];
        acc3 = acc3 * 5 + data[i-2];
        acc4 = acc4 * 7 + data[i-3];
        
        /* Store results with permutation */
        data[i] = acc1;
        data[i-1] = acc2;
        data[i-2] = acc3;
        data[i-3] = acc4;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}
#endif

/* Main test driver */
int main() {
    /* Initialize data arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    double *dbl_arr = malloc(SIZE * sizeof(double));
    int *mat = malloc(SIZE * SIZE / 4 * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        arr4[i] = rand() % 1000;
        dbl_arr[i] = (double)(rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < SIZE * SIZE / 4; i++) {
        mat[i] = rand() % 1000;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_multi_recurrence_int(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(dbl_arr, dbl_arr + SIZE/2, dbl_arr + SIZE/4, SIZE/2);
        test_strided_access(arr1, 4, SIZE);
        test_nested_loops(mat, 32, 32);
        test_pointer_chasing(arr2, SIZE);
        test_unrolled_loop(arr3, arr4, arr1, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_specific(dbl_arr, (long long*)arr1, SIZE/2);
        #endif
        
        #ifdef __ARM_FEATURE_SVE
        test_sve_pattern(arr2, SIZE);
        #endif
    }
    
    printf("Final accumulator value: %lld\n", global_acc);
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(dbl_arr);
    free(mat);
    
    return 0;
}

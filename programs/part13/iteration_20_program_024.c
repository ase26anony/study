/* test_modulo_sched.c
 * 
 * Test program to trigger GCC's modulo scheduler register move logic
 * targeting uncovered lines in modulo-sched.cc:596-606
 * 
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * 
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
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains to increase register pressure */
    int acc1 = a[0];
    int acc2 = b[0] + b[1];
    int acc3 = c[0];
    int acc4 = d[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple recurrence */
        acc1 = acc1 * 3 + a[i];
        
        /* Chain 2: Recurrence with offset 2 */
        if (i >= 2) {
            acc2 = acc2 + b[i] * acc2;
        }
        
        /* Chain 3: Complex recurrence with multiple operations */
        acc3 = (acc3 << 2) | (c[i] & 0xFF);
        acc3 = acc3 ^ (acc3 >> 3);
        
        /* Chain 4: Another recurrence */
        acc4 = acc4 * 7 - d[i];
        
        /* Cross-chain operations to create register pressure */
        a[i] = acc1 + acc3;
        b[i] = acc2 - acc4;
        c[i] = acc3 * acc1;
        d[i] = acc4 | acc2;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *x, double *y, double *z, int n) {
    int i;
    double sum1 = x[0];
    double sum2 = y[0];
    double prod1 = z[0];
    double prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + x[i] * 1.5;
        sum2 = sum2 * 0.99 + y[i];
        
        prod1 = prod1 * (1.0 + z[i] * 0.01);
        prod2 = prod2 / (1.0 + x[i] * 0.001) + y[i];
        
        /* Cross dependencies to force register moves */
        x[i] = sum1 * prod1;
        y[i] = sum2 + prod2;
        z[i] = (sum1 - sum2) * (prod1 / prod2);
    }
    
    global_acc += (long long)(sum1 + sum2 + prod1 + prod2);
}

/* Test 3: Mixed integer/float with strided access */
void test_mixed_strided(int *arr_int, double *arr_dbl, int n) {
    int i;
    int int_acc = arr_int[0];
    double dbl_acc = arr_dbl[0];
    int int_acc2 = arr_int[1];
    double dbl_acc2 = arr_dbl[1];
    
    /* Strided access pattern */
    for (i = 2; i < n - 2; i += 1) {
        /* Integer chain with shift operations */
        int_acc = (int_acc << 3) | (arr_int[i] & 0x7);
        int_acc = int_acc ^ (int_acc >> 5);
        
        /* Floating chain with recurrence */
        dbl_acc = dbl_acc * 1.01 + arr_dbl[i] * 0.5;
        
        /* Second set of chains */
        int_acc2 = int_acc2 * 11 + arr_int[i+1];
        dbl_acc2 = dbl_acc2 / 1.02 - arr_dbl[i+1];
        
        /* Store results with anti-dependencies */
        arr_int[i-1] = int_acc + int_acc2;
        arr_dbl[i-1] = dbl_acc * dbl_acc2;
    }
    
    global_acc += int_acc + int_acc2 + (long long)(dbl_acc + dbl_acc2);
}

/* Test 4: Pointer-chasing with arithmetic (triggers register moves) */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = &data[0];
    int *ptr2 = &data[1];
    int *ptr3 = &data[2];
    int *end = &data[n-3];
    
    int sum1 = *ptr1;
    int sum2 = *ptr2;
    int sum3 = *ptr3;
    
    while (ptr3 < end) {
        /* Pointer chasing with different strides */
        sum1 = sum1 * 5 + *ptr1;
        sum2 = sum2 + *ptr2 * 3;
        sum3 = sum3 ^ *ptr3;
        
        /* Update pointers - creates anti-dependencies */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 1;
        
        /* More operations to increase pressure */
        sum1 = sum1 ^ sum2;
        sum2 = sum2 + sum3;
        sum3 = sum3 * 7 - sum1;
        
        /* Store back through pointers */
        *(ptr1-1) = sum1;
        *(ptr2-2) = sum2;
        *(ptr3-1) = sum3;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: Nested loops with inner loop carrying dependencies */
void test_nested_loops(int *mat, int rows, int cols) {
    int i, j;
    
    for (i = 1; i < rows; i++) {
        int row_acc = mat[i * cols];
        int diag_acc = mat[i * cols + i % cols];
        
        /* Inner loop with carried dependency */
        for (j = 1; j < cols; j++) {
            int idx = i * cols + j;
            int prev_idx = (i-1) * cols + j;
            int left_idx = i * cols + (j-1);
            
            /* Multiple recurrence relations */
            row_acc = row_acc * 3 + mat[idx];
            diag_acc = diag_acc + mat[prev_idx] - mat[left_idx];
            
            /* Anti-dependency: read old value, write new */
            mat[idx] = row_acc + diag_acc;
        }
        
        global_acc += row_acc + diag_acc;
    }
}

/* Test 6: SIMD-style operations (triggers vector modulo scheduling) */
void test_simd_style(float *a, float *b, float *c, int n) {
    int i;
    float acc0 = a[0];
    float acc1 = b[0];
    float acc2 = c[0];
    float acc3 = a[1];
    
    /* Manual unrolling to increase register pressure */
    for (i = 1; i < n - 3; i += 2) {
        /* Four parallel dependency chains */
        acc0 = acc0 * 1.1f + a[i];
        acc1 = acc1 + b[i] * acc0;
        acc2 = acc2 * 0.9f - c[i];
        acc3 = acc3 / 1.05f + a[i+1];
        
        /* Cross-chain operations */
        float tmp0 = acc0 + acc1;
        float tmp1 = acc2 * acc3;
        float tmp2 = acc0 - acc2;
        float tmp3 = acc1 + acc3;
        
        /* Store with anti-dependencies */
        a[i] = tmp0;
        b[i] = tmp1;
        c[i] = tmp2;
        a[i+1] = tmp3;
        
        /* More operations to prevent optimization */
        acc0 = acc0 ^ (int)tmp0;
        acc1 = acc1 + (int)tmp1;
        acc2 = acc2 * (int)tmp2;
        acc3 = acc3 - (int)tmp3;
    }
    
    global_acc += (long long)(acc0 + acc1 + acc2 + acc3);
}

/* Test 7: PowerPC specific - double precision with FMA-like pattern */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    int i;
    double sum = a[0];
    double prod = b[0];
    double acc = c[0];
    
    for (i = 1; i < n; i++) {
        /* FMA-like patterns that use FP registers heavily */
        sum = sum + a[i] * 2.0;
        prod = prod * b[i] * 1.5;
        acc = acc * 0.999 + c[i];
        
        /* Cross dependencies */
        double t1 = sum * prod;
        double t2 = acc + prod;
        double t3 = sum - acc;
        
        /* Store results */
        a[i] = t1;
        b[i] = t2;
        c[i] = t3;
        
        /* Update accumulators */
        sum = sum + t1;
        prod = prod * t2;
        acc = acc - t3;
    }
    
    global_acc += (long long)(sum + prod + acc);
}
#endif

/* Test 8: Compile-time unrolled loop */
#pragma GCC unroll 4
void test_unrolled_loop(int *arr, int n) {
    int i;
    int acc0 = arr[0];
    int acc1 = arr[1];
    int acc2 = arr[2];
    int acc3 = arr[3];
    
    for (i = 4; i < n; i++) {
        /* Unrolled operations create register pressure */
        acc0 = acc0 * 3 + arr[i];
        acc1 = acc1 + arr[i-1] * 5;
        acc2 = acc2 ^ arr[i-2];
        acc3 = acc3 - arr[i-3];
        
        /* Rotate and mix */
        int tmp = acc0;
        acc0 = acc1;
        acc1 = acc2;
        acc2 = acc3;
        acc3 = tmp;
        
        /* Store with anti-dependency */
        arr[i-3] = acc0 + acc1 + acc2 + acc3;
    }
    
    global_acc += acc0 + acc1 + acc2 + acc3;
}

/* Initialize arrays with pattern */
void init_arrays(int *a, int *b, int *c, int *d, 
                 double *da, double *db, double *dc,
                 float *fa, float *fb, float *fc, int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i] = i % 100;
        b[i] = (i * 3) % 100;
        c[i] = (i * 5) % 100;
        d[i] = (i * 7) % 100;
        da[i] = i * 0.1;
        db[i] = i * 0.3;
        dc[i] = i * 0.5;
        fa[i] = i * 0.01f;
        fb[i] = i * 0.03f;
        fc[i] = i * 0.05f;
    }
}

int main() {
    int i;
    
    /* Allocate arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    int *mat = malloc(SIZE * SIZE/4 * sizeof(int));
    
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    double *darr3 = malloc(SIZE * sizeof(double));
    
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    float *farr3 = malloc(SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !mat ||
        !darr1 || !darr2 || !darr3 ||
        !farr1 || !farr2 || !farr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERS; i++) {
        /* Re-initialize with different patterns */
        init_arrays(arr1, arr2, arr3, arr4, darr1, darr2, darr3, 
                   farr1, farr2, farr3, SIZE);
        
        /* Call test functions */
        test_int_recurrence_multi_chain(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(darr1, darr2, darr3, SIZE);
        test_mixed_strided(arr1, darr1, SIZE);
        test_pointer_chasing(arr2, SIZE);
        test_nested_loops(mat, 64, 16);
        test_simd_style(farr1, farr2, farr3, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_double(darr1, darr2, darr3, SIZE);
        #endif
        
        test_unrolled_loop(arr3, SIZE);
        
        /* Add some randomness to prevent pattern recognition */
        if (i % 100 == 0) {
            arr1[0] = rand() % 100;
            darr1[0] = (double)rand() / RAND_MAX;
        }
    }
    
    /* Output result to prevent dead code elimination */
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1); free(arr2); free(arr3); free(arr4); free(mat);
    free(darr1); free(darr2); free(darr3);
    free(farr1); free(farr2); free(farr3);
    
    return 0;
}

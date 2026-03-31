/* test_modulo_sched.c
 * 
 * This program creates loops designed to trigger GCC's modulo scheduler
 * register move logic, specifically targeting the uncovered lines in
 * modulo-sched.cc's schedule_reg_move() function.
 *
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

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent dependency chains */
    int x = a[0];
    int y = b[0];
    int z = c[0];
    int w = d[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: distance 1 dependency */
        x = x * 3 + a[i];
        
        /* Chain 2: distance 1 dependency with different operation */
        y = (y << 2) ^ b[i];
        
        /* Chain 3: distance 1 dependency with multiply */
        z = z * 7 + c[i] * 5;
        
        /* Chain 4: mixed operations to increase register pressure */
        w = (w & 0xFF) * 11 + d[i];
        
        /* Additional operations to create more register pressure */
        a[i] = x + y;
        b[i] = z - w;
        c[i] = x * z;
        d[i] = y & w;
    }
    
    global_acc += x + y + z + w;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = c[0];
    double prod1 = 1.0;
    double prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];
        sum2 = sum2 * 0.99 + b[i];
        sum3 = sum3 * 1.05 + c[i];
        
        /* Additional chains with multiplication */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 * (1.0 - b[i] * 0.001);
        
        /* Cross-chain dependencies to increase complexity */
        a[i] = sum1 + prod1;
        b[i] = sum2 - prod2;
        c[i] = sum3 * prod1;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Mixed integer/float operations with strided access */
void test_mixed_operations(int *arr_int, double *arr_dbl, int n) {
    int i;
    int int_acc = arr_int[0];
    double dbl_acc = arr_dbl[0];
    int tmp1 = 0, tmp2 = 0;
    double tmp3 = 0.0, tmp4 = 0.0;
    
    /* Use pragma to encourage unrolling */
    #pragma GCC unroll 4
    for (i = 1; i < n; i++) {
        /* Integer chain with shift and arithmetic */
        int_acc = (int_acc << 3) | (arr_int[i] & 0xFF);
        tmp1 = int_acc * 13 + arr_int[i-1];
        
        /* Floating chain with multiplication */
        dbl_acc = dbl_acc * 1.5 + arr_dbl[i];
        tmp3 = dbl_acc * 2.0 - arr_dbl[i-1];
        
        /* Additional mixed operations */
        tmp2 = tmp1 ^ (int)(tmp3 * 100);
        tmp4 = tmp3 + (double)tmp2 * 0.01;
        
        /* Store results to create anti-dependencies */
        arr_int[i] = tmp1 + tmp2;
        arr_dbl[i] = tmp3 * tmp4;
    }
    
    global_acc += int_acc + (long long)dbl_acc;
}

/* Test 4: Pointer-chasing pattern with multiple accumulators */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = data;
    int *ptr2 = data + 1;
    int *ptr3 = data + 2;
    int *end = data + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod1 = 1, prod2 = 1;
    
    while (ptr3 < end) {
        /* Multiple pointer-based dependency chains */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 * 3 + *ptr2;
        sum3 = sum3 * 5 + *ptr3;
        
        prod1 = prod1 * (*ptr1 + 1);
        prod2 = prod2 * (*ptr2 - 1);
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional operations to increase register pressure */
        *ptr1 = sum1 ^ sum2;
        *ptr2 = sum2 & sum3;
        *ptr3 = prod1 | prod2;
    }
    
    global_acc += sum1 + sum2 + sum3 + prod1 + prod2;
}

/* Test 5: Nested loops with inner loop having carried dependencies */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = a[i];
        int tmp1 = b[i];
        int tmp2 = c[i];
        
        /* Inner loop with carried dependencies */
        for (j = 1; j < m; j++) {
            /* Multiple dependency chains in inner loop */
            acc = acc * 17 + a[i * m + j];
            tmp1 = tmp1 ^ (b[i * m + j] << (j % 4));
            tmp2 = tmp2 * 3 - c[i * m + j];
            
            /* Cross-store to create anti-dependencies */
            a[i * m + j] = acc + tmp1;
            b[i * m + j] = tmp1 - tmp2;
            c[i * m + j] = acc * tmp2;
        }
        
        global_acc += acc + tmp1 + tmp2;
    }
}

/* Test 6: Architecture-specific patterns */
#ifdef __powerpc__
void test_powerpc_specific(double *a, double *b, int n) {
    int i;
    double sum1 = a[0], sum2 = b[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    /* PowerPC often has good FP modulo scheduling */
    for (i = 1; i < n; i++) {
        sum1 = sum1 * 1.1 + a[i] * 2.0;
        sum2 = sum2 * 0.9 + b[i] * 3.0;
        prod1 = prod1 * (1.0 + a[i]);
        prod2 = prod2 * (1.0 - b[i]);
        
        /* FMA-like pattern */
        a[i] = sum1 * prod1 + sum2;
        b[i] = sum2 * prod2 - sum1;
    }
    
    global_acc += (long long)(sum1 + sum2 + prod1 + prod2);
}
#endif

#ifdef __ARM_FEATURE_SVE
void test_arm_sve_specific(int *a, int *b, int n) {
    int i;
    int sum1 = a[0], sum2 = b[0];
    int prod1 = 1, prod2 = 1;
    
    /* SVE-friendly loop with unknown bounds at compile time */
    for (i = 1; i < n; i++) {
        sum1 = (sum1 << 1) + a[i];
        sum2 = (sum2 >> 1) + b[i];
        prod1 = prod1 * (a[i] | 1);
        prod2 = prod2 * (b[i] & 0xFF);
        
        a[i] = sum1 ^ prod1;
        b[i] = sum2 & prod2;
    }
    
    global_acc += sum1 + sum2 + prod1 + prod2;
}
#endif

/* Main test driver */
int main() {
    int i, iter;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    double *arr_dbl1 = malloc(SIZE * sizeof(double));
    double *arr_dbl2 = malloc(SIZE * sizeof(double));
    double *arr_dbl3 = malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr_dbl1 || !arr_dbl2 || !arr_dbl3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        arr_dbl1[i] = (double)(rand() % 100) / 10.0;
        arr_dbl2[i] = (double)(rand() % 100) / 10.0;
        arr_dbl3[i] = (double)(rand() % 100) / 10.0;
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (iter = 0; iter < ITERS; iter++) {
        /* Alternate between different test patterns */
        switch (iter % 5) {
            case 0:
                test_multi_recurrence_int(arr1, arr2, arr3, arr4, SIZE);
                break;
            case 1:
                test_float_accumulate(arr_dbl1, arr_dbl2, arr_dbl3, SIZE);
                break;
            case 2:
                test_mixed_operations(arr1, arr_dbl1, SIZE);
                break;
            case 3:
                test_pointer_chasing(arr2, SIZE);
                break;
            case 4:
                test_nested_loops(arr1, arr2, arr3, 16, 64);
                break;
        }
        
        #ifdef __powerpc__
        if (iter % 10 == 0) {
            test_powerpc_specific(arr_dbl1, arr_dbl2, SIZE);
        }
        #endif
        
        #ifdef __ARM_FEATURE_SVE
        if (iter % 10 == 0) {
            test_arm_sve_specific(arr3, arr4, SIZE);
        }
        #endif
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr_dbl1);
    free(arr_dbl2);
    free(arr_dbl3);
    
    return 0;
}

/* test_modulo_sched.c
 * 
 * Test program to trigger GCC's modulo scheduling register move logic.
 * Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        x1 = x1 * 3 + a[i] - y1;
        /* Chain 2: distance-1 with different ops */
        x2 = (x2 << 2) | (b[i] & 0xFF);
        /* Chain 3: more complex recurrence */
        x3 = x3 + (y3 * 5) - (c[i] >> 1);
        /* Chain 4: mixed operations */
        x4 = (x4 ^ d[i]) + (y4 * 7);
        
        /* Update secondary variables with cross-chain dependencies */
        y1 = x1 + (y1 >> 1);
        y2 = x2 - (y2 & 0x7F);
        y3 = x3 ^ (y3 * 3);
        y4 = x4 | (y4 + 1);
        
        /* Store results to prevent optimization */
        a[i] = x1;
        b[i] = x2;
        c[i] = x3;
        d[i] = x4;
    }
    
    global_acc += x1 + x2 + x3 + x4;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i] * b[i];
        sum2 = sum2 * 0.99 + c[i] * a[i];
        sum3 = sum3 * 1.05 + b[i] * c[i];
        
        /* Independent product chains */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 * (1.0 - b[i] * 0.001);
        
        /* Cross-chain operations to increase register pressure */
        a[i] = sum1 + prod1;
        b[i] = sum2 - prod2;
        c[i] = sum3 * (prod1 + prod2);
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Mixed integer/float with strided access */
void test_mixed_strided(int *int_arr, double *dbl_arr, int n) {
    int i;
    int int_acc1 = int_arr[0], int_acc2 = int_arr[1];
    double dbl_acc1 = dbl_arr[0], dbl_acc2 = dbl_arr[1];
    
    /* Process with stride 2 to create more register pressure */
    for (i = 2; i < n - 1; i += 2) {
        /* Integer recurrences */
        int_acc1 = (int_acc1 * 5 + int_arr[i]) >> 1;
        int_acc2 = (int_acc2 * 3 + int_arr[i+1]) & 0x7FFFFFFF;
        
        /* Floating-point recurrences */
        dbl_acc1 = dbl_acc1 * 1.5 + dbl_arr[i] * 2.0;
        dbl_acc2 = dbl_acc2 * 0.5 + dbl_arr[i+1] * 3.0;
        
        /* Cross-type operations */
        int_arr[i] = int_acc1 + (int)(dbl_acc1 * 100);
        int_arr[i+1] = int_acc2 - (int)(dbl_acc2 * 50);
        dbl_arr[i] = dbl_acc1 + int_acc1 * 0.01;
        dbl_arr[i+1] = dbl_acc2 - int_acc2 * 0.02;
    }
    
    global_acc += int_acc1 + int_acc2 + (long long)(dbl_acc1 + dbl_acc2);
}

/* Test 4: Pointer-chasing with arithmetic (triggers register moves) */
void test_pointer_chasing(int *arr, int n) {
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[1];
    int *ptr3 = &arr[2];
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n - 3; i++) {
        /* Pointer chasing with different strides */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 * 3 + *ptr2;
        sum3 = sum3 * 5 + *ptr3;
        
        /* Update pointers with different increments */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional arithmetic to increase register usage */
        arr[i] = sum1 ^ sum2;
        arr[i+1] = sum2 & sum3;
        arr[i+2] = sum1 | sum3;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: Nested loops with inner loop having carried dependency */
void test_nested_loops(int *a, int *b, int *c, int n) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = a[i];
        /* Inner loop with carried dependency */
        for (j = 0; j < 8; j++) {  /* Small fixed bound encourages unrolling */
            acc = acc * 3 + b[(i + j) % n] - c[(i - j + n) % n];
            /* Additional operations to increase register pressure */
            b[(i + j) % n] = acc >> 1;
            c[(i - j + n) % n] = acc & 0xFF;
        }
        a[i] = acc;
    }
    
    /* Sum results */
    int total = 0;
    for (i = 0; i < n; i++) {
        total += a[i] + b[i] + c[i];
    }
    global_acc += total;
}

/* Test 6: SIMD-style operations (for architectures with vector support) */
#ifdef __PPC__
void test_powerpc_specific(double *a, double *b, int n) {
    int i;
    double sum[4] = {a[0], a[1], a[2], a[3]};
    double prod[4] = {b[0], b[1], b[2], b[3]};
    
    /* Process 4 elements at a time */
    for (i = 4; i < n - 3; i += 4) {
        /* Multiple dependency chains */
        sum[0] = sum[0] * 1.1 + a[i] * b[i];
        sum[1] = sum[1] * 1.2 + a[i+1] * b[i+1];
        sum[2] = sum[2] * 1.3 + a[i+2] * b[i+2];
        sum[3] = sum[3] * 1.4 + a[i+3] * b[i+3];
        
        prod[0] = prod[0] * (1.0 + a[i] * 0.01);
        prod[1] = prod[1] * (1.0 + a[i+1] * 0.02);
        prod[2] = prod[2] * (1.0 + a[i+2] * 0.03);
        prod[3] = prod[3] * (1.0 + a[i+3] * 0.04);
        
        /* Store with cross dependencies */
        a[i] = sum[0] + prod[0];
        a[i+1] = sum[1] + prod[1];
        a[i+2] = sum[2] + prod[2];
        a[i+3] = sum[3] + prod[3];
    }
    
    global_acc += (long long)(sum[0] + sum[1] + sum[2] + sum[3]);
}
#endif

/* Main driver that runs all tests repeatedly */
int main() {
    int i, iter;
    int *arr1, *arr2, *arr3, *arr4;
    double *darr1, *darr2, *darr3;
    
    /* Allocate and initialize arrays */
    arr1 = (int*)malloc(SIZE * sizeof(int));
    arr2 = (int*)malloc(SIZE * sizeof(int));
    arr3 = (int*)malloc(SIZE * sizeof(int));
    arr4 = (int*)malloc(SIZE * sizeof(int));
    darr1 = (double*)malloc(SIZE * sizeof(double));
    darr2 = (double*)malloc(SIZE * sizeof(double));
    darr3 = (double*)malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        arr4[i] = rand() % 1000;
        darr1[i] = (double)(rand() % 1000) / 1000.0;
        darr2[i] = (double)(rand() % 1000) / 1000.0;
        darr3[i] = (double)(rand() % 1000) / 1000.0;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (iter = 0; iter < ITERS; iter++) {
        test_multi_recurrence_int(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(darr1, darr2, darr3, SIZE);
        test_mixed_strided(arr1, darr1, SIZE);
        test_pointer_chasing(arr2, SIZE);
        test_nested_loops(arr3, arr4, arr1, SIZE/4);
        
        #ifdef __PPC__
        test_powerpc_specific(darr1, darr2, SIZE);
        #endif
    }
    
    /* Output result to prevent optimization */
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(darr1);
    free(darr2);
    free(darr3);
    
    return 0;
}

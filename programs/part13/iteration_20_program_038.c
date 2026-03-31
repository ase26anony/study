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
        w = w * 11 + a[i-1] * z;   /* Chain 4: uses a[i-1], distance 1 */
        v = v * 13 + b[i] ^ w;     /* Chain 5: bitwise ops for variety */
        u = u * 17 + c[i] | v;     /* Chain 6: more operations */
        
        /* Store results to create output dependencies */
        a[i] = x + y;
        b[i] = z - w;
        c[i] = v ^ u;
    }
    
    global_acc += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];         /* Distance 1 recurrence */
        sum2 = sum2 * 0.99 + b[i] * sum1;  /* Depends on sum1 from same iter */
        sum3 = sum3 * 1.02 + c[i] + sum2;  /* Another chain */
        prod1 = prod1 * (1.0 + a[i]/1000.0); /* Multiplicative recurrence */
        prod2 = prod2 * (1.0 - b[i]/1000.0) * prod1; /* Cross-chain dependency */
        
        /* Additional operations to increase register pressure */
        a[i] = sum1 + prod1;
        b[i] = sum2 - prod2;
        c[i] = sum3 * (prod1 + prod2);
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2*stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 1, tmp2 = 2, tmp3 = 3;
    
    while (ptr1 < end && ptr2 < (end - stride) && ptr3 < (end - 2*stride)) {
        /* Multiple pointer-based recurrence chains */
        acc1 = acc1 * 2 + *ptr1;
        acc2 = acc2 * 3 + *ptr2 ^ acc1;    /* Bitwise mix with acc1 */
        acc3 = acc3 * 5 + *ptr3 | acc2;    /* Another operation mixing */
        
        tmp1 = tmp1 * 7 + acc1;
        tmp2 = tmp2 * 11 + acc2 * tmp1;
        tmp3 = tmp3 * 13 + acc3 - tmp2;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Store back to create anti-dependencies */
        *(ptr1-1) = acc1 + tmp1;
        *(ptr2-2) = acc2 ^ tmp2;
        *(ptr3-3) = acc3 | tmp3;
    }
    
    global_acc += acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3;
}

/* Test 4: Mixed integer operations with varying distances */
void test_mixed_ops_varying_distance(short *a, short *b, char *c, int n) {
    int sum1 = a[0], sum2 = b[0], sum3 = c[0];
    int acc1 = 1, acc2 = 2, acc3 = 3;
    
    for (int i = 2; i < n; i++) {
        /* Different dependency distances */
        sum1 = sum1 + a[i-1] * 3;          /* Distance 1 */
        sum2 = sum2 + b[i-2] * 5 + sum1;   /* Distance 2, depends on sum1 */
        sum3 = sum3 + c[i-1] * 7 ^ sum2;   /* Distance 1, bitwise with sum2 */
        
        /* More chains with different operations */
        acc1 = (acc1 << 2) + a[i] & 0xFF;
        acc2 = (acc2 << 3) + b[i] | acc1;
        acc3 = (acc3 << 1) + c[i] ^ acc2;
        
        /* Additional arithmetic to prevent optimization */
        a[i] = (sum1 + acc1) >> 1;
        b[i] = (sum2 - acc2) & 0x7FFF;
        c[i] = (sum3 * acc3) & 0xFF;
    }
    
    global_acc += sum1 + sum2 + sum3 + acc1 + acc2 + acc3;
}

/* Test 5: Nested loops with innermost hot loop */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int r = 1; r < rows; r++) {
        int *prev_row = mat + (r-1)*cols;
        int *curr_row = mat + r*cols;
        
        /* Innermost loop with carried dependencies */
        for (int c = 1; c < cols; c++) {
            /* 2D recurrence: depends on left and above elements */
            int left = curr_row[c-1];
            int above = prev_row[c];
            int diag = prev_row[c-1];
            
            /* Multiple computation chains */
            int val1 = left * 2 + above;
            int val2 = diag * 3 - left;
            int val3 = (above << 1) ^ diag;
            int val4 = val1 * val2 + val3;
            int val5 = val2 | val4;
            int val6 = val3 & val5;
            
            curr_row[c] = val1 + val2 + val3 + val4 + val5 + val6;
        }
    }
    
    /* Accumulate some values to prevent elimination */
    for (int i = 0; i < rows * cols; i += 64) {
        global_acc += mat[i];
    }
}

/* Test 6: Loop with manual unrolling hint */
void test_unrolled_loop(int *a, int *b, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int acc1 = a[0], acc2 = b[0], acc3 = 1, acc4 = 2;
    
    #pragma GCC unroll 4
    for (int i = 1; i < n; i++) {
        /* Unrolled body with interleaved dependencies */
        sum1 = sum1 + acc1 * a[i];
        acc1 = acc1 * 3 + b[i];
        
        sum2 = sum2 + acc2 * b[i];
        acc2 = acc2 * 5 + a[i-1];
        
        sum3 = sum3 ^ acc3;
        acc3 = acc3 * 7 + (a[i] & b[i]);
        
        sum4 = sum4 | acc4;
        acc4 = acc4 * 11 + (a[i] ^ b[i]);
        
        /* Cross dependencies between unrolled iterations */
        a[i] = sum1 + sum2;
        b[i] = sum3 - sum4;
    }
    
    global_acc += sum1 + sum2 + sum3 + sum4 + acc1 + acc2 + acc3 + acc4;
}

/* PowerPC-specific test with potential for FP register pressure */
#ifdef __powerpc__
void test_powerpc_fp_regpressure(double *a, double *b, int n) {
    double f1 = 1.0, f2 = 2.0, f3 = 3.0, f4 = 4.0;
    double f5 = 5.0, f6 = 6.0, f7 = 7.0, f8 = 8.0;
    
    for (int i = 1; i < n; i++) {
        /* Many FP operations to use lots of registers */
        f1 = f1 * 1.1 + a[i];
        f2 = f2 * 1.2 + b[i] * f1;
        f3 = f3 * 1.3 + a[i-1] + f2;
        f4 = f4 * 1.4 + b[i] - f3;
        f5 = f5 * 1.5 + f1 * f2;
        f6 = f6 * 1.6 + f3 / f4;
        f7 = f7 * 1.7 + f5 - f6;
        f8 = f8 * 1.8 + f7 * f1;
        
        a[i] = f1 + f3 + f5 + f7;
        b[i] = f2 + f4 + f6 + f8;
    }
    
    global_acc += (long long)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8);
}
#endif

/* ARM SVE-style test with unknown bounds at compile time */
#ifdef __ARM_FEATURE_SVE
void test_sve_unknown_bounds(int *a, int *b, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int acc1 = a[0], acc2 = b[0], acc3 = 1;
    
    /* Loop with runtime bound to trigger SVE vectorization */
    for (int i = 1; i < n; i++) {
        /* Multiple dependency chains */
        sum1 = sum1 + acc1 * a[i];
        acc1 = acc1 * 2 + b[i-1];
        
        sum2 = sum2 + acc2 * b[i];
        acc2 = acc2 * 3 + a[i];
        
        sum3 = sum3 ^ acc3;
        acc3 = acc3 * 5 + (a[i] & b[i]);
        
        /* Additional operations */
        a[i] = (sum1 << 1) + sum2;
        b[i] = (sum2 >> 1) ^ sum3;
    }
    
    global_acc += sum1 + sum2 + sum3 + acc1 + acc2 + acc3;
}
#endif

/* Main driver that runs all tests repeatedly */
int main() {
    /* Allocate and initialize test arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    double *darr3 = malloc(SIZE * sizeof(double));
    short *sarr1 = malloc(SIZE * sizeof(short));
    short *sarr2 = malloc(SIZE * sizeof(short));
    char *carr1 = malloc(SIZE * sizeof(char));
    int *matrix = malloc(SIZE * SIZE/4 * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        darr1[i] = (double)(rand() % 100) / 10.0;
        darr2[i] = (double)(rand() % 100) / 10.0;
        darr3[i] = (double)(rand() % 100) / 10.0;
        sarr1[i] = rand() % 1000;
        sarr2[i] = rand() % 1000;
        carr1[i] = rand() % 100;
    }
    
    for (int i = 0; i < SIZE * SIZE/4; i++) {
        matrix[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_recurrence_multi_chain(arr1, arr2, arr3, SIZE);
        test_float_accumulate(darr1, darr2, darr3, SIZE);
        test_pointer_chasing(arr1, SIZE, 4);
        test_mixed_ops_varying_distance(sarr1, sarr2, carr1, SIZE);
        test_nested_loops(matrix, 32, 32);
        test_unrolled_loop(arr1, arr2, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_fp_regpressure(darr1, darr2, SIZE);
        #endif
        
        #ifdef __ARM_FEATURE_SVE
        test_sve_unknown_bounds(arr1, arr2, SIZE);
        #endif
        
        /* Modify inputs slightly each iteration */
        if (iter % 100 == 0) {
            arr1[0] = rand() % 100;
            darr1[0] = (double)(rand() % 100) / 10.0;
        }
    }
    
    printf("Tests completed. Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(darr1);
    free(darr2);
    free(darr3);
    free(sarr1);
    free(sarr2);
    free(carr1);
    free(matrix);
    
    return 0;
}

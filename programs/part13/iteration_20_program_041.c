/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched
 * For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve test_modulo_sched.c -o test_modulo_sched
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multichain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains with different distances */
    int acc1 = a[0];
    int acc2 = a[1] + a[0];
    int acc3 = a[2];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: distance 1 recurrence */
        acc1 = acc1 * 3 + b[i];
        a[i] = acc1;
        
        /* Chain 2: distance 1 recurrence with different operation */
        acc2 = (acc2 ^ c[i]) + d[i];
        b[i] = acc2;
        
        /* Chain 3: distance 2 recurrence (uses i-2) */
        if (i >= 2) {
            acc3 = acc3 * 7 - a[i-2];
            c[i] = acc3;
        }
        
        /* Additional operations to increase register pressure */
        d[i] = (a[i] & 0xFF) | (b[i] << 8) | (c[i] << 16);
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(double *arr, double *brr, double *crr, int n) {
    int i;
    double sum1 = arr[0];
    double sum2 = brr[0];
    double prod1 = crr[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + arr[i] * brr[i];
        sum2 = sum2 * 0.99 - crr[i] / (arr[i] + 1.0);
        prod1 = prod1 * (1.0 + brr[i] * 0.001);
        
        /* Cross-chain dependencies to create anti-dependencies */
        arr[i] = sum1 + sum2;
        brr[i] = sum1 * prod1;
        crr[i] = sum2 / (prod1 + 0.5);
        
        /* Additional operations for register pressure */
        double temp = arr[i] * arr[i-1] + brr[i] * brr[i-1];
        global_acc += (long long)temp;
    }
}

/* Test 3: Mixed integer/float with pointer chasing */
void test_mixed_pointer_chasing(int *int_arr, float *float_arr, 
                                double *double_arr, int n) {
    int *ip = int_arr;
    float *fp = float_arr;
    double *dp = double_arr;
    
    int int_sum = *ip;
    float float_sum = *fp;
    double double_prod = *dp;
    
    for (int i = 1; i < n; i++) {
        /* Pointer chasing with different strides */
        ip += 1;
        fp += 1;
        dp += 1;
        
        /* Mixed-type recurrence relations */
        int_sum = int_sum * 5 + *ip;
        float_sum = float_sum * 1.5f + *fp;
        double_prod = double_prod * (1.0 + *dp * 0.01);
        
        /* Store results creating anti-dependencies */
        *(ip-1) = int_sum;
        *(fp-1) = float_sum;
        *(dp-1) = double_prod;
        
        /* Cross-type operations */
        *ip = (int)(float_sum * 10.0f);
        *fp = (float)(double_prod * 0.1);
        *dp = int_sum * 0.01;
    }
    
    global_acc += (long long)(int_sum + float_sum + double_prod);
}

/* Test 4: Nested loops with inner loop carrying dependencies */
void test_nested_loops(int *matrix, int rows, int cols) {
    for (int i = 1; i < rows; i++) {
        int prev_row = (i-1) * cols;
        int curr_row = i * cols;
        
        /* Inner loop with carried dependency across columns */
        for (int j = 1; j < cols; j++) {
            /* Multiple dependency chains in inner loop */
            int diag = matrix[prev_row + (j-1)];
            int left = matrix[curr_row + (j-1)];
            int up = matrix[prev_row + j];
            
            /* Complex recurrence with multiple uses */
            int val = (diag * 3 + left * 2 + up) / 2;
            val = (val ^ (left << 3)) + (up & 0xFF);
            val = val * 7 - (diag % 256);
            
            matrix[curr_row + j] = val;
            
            /* Additional operations for register pressure */
            int temp = matrix[curr_row + j] + matrix[prev_row + j-1];
            global_acc += temp;
        }
    }
}

/* Test 5: Loop with compile-time unrolling hint */
void test_unrolled_recurrence(short *a, short *b, short *c, int n) {
    int sum1 = a[0];
    int sum2 = b[0];
    int sum3 = c[0];
    
    /* Manual unrolling to increase operations per iteration */
    #pragma GCC unroll 4
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains */
        sum1 = sum1 * 11 + a[i];
        sum2 = sum2 * 13 + b[i];
        sum3 = sum3 * 17 + c[i];
        
        /* Cross assignments creating register pressure */
        a[i-1] = (short)(sum1 & 0xFFFF);
        b[i-1] = (short)(sum2 & 0xFFFF);
        c[i-1] = (short)(sum3 & 0xFFFF);
        
        /* Additional arithmetic */
        int temp = (sum1 ^ sum2) | (sum3 << 2);
        global_acc += temp;
    }
}

/* Test 6: PowerPC specific - double precision with FMA-like patterns */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    double acc1 = a[0];
    double acc2 = b[0];
    double acc3 = c[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple double precision chains */
        acc1 = acc1 * 1.1 + a[i] * b[i];
        acc2 = acc2 * 0.9 - c[i] / a[i];
        acc3 = acc3 * (1.0 + b[i] * 0.001);
        
        /* Store with anti-dependencies */
        a[i-1] = acc1;
        b[i-1] = acc2;
        c[i-1] = acc3;
        
        /* Additional operations */
        double temp = acc1 * acc2 + acc3;
        global_acc += (long long)temp;
    }
}
#endif

/* Test 7: Variable distance dependencies */
void test_variable_distance(int *arr, int *mask, int n) {
    int hist[4] = {0};
    
    for (int i = 0; i < n; i++) {
        int idx = mask[i] & 0x3;
        
        /* Variable distance recurrence based on mask */
        hist[idx] = hist[idx] * 3 + arr[i];
        
        /* Multiple consumers of hist values */
        arr[i] = hist[0] + hist[1] + hist[2] + hist[3];
        
        /* Additional operations */
        for (int j = 0; j < 4; j++) {
            hist[j] = (hist[j] ^ (arr[i] >> (j*4))) & 0xFFF;
        }
        
        global_acc += hist[0];
    }
}

/* Main test driver */
int main() {
    /* Allocate and initialize test arrays */
    int *int_arr1 = malloc(SIZE * sizeof(int));
    int *int_arr2 = malloc(SIZE * sizeof(int));
    int *int_arr3 = malloc(SIZE * sizeof(int));
    int *int_arr4 = malloc(SIZE * sizeof(int));
    
    double *double_arr1 = malloc(SIZE * sizeof(double));
    double *double_arr2 = malloc(SIZE * sizeof(double));
    double *double_arr3 = malloc(SIZE * sizeof(double));
    
    float *float_arr = malloc(SIZE * sizeof(float));
    short *short_arr1 = malloc(SIZE * sizeof(short));
    short *short_arr2 = malloc(SIZE * sizeof(short));
    short *short_arr3 = malloc(SIZE * sizeof(short));
    
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    int *mask_arr = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        int_arr1[i] = rand() % 1000;
        int_arr2[i] = rand() % 1000;
        int_arr3[i] = rand() % 1000;
        int_arr4[i] = rand() % 1000;
        
        double_arr1[i] = (rand() % 1000) / 10.0;
        double_arr2[i] = (rand() % 1000) / 10.0;
        double_arr3[i] = (rand() % 1000) / 10.0;
        
        float_arr[i] = (rand() % 1000) / 10.0f;
        short_arr1[i] = rand() % 1000;
        short_arr2[i] = rand() % 1000;
        short_arr3[i] = rand() % 1000;
        
        mask_arr[i] = rand() % 16;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = rand() % 1000;
    }
    
    /* Run tests multiple times to ensure hot loop compilation */
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Cycle through different tests to exercise various patterns */
        switch (iter % 7) {
            case 0:
                test_int_recurrence_multichain(int_arr1, int_arr2, int_arr3, int_arr4, SIZE);
                break;
            case 1:
                test_float_accumulation(double_arr1, double_arr2, double_arr3, SIZE);
                break;
            case 2:
                test_mixed_pointer_chasing(int_arr1, float_arr, double_arr1, SIZE);
                break;
            case 3:
                test_nested_loops(matrix, 64, 64);
                break;
            case 4:
                test_unrolled_recurrence(short_arr1, short_arr2, short_arr3, SIZE);
                break;
            case 5:
                #ifdef __powerpc__
                test_powerpc_double(double_arr1, double_arr2, double_arr3, SIZE);
                #else
                test_variable_distance(int_arr1, mask_arr, SIZE);
                #endif
                break;
            case 6:
                test_variable_distance(int_arr2, mask_arr, SIZE);
                break;
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Output results to prevent optimization */
    printf("Global accumulator: %lld\n", global_acc);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    printf("Iterations: %d\n", ITERATIONS);
    
    /* Cleanup */
    free(int_arr1);
    free(int_arr2);
    free(int_arr3);
    free(int_arr4);
    free(double_arr1);
    free(double_arr2);
    free(double_arr3);
    free(float_arr);
    free(short_arr1);
    free(short_arr2);
    free(short_arr3);
    free(matrix);
    free(mask_arr);
    
    return 0;
}

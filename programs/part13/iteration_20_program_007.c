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

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int i;
    /* Multiple independent recurrence chains to increase register pressure */
    int acc1 = a[0];
    int acc2 = a[1] * 2;
    int acc3 = a[2] + 1;
    int acc4 = a[3] - 1;
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple recurrence */
        acc1 = acc1 + b[i] * 3;
        a[i] = acc1;
        
        /* Chain 2: Recurrence with distance 1 */
        acc2 = (acc2 * 7) ^ b[i-1];
        c[i] = acc2;
        
        /* Chain 3: Another recurrence */
        acc3 = acc3 + (b[i] << 2);
        a[i] += acc3;
        
        /* Chain 4: Mixed operations */
        acc4 = (acc4 & 0xFF) | (b[i] << 8);
        c[i] ^= acc4;
        
        /* Additional operations to increase register pressure */
        a[i] += (acc1 * acc2) >> 3;
        c[i] -= (acc3 & acc4) * 5;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = a[1];
    double prod1 = b[0];
    double prod2 = b[1];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + b[i];
        sum2 = sum2 * 0.99 - c[i];
        prod1 = prod1 * (1.0 + b[i] * 0.001);
        prod2 = prod2 / (1.0 + c[i] * 0.001);
        
        /* Cross-chain dependencies */
        a[i] = sum1 * prod1 + sum2 * prod2;
        b[i] = sum1 - prod2;
        c[i] = sum2 + prod1;
        
        /* Additional operations */
        a[i] += (sum1 * sum2) * 0.5;
        b[i] *= (prod1 + prod2) * 0.25;
    }
    
    global_acc += (long long)(sum1 + sum2 + prod1 + prod2);
}

/* Test 3: Mixed integer operations with strided access */
void test_mixed_strided(int *a, int *b, short *c, char *d, int n) {
    int i;
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int temp1 = a[0], temp2 = b[0];
    
    /* Strided access pattern */
    for (i = 1; i < n - 4; i += 2) {
        /* Multiple recurrence relations */
        acc1 = acc1 + a[i] * 3 - a[i-1];
        acc2 = acc2 ^ (b[i] + acc1);
        acc3 = acc3 | (c[i*2] & 0xFF);
        
        /* Carried dependencies with different distances */
        temp1 = temp1 * 7 + d[i];
        temp2 = (temp2 << 3) | (d[i+1] & 0xF);
        
        /* Store results creating anti-dependencies */
        a[i] = acc1 + temp1;
        b[i] = acc2 ^ temp2;
        c[i*2] = (short)(acc3 & 0xFFFF);
        
        /* More operations to increase pressure */
        acc1 = (acc1 >> 1) + temp2;
        acc2 = acc2 * 11 - temp1;
        acc3 = acc3 + (a[i] & b[i]);
    }
    
    global_acc += acc1 + acc2 + acc3 + temp1 + temp2;
}

/* Test 4: Pointer-chasing with arithmetic */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = data;
    int *ptr2 = data + 1;
    int *ptr3 = data + 2;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    for (i = 0; i < n - 3; i++) {
        /* Pointer chasing with carried dependencies */
        sum1 = sum1 + *ptr1 * 2;
        sum2 = sum2 ^ (*ptr2 + sum1);
        sum3 = sum3 | (*ptr3 & sum2);
        
        /* Update pointers with stride */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computations */
        sum1 = (sum1 << 1) | (sum2 & 1);
        sum2 = sum2 + (sum3 >> 2);
        sum3 = sum3 * 3 - sum1;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: Nested loops with inner loop modulo scheduling */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    int acc;
    
    for (i = 0; i < n; i++) {
        acc = a[i];
        /* Inner loop with carried dependency */
        for (j = 0; j < m; j++) {
            /* Multiple operations to create register pressure */
            acc = acc * 3 + b[j];
            c[j] = acc ^ c[j];
            acc = acc + (b[j] << 1);
            c[j] = c[j] - acc;
            acc = acc | (c[j] & 0xFF);
        }
        a[i] = acc;
    }
    
    global_acc += acc;
}

/* Test 6: SIMD-style operations (triggers vector modulo scheduling) */
void test_simd_style(float *a, float *b, float *c, int n) {
    int i;
    float sum1 = a[0], sum2 = a[1];
    float diff1 = b[0], diff2 = b[1];
    
    /* Loop designed for vectorization and modulo scheduling */
    for (i = 1; i < n - 1; i++) {
        /* Independent but similar operations - vectorizable */
        sum1 = sum1 + b[i] * c[i];
        sum2 = sum2 + b[i+1] * c[i-1];
        diff1 = diff1 - a[i] * 0.5f;
        diff2 = diff2 - a[i-1] * 0.25f;
        
        /* Cross dependencies */
        a[i] = sum1 * diff1;
        b[i] = sum2 * diff2;
        c[i] = sum1 + sum2 + diff1 + diff2;
        
        /* More operations */
        sum1 = sum1 * 1.1f;
        sum2 = sum2 * 0.9f;
        diff1 = diff1 + 0.01f;
        diff2 = diff2 - 0.01f;
    }
    
    global_acc += (long long)(sum1 + sum2 + diff1 + diff2);
}

/* Test 7: PowerPC specific patterns with double precision */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0], acc2 = a[1];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Double precision operations that use FP registers */
        acc1 = acc1 * 1.01 + b[i];
        acc2 = acc2 * 0.99 - c[i];
        prod1 = prod1 * (1.0 + b[i]);
        prod2 = prod2 / (1.0 + c[i]);
        
        /* Complex dependency web */
        a[i] = acc1 * prod1 + acc2 * prod2;
        b[i] = (acc1 - acc2) * prod1;
        c[i] = (acc1 + acc2) * prod2;
        
        /* More double ops */
        acc1 = acc1 + prod2 * 0.5;
        acc2 = acc2 - prod1 * 0.5;
        prod1 = prod1 * 1.001;
        prod2 = prod2 * 0.999;
    }
    
    global_acc += (long long)(acc1 + acc2 + prod1 + prod2);
}
#endif

/* Test 8: Manual unrolling to increase operations per iteration */
void test_manual_unroll(int *a, int *b, int n) {
    int i;
    int acc1 = a[0], acc2 = a[1], acc3 = a[2], acc4 = a[3];
    
    /* Manually unrolled loop */
    for (i = 4; i < n - 4; i += 4) {
        /* Unrolled iteration 1 */
        acc1 = acc1 * 3 + b[i];
        a[i] = acc1 ^ a[i-1];
        
        /* Unrolled iteration 2 */
        acc2 = acc2 * 5 + b[i+1];
        a[i+1] = acc2 ^ a[i];
        
        /* Unrolled iteration 3 */
        acc3 = acc3 * 7 + b[i+2];
        a[i+2] = acc3 ^ a[i+1];
        
        /* Unrolled iteration 4 */
        acc4 = acc4 * 11 + b[i+3];
        a[i+3] = acc4 ^ a[i+2];
        
        /* Cross dependencies between unrolled iterations */
        acc1 = acc1 + acc4;
        acc2 = acc2 ^ acc1;
        acc3 = acc3 | acc2;
        acc4 = acc4 & acc3;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *int_data1 = (int*)malloc(SIZE * sizeof(int));
    int *int_data2 = (int*)malloc(SIZE * sizeof(int));
    int *int_data3 = (int*)malloc(SIZE * sizeof(int));
    double *double_data1 = (double*)malloc(SIZE * sizeof(double));
    double *double_data2 = (double*)malloc(SIZE * sizeof(double));
    double *double_data3 = (double*)malloc(SIZE * sizeof(double));
    short *short_data = (short*)malloc(SIZE * 2 * sizeof(short));
    char *char_data = (char*)malloc(SIZE * sizeof(char));
    
    /* Initialize with pattern */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        int_data1[i] = rand() % 100;
        int_data2[i] = rand() % 100;
        int_data3[i] = rand() % 100;
        double_data1[i] = (double)(rand() % 100) / 10.0;
        double_data2[i] = (double)(rand() % 100) / 10.0;
        double_data3[i] = (double)(rand() % 100) / 10.0;
        short_data[i] = (short)(rand() % 100);
        char_data[i] = (char)(rand() % 100);
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        /* Cycle through different test patterns */
        switch (i % 8) {
            case 0:
                test_int_recurrence_multi_chain(int_data1, int_data2, int_data3, SIZE);
                break;
            case 1:
                test_float_accumulate(double_data1, double_data2, double_data3, SIZE);
                break;
            case 2:
                test_mixed_strided(int_data1, int_data2, short_data, char_data, SIZE);
                break;
            case 3:
                test_pointer_chasing(int_data1, SIZE);
                break;
            case 4:
                test_nested_loops(int_data1, int_data2, int_data3, 10, SIZE/10);
                break;
            case 5:
                test_simd_style((float*)double_data1, (float*)double_data2, 
                               (float*)double_data3, SIZE);
                break;
            case 6:
                test_manual_unroll(int_data1, int_data2, SIZE);
                break;
            case 7:
#ifdef __powerpc__
                test_powerpc_double(double_data1, double_data2, double_data3, SIZE);
#endif
                break;
        }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(int_data1);
    free(int_data2);
    free(int_data3);
    free(double_data1);
    free(double_data2);
    free(double_data3);
    free(short_data);
    free(char_data);
    
    return 0;
}

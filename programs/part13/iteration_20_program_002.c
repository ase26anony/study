/* test_modulo_sched.c
 * 
 * This program is designed to trigger GCC's modulo scheduler register move
 * logic, specifically targeting the uncovered code in schedule_reg_move()
 * that prints debug information about register move scheduling.
 *
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
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
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains */
    int x1 = a[0];
    int x2 = b[0];
    int x3 = c[0];
    int x4 = d[0];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: simple recurrence */
        x1 = x1 * 3 + b[i];
        
        /* Chain 2: recurrence with more operations */
        x2 = (x2 << 2) ^ (x2 + c[i]);
        
        /* Chain 3: mixed operations recurrence */
        x3 = x3 + (x3 & 0xFF) * d[i];
        
        /* Chain 4: another recurrence */
        x4 = x4 * 5 - a[i] + 7;
        
        /* Store results to create register pressure */
        a[i] = x1;
        b[i] = x2;
        c[i] = x3;
        d[i] = x4;
    }
    
    /* Use results to prevent optimization */
    global_acc += x1 + x2 + x3 + x4;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0];
    double acc2 = b[0];
    double acc3 = c[0];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        acc1 = acc1 * 1.01 + a[i] * b[i];
        acc2 = acc2 * 0.99 + c[i] * a[i];
        acc3 = acc3 * 1.02 + b[i] * c[i];
        
        /* Additional operations to increase register pressure */
        double t1 = acc1 * acc2;
        double t2 = acc2 * acc3;
        double t3 = acc3 * acc1;
        
        a[i] = t1;
        b[i] = t2;
        c[i] = t3;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3);
}

/* Test 3: Mixed integer operations with strided access */
void test_mixed_ops_strided(short *arr, int *indices, int n) {
    int i;
    int sum1 = arr[0];
    int sum2 = arr[1];
    int sum3 = arr[2];
    
    for (i = 0; i < n; i++) {
        int idx = indices[i] % (n - 3);
        
        /* Multiple operations with carried dependencies */
        sum1 = sum1 + (sum1 >> 3) + arr[idx];
        sum2 = sum2 * 2 - (sum2 & 0xFF) + arr[idx + 1];
        sum3 = (sum3 ^ sum1) + (sum3 & sum2) + arr[idx + 2];
        
        /* More operations to increase pressure */
        int t1 = sum1 * sum2;
        int t2 = sum2 * sum3;
        int t3 = sum3 * sum1;
        
        arr[idx] = t1 & 0xFFFF;
        arr[idx + 1] = t2 & 0xFFFF;
        arr[idx + 2] = t3 & 0xFFFF;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Pointer chasing with arithmetic (triggers register moves) */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = &data[0];
    int *ptr2 = &data[1];
    int *ptr3 = &data[2];
    int *end = &data[n-3];
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    while (ptr1 < end) {
        /* Pointer chasing with dependencies */
        acc1 = acc1 + *ptr1 + (acc1 >> 2);
        acc2 = acc2 ^ *ptr2 + (acc2 << 1);
        acc3 = acc3 * 3 + *ptr3 - (acc3 & 0xFF);
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional computation */
        *ptr1 = acc1;
        *ptr2 = acc2;
        *ptr3 = acc3;
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 5: Nested loops with inner loop having carried dependency */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = a[i];
        
        /* Inner loop with carried dependency */
        for (j = 0; j < m; j++) {
            /* Multiple operations to create register pressure */
            acc = acc * 2 + b[j] - c[j];
            int t1 = acc & 0xFF;
            int t2 = (acc >> 8) & 0xFF;
            int t3 = (acc >> 16) & 0xFF;
            
            /* Use all temporaries */
            b[j] = b[j] + t1;
            c[j] = c[j] ^ t2;
            a[i] = a[i] * t3;
        }
        
        a[i] = acc;
    }
    
    /* Simple reduction to use results */
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += a[i];
    }
    global_acc += sum;
}

/* Test 6: PowerPC specific - double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = a[1];
    
    for (i = 1; i < n; i++) {
        /* Multiple double precision operations */
        sum1 = sum1 * 1.5 + a[i];
        sum2 = sum2 / 1.3 + b[i];
        sum3 = sum3 * 2.0 - a[i] * b[i];
        
        /* Cross dependencies */
        double t1 = sum1 * sum2;
        double t2 = sum2 * sum3;
        double t3 = sum3 * sum1;
        
        a[i] = t1;
        b[i] = t2;
        
        /* More operations */
        sum1 = sum1 + t3;
        sum2 = sum2 - t1;
        sum3 = sum3 * t2;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
void test_manual_unroll(int *a, int *b, int n) {
    int i;
    int x0 = a[0], x1 = a[1], x2 = a[2], x3 = a[3];
    int y0 = b[0], y1 = b[1], y2 = b[2], y3 = b[3];
    
    for (i = 4; i < n; i += 4) {
        /* Unrolled operations with dependencies */
        x0 = x0 * 3 + a[i];
        y0 = y0 * 2 + b[i];
        
        x1 = x1 * 5 + a[i+1] + x0;
        y1 = y1 * 3 + b[i+1] ^ y0;
        
        x2 = x2 * 7 + a[i+2] * x1;
        y2 = y2 * 11 + b[i+2] & y1;
        
        x3 = x3 * 13 + a[i+3] - x2;
        y3 = y3 * 17 + b[i+3] | y2;
        
        /* Store results */
        a[i] = x0; b[i] = y0;
        a[i+1] = x1; b[i+1] = y1;
        a[i+2] = x2; b[i+2] = y2;
        a[i+3] = x3; b[i+3] = y3;
        
        /* Rotate values for next iteration */
        int tx = x0; x0 = x1; x1 = x2; x2 = x3; x3 = tx;
        int ty = y0; y0 = y1; y1 = y2; y2 = y3; y3 = ty;
    }
    
    global_acc += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
}

int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *a_int = malloc(SIZE * sizeof(int));
    int *b_int = malloc(SIZE * sizeof(int));
    int *c_int = malloc(SIZE * sizeof(int));
    int *d_int = malloc(SIZE * sizeof(int));
    double *a_double = malloc(SIZE * sizeof(double));
    double *b_double = malloc(SIZE * sizeof(double));
    double *c_double = malloc(SIZE * sizeof(double));
    short *arr_short = malloc(SIZE * sizeof(short));
    int *indices = malloc(SIZE * sizeof(int));
    
    if (!a_int || !b_int || !c_int || !d_int || 
        !a_double || !b_double || !c_double ||
        !arr_short || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (i = 0; i < SIZE; i++) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        c_int[i] = rand() % 1000;
        d_int[i] = rand() % 1000;
        a_double[i] = (double)(rand() % 1000) / 10.0;
        b_double[i] = (double)(rand() % 1000) / 10.0;
        c_double[i] = (double)(rand() % 1000) / 10.0;
        arr_short[i] = rand() % 1000;
        indices[i] = rand() % (SIZE - 10);
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        /* Alternate between different tests to exercise different patterns */
        switch (i % 7) {
            case 0:
                test_int_recurrence_multi_chain(a_int, b_int, c_int, d_int, SIZE);
                break;
            case 1:
                test_float_accumulate(a_double, b_double, c_double, SIZE);
                break;
            case 2:
                test_mixed_ops_strided(arr_short, indices, SIZE);
                break;
            case 3:
                test_pointer_chasing(a_int, SIZE);
                break;
            case 4:
                test_nested_loops(a_int, b_int, c_int, 64, 16);
                break;
            case 5:
                test_manual_unroll(a_int, b_int, SIZE);
                break;
            case 6:
#ifdef __powerpc__
                test_powerpc_double(a_double, b_double, SIZE);
#endif
                break;
        }
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Tests completed in %.2f seconds\n", cpu_time_used);
    printf("Global accumulator: %lld\n", global_acc);
    
    /* Clean up */
    free(a_int);
    free(b_int);
    free(c_int);
    free(d_int);
    free(a_double);
    free(b_double);
    free(c_double);
    free(arr_short);
    free(indices);
    
    return 0;
}

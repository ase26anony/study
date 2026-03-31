/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
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

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains to increase register pressure */
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    for (i = 2; i < n; i++) {
        /* Chain 1: distance-1 dependency */
        x1 = x1 * 3 + a[i];
        a[i] = x1;
        
        /* Chain 2: distance-1 with different operation */
        x2 = (x2 << 2) ^ b[i];
        b[i] = x2;
        
        /* Chain 3: distance-1 with multiply */
        x3 = x3 * 7 + c[i] * 5;
        c[i] = x3;
        
        /* Chain 4: distance-2 dependency pattern */
        y1 = y1 + a[i-1] * 2;
        d[i] = y1;
        
        /* More operations to increase register pressure */
        y2 = y2 ^ (b[i-1] << 1);
        y3 = y3 * 11 - c[i-1];
        y4 = y4 + (a[i-1] & b[i-1]) | c[i-1];
    }
    
    /* Use results to prevent optimization */
    global_acc += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0], acc2 = b[0], acc3 = c[0];
    double tmp1 = a[1], tmp2 = b[1], tmp3 = c[1];
    
    for (i = 2; i < n; i++) {
        /* Multiple FP dependency chains */
        acc1 = acc1 * 1.01 + a[i];
        acc2 = acc2 * 0.99 + b[i] * acc1;
        acc3 = acc3 + c[i] * acc2;
        
        /* Cross-iteration dependencies */
        tmp1 = tmp1 + a[i-1] * b[i];
        tmp2 = tmp2 * 0.95 + c[i-1];
        tmp3 = tmp3 - a[i-1] + b[i-1] * 1.5;
        
        /* Store results creating anti-dependencies */
        a[i-1] = tmp1;
        b[i-1] = tmp2;
        c[i-1] = tmp3;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int acc1 = *ptr1, acc2 = *ptr2, acc3 = *ptr3;
    
    while (ptr3 < end) {
        /* Pointer chasing with carried dependencies */
        acc1 = acc1 * 2 + *ptr1;
        acc2 = (acc2 << 1) ^ *ptr2;
        acc3 = acc3 + *ptr3 * acc1;
        
        sum1 += acc1;
        sum2 += acc2;
        sum3 += acc3;
        
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Additional operations to increase register pressure */
        acc1 = acc1 ^ sum1;
        acc2 = acc2 + sum2;
        acc3 = acc3 * 3 - sum3;
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with unrolling hint */
void test_mixed_ops_unrolled(short *a, short *b, int *c, int n) {
    int i;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int acc1 = a[0], acc2 = b[0], acc3 = c[0];
    
    /* Manual unrolling to increase operations per iteration */
    for (i = 1; i < n - 3; i += 2) {
        /* First set of operations */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 ^ (b[i] << 1);
        acc3 = acc3 + c[i] * 2;
        
        /* Dependency on previous iteration's values */
        sum1 = sum1 + acc1 * a[i-1];
        sum2 = sum2 ^ (acc2 & b[i-1]);
        sum3 = sum3 + acc3 - c[i-1];
        
        /* Second set (next iteration in unrolled loop) */
        acc1 = acc1 + a[i+1] * 5;
        acc2 = acc2 * 7 - b[i+1];
        acc3 = acc3 ^ c[i+1];
        
        sum1 = sum1 * 2 + acc1;
        sum2 = sum2 - acc2;
        sum3 = sum3 & (acc3 | 0xFF);
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: PowerPC specific - double precision operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple double-precision dependency chains */
        acc1 = acc1 * 1.1 + a[i];
        acc2 = acc2 * 0.9 + b[i] * acc1;
        acc3 = acc3 + a[i-1] * b[i];
        acc4 = acc4 * 1.05 - b[i-1] + acc2;
        
        /* Cross dependencies */
        a[i] = acc1 + acc3;
        b[i] = acc2 * acc4;
        
        /* More operations for register pressure */
        acc1 = acc1 - b[i] * 0.5;
        acc3 = acc3 * 1.01 + a[i];
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3 + acc4);
}
#endif

/* Test 6: Array accumulation with variable distance */
void test_variable_distance(int *a, int *b, int *c, int n, int dist) {
    int i;
    int history[4] = {0};
    int sum = 0;
    
    for (i = 0; i < n; i++) {
        /* Variable distance dependency */
        int idx = i % 4;
        history[idx] = history[(idx + dist) % 4] * 2 + a[i];
        
        /* Multiple operations */
        b[i] = history[idx] + b[i] * 3;
        c[i] = c[i] ^ history[(idx + 1) % 4];
        
        /* Accumulate with dependency */
        sum = sum * 5 + history[idx] + b[i] + c[i];
        
        /* Additional arithmetic chains */
        history[(idx + 2) % 4] = history[idx] - history[(idx + 1) % 4];
    }
    
    global_acc += sum;
}

/* Main test driver */
int main() {
    int i, j;
    clock_t start, end;
    
    /* Allocate and initialize test arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    short *arr_short1 = malloc(SIZE * sizeof(short));
    short *arr_short2 = malloc(SIZE * sizeof(short));
    double *arr_double1 = malloc(SIZE * sizeof(double));
    double *arr_double2 = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        arr4[i] = rand() % 100;
        arr_short1[i] = rand() % 100;
        arr_short2[i] = rand() % 100;
        arr_double1[i] = (double)(rand() % 100) / 10.0;
        arr_double2[i] = (double)(rand() % 100) / 10.0;
    }
    
    printf("Starting modulo scheduling tests...\n");
    start = clock();
    
    /* Run multiple iterations to ensure hot loop compilation */
    for (j = 0; j < ITERS; j++) {
        /* Alternate between different test patterns */
        switch (j % 6) {
            case 0:
                test_int_recurrence_multi_chain(arr1, arr2, arr3, arr4, SIZE);
                break;
            case 1:
                test_float_accumulate(arr_double1, arr_double2, arr_double1, SIZE);
                break;
            case 2:
                test_pointer_chasing(arr1, SIZE, 4);
                break;
            case 3:
                test_mixed_ops_unrolled(arr_short1, arr_short2, arr1, SIZE);
                break;
            case 4:
                test_variable_distance(arr1, arr2, arr3, SIZE, 2);
                break;
            case 5:
                #ifdef __powerpc__
                test_powerpc_double(arr_double1, arr_double2, SIZE);
                #else
                test_int_recurrence_multi_chain(arr2, arr3, arr4, arr1, SIZE);
                #endif
                break;
        }
    }
    
    end = clock();
    
    printf("Tests completed in %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    printf("Global accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr_short1);
    free(arr_short2);
    free(arr_double1);
    free(arr_double2);
    
    return 0;
}

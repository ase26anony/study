/* test_modulo_sched.c
 * Test program to cover register move scheduling in GCC's modulo scheduler
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched
 * Or for ARM: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve -ftree-vectorize test_modulo_sched.c -o test_modulo_sched
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
    /* Multiple independent recurrence chains to increase register pressure */
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    int acc4 = d[0];
    int acc5 = a[1];
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple recurrence */
        acc1 = acc1 + b[i] * 3;
        a[i] = acc1;
        
        /* Chain 2: Recurrence with different distance */
        acc2 = acc2 * 2 - c[i];
        b[i] = acc2;
        
        /* Chain 3: Another recurrence */
        acc3 = (acc3 << 1) | (d[i] & 1);
        c[i] = acc3;
        
        /* Chain 4: Mixed operations */
        acc4 = (acc4 + d[i]) * 7;
        d[i] = acc4;
        
        /* Chain 5: Cross-iteration dependency with offset */
        acc5 = acc5 + a[i-1] * 5;
        if (i < n-1) a[i+1] = acc5;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4 + acc5;
}

/* Test 2: Floating-point accumulation with carried dependencies */
void test_float_accumulate(double *a, double *b, double *c, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = c[0];
    double sum4 = a[1];
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + b[i];
        a[i] = sum1;
        
        sum2 = sum2 + c[i] * sum1;
        b[i] = sum2;
        
        sum3 = sum3 * 0.99 - a[i-1];
        c[i] = sum3;
        
        sum4 = (sum4 + b[i-1]) * 1.5;
        if (i < n-1) a[i+1] = sum4;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + sum4);
}

/* Test 3: Mixed integer operations with strided access */
void test_mixed_ops_strided(int *arr, int n, int stride) {
    int i, j;
    int acc1 = arr[0];
    int acc2 = arr[1];
    int acc3 = arr[2];
    
    /* Manual unrolling to increase register pressure */
    for (i = 3; i < n - 3; i += 4) {
        /* Group 1 - various operations with carried dependencies */
        acc1 = (acc1 * 3 + arr[i]) >> 1;
        arr[i] = acc1;
        
        acc2 = acc2 + (arr[i+1] & 0xFF) * 5;
        arr[i+1] = acc2;
        
        acc3 = (acc3 << 2) | (arr[i+2] & 3);
        arr[i+2] = acc3;
        
        /* Cross-lane dependency */
        int temp = acc1 + acc2;
        arr[i+3] = temp * acc3;
        
        /* Additional operations to increase pressure */
        acc1 = acc1 ^ (arr[i] * 7);
        acc2 = acc2 - (arr[i+1] / 3);
        acc3 = acc3 | (arr[i+2] << 4);
    }
    
    global_acc += acc1 + acc2 + acc3;
}

/* Test 4: Pointer-chasing with arithmetic */
void test_pointer_chasing(int *data, int n) {
    int *ptr1 = data;
    int *ptr2 = data + 1;
    int *ptr3 = data + 2;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < n - 3; i++) {
        /* Multiple pointer chains with dependencies */
        sum1 = sum1 * 2 + *ptr1;
        sum2 = sum2 + (*ptr2 << 1);
        sum3 = sum3 * 3 - *ptr3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 1;
        
        /* Cross-iteration store with dependency */
        if (i > 0) {
            *(ptr1 - 1) = sum1 + sum2;
        }
    }
    
    global_acc += sum1 + sum2 + sum3;
}

/* Test 5: Nested loops with inner loop being modulo-scheduled */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        int acc = a[i];
        int acc2 = b[i];
        
        /* Inner loop with carried dependency - target for modulo scheduling */
        for (j = 0; j < m; j++) {
            /* Multiple operations with register pressure */
            acc = acc * 3 + c[j];
            acc2 = acc2 + (acc >> 2);
            
            /* Store with dependency */
            if (j > 0) {
                c[j-1] = acc + acc2;
            }
        }
        
        a[i] = acc;
        b[i] = acc2;
    }
    
    /* Use results */
    for (i = 0; i < n; i++) {
        global_acc += a[i] + b[i];
    }
}

/* Test 6: PowerPC specific patterns with double operations */
#ifdef __powerpc__
void test_powerpc_double(double *a, double *b, int n) {
    int i;
    double sum1 = a[0];
    double sum2 = b[0];
    double sum3 = a[1];
    double sum4 = b[1];
    
    for (i = 1; i < n; i++) {
        /* Multiple double precision chains */
        sum1 = sum1 * 1.5 + b[i];
        sum2 = sum2 - a[i] * 0.5;
        sum3 = sum3 * 2.0 + sum1;
        sum4 = sum4 / 1.1 + sum2;
        
        /* Store results with dependencies */
        a[i] = sum1 + sum3;
        b[i] = sum2 + sum4;
        
        /* Additional operation to increase pressure */
        sum1 = sum1 * sum2;
        sum3 = sum3 - sum4;
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + sum4);
}
#endif

/* Test 7: Vector-style operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
void test_vector_style(int *a, int *b, int *c, int n) {
    int i;
    int acc1 = a[0];
    int acc2 = b[0];
    int acc3 = c[0];
    int acc4 = a[1];
    
    /* Loop with strided accesses and multiple accumulators */
    for (i = 1; i < n - 4; i += 2) {
        /* Multiple dependency chains */
        acc1 = acc1 + b[i] * 7;
        a[i] = acc1;
        
        acc2 = acc2 * 3 - c[i+1];
        b[i+1] = acc2;
        
        acc3 = (acc3 << 1) + a[i-1];
        c[i] = acc3;
        
        acc4 = acc4 + (b[i] & c[i+1]);
        a[i+2] = acc4;
        
        /* Additional operations */
        acc1 = acc1 ^ (acc2 >> 3);
        acc3 = acc3 | (acc4 << 2);
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}
#endif

/* Main test driver */
int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    int *arr3 = (int*)malloc(SIZE * sizeof(int));
    int *arr4 = (int*)malloc(SIZE * sizeof(int));
    double *darr1 = (double*)malloc(SIZE * sizeof(double));
    double *darr2 = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !darr1 || !darr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
        darr1[i] = i * 0.5;
        darr2[i] = i * 0.25;
    }
    
    start = clock();
    
    /* Run multiple test functions many times to ensure hot loops */
    for (i = 0; i < ITERATIONS; i++) {
        test_int_recurrence_multichain(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulate(darr1, darr2, darr1, SIZE); /* Reuse darr1 as third param */
        test_mixed_ops_strided(arr1, SIZE, 2);
        test_pointer_chasing(arr2, SIZE);
        test_nested_loops(arr3, arr4, arr1, 64, 16);
        
        #ifdef __powerpc__
        test_powerpc_double(darr1, darr2, SIZE);
        #endif
        
        #if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_style(arr1, arr2, arr3, SIZE);
        #endif
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Output results to prevent optimization */
    printf("Global accumulator: %lld\n", global_acc);
    printf("Time used: %f seconds\n", cpu_time_used);
    printf("Array samples: arr1[10]=%d, arr2[20]=%d, darr1[30]=%f\n", 
           arr1[10], arr2[20], darr1[30]);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(darr1);
    free(darr2);
    
    return 0;
}

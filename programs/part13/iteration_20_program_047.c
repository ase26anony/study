/* test_modulo_sched.c
 * 
 * This program is designed to trigger GCC's modulo scheduling optimization,
 * specifically targeting the register move scheduling logic in modulo-sched.cc.
 * The goal is to cover lines 596-606 in schedule_reg_move() function.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Global volatile variable to prevent dead code elimination */
volatile long long global_sum = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        x = x * 3 + a[i];          /* Chain 1: distance 1 dependency */
        y = y * 5 + b[i] - x;      /* Chain 2: depends on x from current iteration */
        z = z * 7 + c[i] + y;      /* Chain 3: depends on y from current iteration */
        w = w * 11 + x * y;        /* Chain 4: uses both x and y */
        v = v * 13 + z - w;        /* Chain 5: uses z and w */
        u = u * 17 + v * 2;        /* Chain 6: uses v */
        
        /* Cross-chain dependencies to create anti-dependencies */
        a[i] = x + y;
        b[i] = z - w;
        c[i] = v * u;
    }
    
    global_sum += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *arr, double *out, int n) {
    double sum1 = 0.1, sum2 = 0.2, sum3 = 0.3;
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + arr[i];           /* Distance 1 dependency */
        sum2 = sum2 * 1.02 + arr[i] * 2.0;     /* Another chain */
        sum3 = sum3 * 1.03 + arr[i] * 3.0;     /* Third chain */
        
        /* Product chains with different distances */
        if (i >= 1) prod1 = prod1 * (arr[i] + arr[i-1]);
        if (i >= 2) prod2 = prod2 * (arr[i] + arr[i-2]);
        
        /* Output depends on all chains */
        out[i] = sum1 + sum2 + sum3 + prod1 + prod2;
    }
    
    global_sum += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer chasing chains */
        tmp1 = acc1 + *ptr1;
        tmp2 = acc2 + *ptr2;
        tmp3 = acc3 + *ptr3;
        
        /* Cross dependencies between chains */
        acc1 = tmp1 * 2 - tmp2;
        acc2 = tmp2 * 3 - tmp3;
        acc3 = tmp3 * 4 - tmp1;
        
        /* Update pointers with stride */
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Anti-dependency: store results back */
        *(ptr1 - stride) = acc1;
        *(ptr2 - stride) = acc2;
        *(ptr3 - stride) = acc3;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops_varying_distance(short *s, int *i, long *l, int n) {
    long long ll_acc = 1;
    int i_acc = 0;
    short s_acc = 0;
    
    for (int j = 0; j < n; j++) {
        /* Different dependency distances */
        ll_acc = ll_acc * 3 + l[j];            /* Distance 1 */
        if (j >= 1) i_acc = i_acc * 2 + i[j-1]; /* Distance 1 */
        if (j >= 2) s_acc = s_acc + s[j-2];     /* Distance 2 */
        
        /* Cross-type operations to increase register pressure */
        l[j] = ll_acc + i_acc;
        i[j] = i_acc * s_acc;
        s[j] = s_acc + (j & 0xFF);
    }
    
    global_sum += ll_acc + i_acc + s_acc;
}

/* Test 5: Nested loops with inner loop being modulo scheduled */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int i = 1; i < rows; i++) {
        int *curr = mat + i * cols;
        int *prev = mat + (i-1) * cols;
        
        /* Inner loop with carried dependency */
        for (int j = 0; j < cols; j++) {
            /* Multiple operations to increase register pressure */
            int a = prev[j] * 3;
            int b = curr[j] * 5;
            int c = (j > 0) ? curr[j-1] * 7 : 0;
            int d = a + b + c;
            
            /* Recurrence with distance 1 */
            curr[j] = d + (j > 0 ? curr[j-1] : 0);
            
            /* Additional independent chain */
            prev[j] = prev[j] * 2 - b;
        }
    }
    
    /* Sum a few elements to prevent elimination */
    for (int i = 0; i < rows * cols; i += rows + 1) {
        global_sum += mat[i];
    }
}

/* Test 6: PowerPC specific - double precision operations */
#ifdef __powerpc__ || __PPC__
void test_powerpc_double_ops(double *a, double *b, double *c, int n) {
    double sum0 = 0.0, sum1 = 0.0, sum2 = 0.0;
    double prod0 = 1.0, prod1 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double precision dependency chains */
        sum0 = sum0 * 1.0001 + a[i];
        sum1 = sum1 * 1.0002 + b[i];
        sum2 = sum2 * 1.0003 + c[i];
        
        /* Cross dependencies */
        prod0 = prod0 * (sum0 + 0.001);
        prod1 = prod1 * (sum1 - sum2);
        
        /* Store results creating anti-dependencies */
        a[i] = sum0 + prod0;
        b[i] = sum1 + prod1;
        c[i] = sum2 * 2.0;
    }
    
    global_sum += (long long)(sum0 + sum1 + sum2 + prod0 + prod1);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_manual_unroll(int *data, int n) {
    int acc0 = 1, acc1 = 2, acc2 = 3, acc3 = 4;
    int tmp0, tmp1, tmp2, tmp3;
    
    for (int i = 0; i < n; i += 4) {
        /* Unrolled operations with dependencies */
        tmp0 = acc0 * 3 + data[i];
        tmp1 = acc1 * 5 + data[i+1] + tmp0;
        tmp2 = acc2 * 7 + data[i+2] + tmp1;
        tmp3 = acc3 * 11 + data[i+3] + tmp2;
        
        /* Cross-iteration dependencies */
        acc0 = tmp0 - tmp3;
        acc1 = tmp1 - tmp0;
        acc2 = tmp2 - tmp1;
        acc3 = tmp3 - tmp2;
        
        /* Store back with anti-dependencies */
        data[i] = acc0;
        data[i+1] = acc1;
        data[i+2] = acc2;
        data[i+3] = acc3;
    }
    
    global_sum += acc0 + acc1 + acc2 + acc3;
}

/* Main test driver */
int main() {
    const int N = 1024;
    const int ITERS = 10000;
    
    /* Allocate and initialize test arrays */
    int *arr1 = malloc(N * sizeof(int));
    int *arr2 = malloc(N * sizeof(int));
    int *arr3 = malloc(N * sizeof(int));
    double *darr1 = malloc(N * sizeof(double));
    double *darr2 = malloc(N * sizeof(double));
    short *sarr = malloc(N * sizeof(short));
    long *larr = malloc(N * sizeof(long));
    int *matrix = malloc(64 * 64 * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        darr1[i] = (double)(rand() % 100) / 10.0;
        darr2[i] = 0.0;
        sarr[i] = rand() % 100;
        larr[i] = rand() % 100;
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = rand() % 100;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERS; iter++) {
        test_int_recurrence_multi_chain(arr1, arr2, arr3, N);
        test_float_accumulate(darr1, darr2, N);
        test_pointer_chasing(arr1, 4, N/4);
        test_mixed_ops_varying_distance(sarr, arr2, larr, N);
        test_nested_loops(matrix, 64, 64);
        test_manual_unroll(arr3, N);
        
        #ifdef __powerpc__ || __PPC__
        test_powerpc_double_ops(darr1, darr2, darr1, N);
        #endif
    }
    
    printf("Final sum: %lld\n", global_sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(darr1);
    free(darr2);
    free(sarr);
    free(larr);
    free(matrix);
    
    return 0;
}

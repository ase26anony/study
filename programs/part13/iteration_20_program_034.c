/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves test_modulo_sched.c -o test_modulo_sched
 * For PowerPC: add -mtune=powerpc -mcpu=power8
 * For ARM SVE: add -march=armv8-a+sve -ftree-vectorize
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

volatile int global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_int_recurrence_chains(int *a, int *b, int *c, int *d, int n) {
    int x = 1, y = 2, z = 3;
    int acc1 = a[0], acc2 = b[0], acc3 = c[0];
    
    for (int i = 1; i < n; i++) {
        /* Multiple independent recurrence chains */
        acc1 = acc1 * 3 + a[i] * 7;      /* Chain 1: distance 1 */
        acc2 = acc2 + b[i] * acc1;       /* Chain 2: depends on chain1 */
        acc3 = acc3 * 5 - c[i] + acc2;   /* Chain 3: depends on chain2 */
        
        /* Additional operations to increase register pressure */
        x = (x << 1) ^ (acc1 & 0xFF);
        y = (y * 13) | (acc2 & 0xFF);
        z = (z + 29) & (acc3 | 0xFF);
        
        /* Store results with strided access pattern */
        d[i] = acc1 + acc2 + acc3 + x + y + z;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(float *fa, float *fb, float *fc, float *fd, int n) {
    float sum1 = fa[0], sum2 = fb[0], sum3 = fc[0];
    float prod1 = 1.0f, prod2 = 1.0f;
    
    for (int i = 1; i < n; i++) {
        /* Floating-point recurrence with multiple chains */
        sum1 = sum1 * 1.5f + fa[i] * 2.0f;
        sum2 = sum2 + fb[i] * sum1;
        sum3 = sum3 - fc[i] / (sum2 + 1.0f);
        
        /* Parallel product chains */
        prod1 = prod1 * (fa[i] + 0.5f);
        prod2 = prod2 * (fb[i] - 0.25f);
        
        /* Mixed operations to prevent optimization */
        fd[i] = sum1 * sum2 + sum3 * prod1 - prod2;
    }
    
    global_sum += (int)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access and high register usage */
void test_pointer_chasing(int *arr, int n, int stride) {
    int *ptr1 = arr;
    int *ptr2 = arr + stride;
    int *ptr3 = arr + 2 * stride;
    int *end = arr + n;
    
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int acc1 = *ptr1, acc2 = *ptr2, acc3 = *ptr3;
    
    while (ptr3 < end) {
        /* Pointer chasing with carried dependencies */
        acc1 = (acc1 << 3) + *ptr1 + sum1;
        acc2 = (acc2 * 11) ^ *ptr2 + sum2;
        acc3 = (acc3 & 0xFFFF) | (*ptr3 << 16) + sum3;
        
        /* Update sums with cross-dependencies */
        sum1 = sum1 + (acc2 & 0xFF);
        sum2 = sum2 ^ (acc3 & 0xFF);
        sum3 = sum3 | (acc1 & 0xFF);
        
        /* Strided pointer advancement */
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
    }
    
    global_sum += acc1 + acc2 + acc3 + sum1 + sum2 + sum3;
}

/* Test 4: Mixed integer operations with variable dependency distances */
void test_mixed_operations(short *sa, int *ia, long long *lla, int n) {
    short s_acc = sa[0];
    int i_acc = ia[0];
    long long ll_acc = lla[0];
    
    for (int i = 1; i < n; i++) {
        /* Different types of operations on different dependency chains */
        s_acc = (s_acc * 3 + sa[i]) & 0x7FFF;
        i_acc = i_acc + (ia[i] << 2) - (s_acc * 7);
        ll_acc = ll_acc * 5 + lla[i] + (i_acc * 11LL);
        
        /* Additional arithmetic to increase register pressure */
        int temp1 = (s_acc << 5) | (i_acc & 0xFF);
        long long temp2 = (ll_acc >> 3) + (temp1 * 13LL);
        
        /* Cross-chain dependencies */
        s_acc ^= (temp2 & 0xFFFF);
        i_acc += (temp2 >> 16) & 0xFFFF;
        ll_acc -= temp1 * temp2;
    }
    
    global_sum += s_acc + i_acc + (int)(ll_acc & 0xFFFFFFFF);
}

/* Test 5: Nested loops with innermost loop having carried dependencies */
void test_nested_loops(int *mat, int rows, int cols) {
    for (int r = 1; r < rows; r++) {
        int prev_row = (r - 1) * cols;
        int curr_row = r * cols;
        int acc = mat[prev_row];
        
        /* Innermost loop with carried dependency */
        for (int c = 1; c < cols; c++) {
            /* 2D recurrence: depends on left neighbor and top neighbor */
            int left = mat[curr_row + c - 1];
            int top = mat[prev_row + c];
            int diag = mat[prev_row + c - 1];
            
            /* Complex recurrence relation */
            int val = (left * 3 + top * 5 - diag * 2) / 4;
            val = (val + acc) & 0xFFF;
            acc = (acc * 7 + val) & 0xFFF;
            
            mat[curr_row + c] = val + acc;
        }
    }
    
    /* Use some values to prevent dead code elimination */
    for (int i = 0; i < rows * cols; i += cols + 1) {
        global_sum += mat[i];
    }
}

/* Test 6: PowerPC-specific operations (if compiled for PowerPC) */
#ifdef __powerpc__
void test_powerpc_specific(double *da, double *db, double *dc, int n) {
    double sum = da[0];
    double prod = db[0];
    
    for (int i = 1; i < n; i++) {
        /* Double precision operations that use FP registers */
        sum = sum * 1.25 + da[i] * 2.5;
        prod = prod * (db[i] + 0.125);
        
        /* Cross dependency */
        double temp = sum * prod;
        sum = sum + dc[i] * temp;
        prod = prod - temp / (dc[i] + 1.0);
    }
    
    global_sum += (int)(sum + prod);
}
#endif

/* Test 7: Manual unrolling to increase operations per iteration */
#pragma GCC unroll 4
void test_unrolled_loop(int *a, int *b, int *c, int n) {
    int acc1 = a[0], acc2 = b[0], acc3 = c[0];
    int x = 1, y = 2;
    
    for (int i = 1; i < n; i++) {
        /* Manually unrolled-like operations */
        acc1 = acc1 * 3 + a[i];
        acc2 = acc2 + b[i] * acc1;
        acc3 = acc3 ^ c[i] + acc2;
        
        /* Additional parallel chains */
        x = (x << 2) + (acc1 & 0xFF);
        y = (y * 17) - (acc2 & 0xFF);
        
        /* More operations to fill issue slots */
        int t1 = acc1 * x;
        int t2 = acc2 * y;
        int t3 = acc3 + t1;
        int t4 = x * y + t2;
        
        a[i] = t3;
        b[i] = t4;
        c[i] = t3 + t4;
    }
    
    global_sum += acc1 + acc2 + acc3 + x + y;
}

int main() {
    /* Initialize data arrays */
    int *data1 = malloc(SIZE * sizeof(int));
    int *data2 = malloc(SIZE * sizeof(int));
    int *data3 = malloc(SIZE * sizeof(int));
    int *data4 = malloc(SIZE * sizeof(int));
    float *fdata1 = malloc(SIZE * sizeof(float));
    float *fdata2 = malloc(SIZE * sizeof(float));
    float *fdata3 = malloc(SIZE * sizeof(float));
    float *fdata4 = malloc(SIZE * sizeof(float));
    short *sdata = malloc(SIZE * sizeof(short));
    long long *lldata = malloc(SIZE * sizeof(long long));
    int *matrix = malloc(100 * 100 * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        data4[i] = rand() % 100;
        fdata1[i] = (float)(rand() % 100) / 10.0f;
        fdata2[i] = (float)(rand() % 100) / 10.0f;
        fdata3[i] = (float)(rand() % 100) / 10.0f;
        fdata4[i] = (float)(rand() % 100) / 10.0f;
        sdata[i] = rand() % 1000;
        lldata[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 100 * 100; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Run test functions multiple times to ensure hot loop compilation */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        test_int_recurrence_chains(data1, data2, data3, data4, SIZE);
        test_float_accumulation(fdata1, fdata2, fdata3, fdata4, SIZE);
        test_pointer_chasing(data1, SIZE, 4);
        test_mixed_operations(sdata, data1, lldata, SIZE);
        test_nested_loops(matrix, 100, 100);
        test_unrolled_loop(data1, data2, data3, SIZE);
        
        #ifdef __powerpc__
        double *darray1 = malloc(SIZE * sizeof(double));
        double *darray2 = malloc(SIZE * sizeof(double));
        double *darray3 = malloc(SIZE * sizeof(double));
        for (int i = 0; i < SIZE; i++) {
            darray1[i] = (double)(rand() % 100) / 10.0;
            darray2[i] = (double)(rand() % 100) / 10.0;
            darray3[i] = (double)(rand() % 100) / 10.0;
        }
        test_powerpc_specific(darray1, darray2, darray3, SIZE);
        free(darray1);
        free(darray2);
        free(darray3);
        #endif
    }
    
    /* Output result to prevent optimization */
    printf("Final result: %d\n", global_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(fdata4);
    free(sdata);
    free(lldata);
    free(matrix);
    
    return 0;
}

/* test_modulo_sched.c - Test program for GCC modulo scheduling register moves */
/* Compile with: gcc -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves -mtune=powerpc -mcpu=power8 test_modulo_sched.c -o test_modulo_sched */
/* For ARM SVE: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=armv8-a+sve -ftree-vectorize test_modulo_sched.c -o test_modulo_sched */
/* For RISC-V: gcc -O3 -fdump-rtl-sms -fmodulo-sched -march=rv64gcv test_modulo_sched.c -o test_modulo_sched */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 10000

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multi_chain(int *a, int *b, int *c, int *d, int n) {
    int i;
    /* Multiple independent recurrence chains to increase register pressure */
    int acc1 = a[0];
    int acc2 = b[0] + b[1];
    int acc3 = c[0] * 2;
    int acc4 = d[0] ^ 0x55;
    
    for (i = 1; i < n; i++) {
        /* Chain 1: Simple accumulation with dependency distance 1 */
        acc1 = acc1 + a[i] * 3;
        a[i] = acc1;
        
        /* Chain 2: More complex recurrence with shift operations */
        acc2 = (acc2 << 1) ^ b[i];
        b[i] = acc2 & 0xFF;
        
        /* Chain 3: Multiplicative recurrence */
        acc3 = acc3 * 7 + c[i];
        c[i] = acc3 % 1000;
        
        /* Chain 4: Bitwise operations recurrence */
        acc4 = (acc4 | d[i]) ^ (acc4 << 3);
        d[i] = acc4;
        
        /* Additional operations to increase register pressure */
        a[i] += (acc2 & 0xF) - (acc3 >> 2);
        b[i] ^= acc1 & acc4;
    }
    
    global_acc += acc1 + acc2 + acc3 + acc4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulation(double *x, double *y, double *z, int n) {
    int i;
    double sum1 = x[0];
    double sum2 = y[0];
    double prod1 = z[0];
    double prod2 = 1.0;
    
    for (i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + x[i] * 1.5;
        sum2 = sum2 - y[i] / 2.0;
        prod1 = prod1 * (z[i] + 0.1);
        prod2 = prod2 * (x[i] * y[i] * 0.01);
        
        /* Cross-chain dependencies to create anti-dependencies */
        x[i] = sum1 * prod1;
        y[i] = sum2 + prod2;
        z[i] = (sum1 - sum2) * (prod1 / prod2);
        
        /* Additional operations to increase register pressure */
        double temp1 = sum1 * sum2;
        double temp2 = prod1 - prod2;
        x[i] += temp1 * temp2;
        y[i] -= temp1 / (temp2 + 1.0);
    }
    
    global_acc += (long long)(sum1 + sum2 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access patterns */
void test_pointer_chasing(int *data, int n, int stride) {
    int i;
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *ptr4 = data + 3 * stride;
    
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Manual unrolling to increase operations per iteration */
    for (i = 0; i < n - 4 * stride; i++) {
        /* Multiple pointer-chasing chains with dependencies */
        sum1 = sum1 * 3 + *ptr1;
        sum2 = sum2 ^ (*ptr2 + sum1);
        sum3 = sum3 + (*ptr3 * sum2);
        sum4 = sum4 | (*ptr4 ^ sum3);
        
        /* Store results back creating anti-dependencies */
        *ptr1 = sum1;
        *ptr2 = sum2;
        *ptr3 = sum3;
        *ptr4 = sum4;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        ptr4 += 4;
        
        /* Additional computations to increase register pressure */
        int temp = sum1 + sum2 - sum3 * sum4;
        *ptr1 += temp;
        *ptr2 -= temp;
    }
    
    global_acc += sum1 + sum2 + sum3 + sum4;
}

/* Test 4: Mixed integer operations with varying dependency distances */
void test_mixed_ops_varying_distance(short *s, int *i, long *l, int n) {
    int j;
    long acc_long = l[0];
    int acc_int = i[0];
    short acc_short = s[0];
    
    /* Loop with operations on different data types */
    for (j = 1; j < n; j++) {
        /* Chain with distance 1 */
        acc_short = acc_short + s[j] * 2;
        s[j] = acc_short;
        
        /* Chain with implicit distance 2 through indexing */
        if (j >= 2) {
            acc_int = acc_int * 5 + i[j-2];
            i[j] = acc_int;
        }
        
        /* Chain with more complex recurrence */
        acc_long = (acc_long << 3) | (l[j] & 0xFF);
        l[j] = acc_long;
        
        /* Cross-type operations to increase register pressure */
        i[j] += (int)acc_short;
        l[j] += (long)acc_int;
        s[j] ^= (short)(acc_long & 0xFFFF);
        
        /* Additional arithmetic to prevent optimization */
        int temp = j * 7 + acc_int;
        acc_long += temp;
        acc_short -= temp & 0xFF;
    }
    
    global_acc += acc_long + acc_int + acc_short;
}

/* Test 5: Nested loops where inner loop is modulo scheduled */
void test_nested_loop_modulo(int *mat, int rows, int cols) {
    int i, j;
    int diag_acc = 0;
    int row_acc = 0;
    int col_acc = 0;
    
    /* Outer loop */
    for (i = 1; i < rows - 1; i++) {
        /* Inner loop - this should be modulo scheduled */
        for (j = 1; j < cols - 1; j++) {
            /* Multiple recurrence relations within inner loop */
            int idx = i * cols + j;
            
            /* Horizontal recurrence */
            int left = mat[idx - 1];
            mat[idx] = left + mat[idx] * 2;
            
            /* Vertical recurrence */
            int up = mat[idx - cols];
            mat[idx] += up * 3;
            
            /* Diagonal recurrence */
            int diag = mat[idx - cols - 1];
            diag_acc = diag_acc * 7 + diag;
            mat[idx] ^= diag_acc;
            
            /* Additional operations to increase register pressure */
            row_acc += mat[idx] & 0xF;
            col_acc ^= mat[idx] >> 4;
            mat[idx] += row_acc - col_acc;
        }
        
        /* Prevent outer loop optimization */
        global_acc += diag_acc + row_acc + col_acc;
        diag_acc = 0;
        row_acc = 0;
        col_acc = 0;
    }
}

/* Test 6: PowerPC specific - double precision with FMA-like patterns */
#ifdef __powerpc__
void test_powerpc_double_ops(double *a, double *b, double *c, int n) {
    int i;
    double acc1 = a[0];
    double acc2 = b[0];
    double acc3 = c[0];
    
    /* Loop designed to use multiple FP registers */
    for (i = 1; i < n; i++) {
        /* FMA-like patterns common in PowerPC */
        acc1 = acc1 * 1.1 + a[i] * 2.2;
        acc2 = acc2 - b[i] * 3.3;
        acc3 = acc3 * 0.5 + c[i] * 4.4;
        
        /* Cross dependencies */
        a[i] = acc1 * acc2;
        b[i] = acc2 + acc3;
        c[i] = acc1 - acc3;
        
        /* More operations to increase register pressure */
        double t1 = a[i] * b[i];
        double t2 = c[i] / a[i];
        double t3 = t1 + t2;
        
        acc1 += t3;
        acc2 -= t3 * 0.1;
        acc3 *= t3;
    }
    
    global_acc += (long long)(acc1 + acc2 + acc3);
}
#endif

/* Test 7: Vector-style operations for ARM SVE/RISC-V V */
#if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
void test_vector_style_ops(int *src, int *dst, int n, int inc) {
    int i;
    int sum = src[0];
    int prod = 1;
    int xor_acc = 0;
    
    /* Strided access pattern common in vector loops */
    for (i = 0; i < n; i += inc) {
        /* Multiple parallel chains */
        sum = sum + src[i] * 3;
        prod = prod * (src[i] + 1);
        xor_acc = xor_acc ^ src[i];
        
        /* Store with anti-dependency */
        dst[i] = sum + prod - xor_acc;
        
        /* Additional operations */
        int idx = i + inc;
        if (idx < n) {
            src[idx] = dst[i] & 0xFF;
            sum += src[idx] * 2;
        }
    }
    
    global_acc += sum + prod + xor_acc;
}
#endif

/* Main driver */
int main() {
    int i;
    clock_t start, end;
    double cpu_time_used;
    
    /* Initialize arrays */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    int *arr3 = malloc(SIZE * sizeof(int));
    int *arr4 = malloc(SIZE * sizeof(int));
    double *darr1 = malloc(SIZE * sizeof(double));
    double *darr2 = malloc(SIZE * sizeof(double));
    double *darr3 = malloc(SIZE * sizeof(double));
    short *sarr = malloc(SIZE * sizeof(short));
    long *larr = malloc(SIZE * sizeof(long));
    int *matrix = malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
        arr4[i] = i * 4;
        darr1[i] = i * 0.1;
        darr2[i] = i * 0.2;
        darr3[i] = i * 0.3;
        sarr[i] = i & 0x7FFF;
        larr[i] = i * 1000L;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 256;
    }
    
    start = clock();
    
    /* Run tests multiple times to ensure hot loop compilation */
    for (i = 0; i < ITERATIONS; i++) {
        test_int_recurrence_multi_chain(arr1, arr2, arr3, arr4, SIZE);
        test_float_accumulation(darr1, darr2, darr3, SIZE);
        test_pointer_chasing(arr1, SIZE, 4);
        test_mixed_ops_varying_distance(sarr, arr2, larr, SIZE);
        test_nested_loop_modulo(matrix, 32, 32);
        
        #ifdef __powerpc__
        test_powerpc_double_ops(darr1, darr2, darr3, SIZE);
        #endif
        
        #if defined(__ARM_FEATURE_SVE) || defined(__riscv_v)
        test_vector_style_ops(arr1, arr3, SIZE, 2);
        #endif
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Modulo scheduling test completed.\n");
    printf("Global accumulator: %lld\n", global_acc);
    printf("Time taken: %f seconds\n", cpu_time_used);
    printf("Check dump files for '--(T,' pattern to verify coverage\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(darr1);
    free(darr2);
    free(darr3);
    free(sarr);
    free(larr);
    free(matrix);
    
    return 0;
}

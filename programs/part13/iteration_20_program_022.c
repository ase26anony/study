/* test_modulo_sched.c
 * Comprehensive test for GCC modulo scheduling register move coverage
 * Targets lines 596-606 in modulo-sched.cc
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* Global accumulator to prevent dead code elimination */
volatile long long global_acc = 0;

/* Test 1: Integer recurrence with multiple dependency chains */
void test_int_recurrence_multichain(int *a, int *b, int *c, int n) {
    int x = 1, y = 2, z = 3;
    int w = 4, v = 5, u = 6;
    
    for (int i = 0; i < n; i++) {
        /* Multiple independent recurrence chains to increase register pressure */
        x = x * 3 + a[i];          /* Chain 1: distance 1 dependency */
        y = y + x * 2 - b[i];      /* Chain 2: depends on x from same iteration */
        z = z * 5 + y + c[i];      /* Chain 3: depends on y */
        w = w * 7 - z + a[i];      /* Chain 4: depends on z */
        v = v * 11 + w * b[i];     /* Chain 5: depends on w */
        u = u * 13 + v - c[i];     /* Chain 6: depends on v */
        
        /* Cross-chain operations to create anti-dependencies */
        a[i] = x + y;
        b[i] = z - w;
        c[i] = v * u;
    }
    
    global_acc += x + y + z + w + v + u;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_accumulate(double *arr, double *out, int n) {
    double sum1 = 0.0, sum2 = 0.0, sum3 = 0.0;
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 + arr[i] * 1.5;          /* Distance 1 true dependency */
        sum2 = sum2 + sum1 * 0.7;            /* Depends on sum1 from same iteration */
        sum3 = sum3 + sum2 * arr[i];         /* Depends on sum2 */
        prod1 = prod1 * (1.0 + arr[i] * 0.1); /* Independent chain */
        prod2 = prod2 * (1.0 - arr[i] * 0.05); /* Another independent chain */
        
        /* Complex output calculation with register pressure */
        out[i] = sum1 * sum2 + sum3 * prod1 - prod2;
        
        /* Additional operations to increase register usage */
        arr[i] = (arr[i] + sum1) * (sum2 - prod1) / (prod2 + 1.0);
    }
    
    global_acc += (long long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int stride, int n) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple pointer chains with carried dependencies */
        acc1 = acc1 * 3 + *ptr1;      /* Chain 1 */
        acc2 = acc2 * 5 + *ptr2;      /* Chain 2 */
        acc3 = acc3 * 7 + *ptr3;      /* Chain 3 */
        
        /* Cross dependencies between chains */
        tmp1 = acc1 + acc2;
        tmp2 = acc2 * acc3;
        tmp3 = acc3 - acc1;
        
        /* Update pointers with stride */
        ptr1 += stride;
        ptr2 += stride;
        ptr3 += stride;
        
        /* Store results back to create register pressure */
        *(ptr1 - stride) = tmp1;
        *(ptr2 - stride) = tmp2;
        *(ptr3 - stride) = tmp3;
    }
    
    global_acc += acc1 + acc2 + acc3 + tmp1 + tmp2 + tmp3;
}

/* Test 4: Mixed integer operations with manual unrolling hint */
void test_mixed_ops_unrolled(int *a, int *b, int *c, int n) {
    int x0 = 1, x1 = 2, x2 = 3, x3 = 4;
    int y0 = 5, y1 = 6, y2 = 7, y3 = 8;
    
    /* Manual unrolling to increase operations per iteration */
    for (int i = 0; i < n; i += 4) {
        /* First set of operations */
        x0 = x0 * 17 + a[i] - b[i];
        y0 = y0 * 19 + x0 * c[i];
        
        /* Second set with dependencies on first */
        x1 = x1 * 23 + x0 + a[i+1];
        y1 = y1 * 29 + y0 - b[i+1];
        
        /* Third set with longer dependency chain */
        x2 = x2 * 31 + x1 * y1;
        y2 = y2 * 37 + x2 + c[i+2];
        
        /* Fourth set with complex dependencies */
        x3 = x3 * 41 + x2 * y2;
        y3 = y3 * 43 + x3 - a[i+3];
        
        /* Store results creating anti-dependencies */
        a[i] = x0 + y0;
        a[i+1] = x1 * y1;
        a[i+2] = x2 - y2;
        a[i+3] = x3 + y3;
        
        /* Additional operations on b and c arrays */
        b[i] = (x0 << 3) & 0xFF;
        b[i+1] = (x1 << 2) | 0x0F;
        b[i+2] = (x2 >> 1) ^ 0xAA;
        b[i+3] = (x3 << 1) & 0x55;
        
        c[i] = y0 * 2;
        c[i+1] = y1 / 2;
        c[i+2] = y2 + 100;
        c[i+3] = y3 - 50;
    }
    
    global_acc += x0 + x1 + x2 + x3 + y0 + y1 + y2 + y3;
}

/* Test 5: PowerPC specific - double precision with FMA-like patterns */
#ifdef __powerpc__ || __PPC__
void test_powerpc_double(double *a, double *b, double *c, int n) {
    double acc0 = 0.0, acc1 = 0.0, acc2 = 0.0, acc3 = 0.0;
    double tmp0 = 1.0, tmp1 = 2.0, tmp2 = 3.0, tmp3 = 4.0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple double precision chains */
        acc0 = acc0 * 1.1 + a[i] * b[i];
        acc1 = acc1 * 1.2 + acc0 * c[i];
        acc2 = acc2 * 1.3 + acc1 * a[i];
        acc3 = acc3 * 1.4 + acc2 * b[i];
        
        /* Cross dependencies */
        tmp0 = tmp0 * 1.5 + acc0;
        tmp1 = tmp1 * 1.6 + acc1;
        tmp2 = tmp2 * 1.7 + acc2;
        tmp3 = tmp3 * 1.8 + acc3;
        
        /* Store results */
        a[i] = acc0 + tmp0;
        b[i] = acc1 * tmp1;
        c[i] = acc2 - tmp2;
        
        /* Additional operation to use result */
        global_acc += (long long)(acc3 * tmp3);
    }
}
#endif

/* Test 6: ARM SVE style with unknown bounds */
#ifdef __ARM_FEATURE_SVE
void test_arm_sve_style(int *a, int *b, int n) {
    int sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
    int prod0 = 1, prod1 = 1, prod2 = 1, prod3 = 1;
    
    /* Loop with runtime bounds to trigger SVE vectorization */
    for (int i = 0; i < n; i++) {
        /* Multiple independent chains */
        sum0 = sum0 + a[i] * 2;
        sum1 = sum1 + b[i] * 3;
        sum2 = sum2 + a[i] * b[i];
        sum3 = sum3 + (a[i] << 2);
        
        /* Product chains with carried dependencies */
        prod0 = prod0 * (sum0 + 1);
        prod1 = prod1 * (sum1 - 1);
        prod2 = prod2 * (sum2 * 2);
        prod3 = prod3 * (sum3 >> 1);
        
        /* Store with anti-dependencies */
        a[i] = sum0 + prod0;
        b[i] = sum1 * prod1;
        
        /* Additional computation */
        global_acc += sum2 + prod2 + sum3 + prod3;
    }
}
#endif

/* Test 7: RISC-V V extension style with strided access */
void test_riscv_vector_style(int *data, int stride, int n) {
    int acc[4] = {1, 2, 3, 4};
    int idx = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple accumulators with strided access */
        acc[0] = acc[0] * 11 + data[idx];
        acc[1] = acc[1] * 13 + data[idx + stride];
        acc[2] = acc[2] * 17 + data[idx + 2 * stride];
        acc[3] = acc[3] * 19 + data[idx + 3 * stride];
        
        /* Cross-accumulator operations */
        int tmp0 = acc[0] + acc[1];
        int tmp1 = acc[1] * acc[2];
        int tmp2 = acc[2] - acc[3];
        int tmp3 = acc[3] & acc[0];
        
        /* Store results back with different stride */
        data[idx] = tmp0;
        data[idx + stride] = tmp1;
        data[idx + 2 * stride] = tmp2;
        data[idx + 3 * stride] = tmp3;
        
        /* Update index with wrap-around */
        idx = (idx + 1) % stride;
        
        global_acc += tmp0 + tmp1 + tmp2 + tmp3;
    }
}

/* Main driver with hot loops */
int main() {
    const int N = 1024;
    const int ITERS = 10000;
    const int STRIDE = 8;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    double *darr1 = (double*)malloc(N * sizeof(double));
    double *darr2 = (double*)malloc(N * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        darr1[i] = (double)(rand() % 100) / 10.0;
        darr2[i] = 0.0;
    }
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Hot loop running multiple test patterns */
    for (int iter = 0; iter < ITERS; iter++) {
        test_int_recurrence_multichain(arr1, arr2, arr3, N);
        test_float_accumulate(darr1, darr2, N);
        test_pointer_chasing(arr1, STRIDE, N/STRIDE);
        test_mixed_ops_unrolled(arr1, arr2, arr3, N);
        test_riscv_vector_style(arr1, STRIDE, N/STRIDE);
        
        #ifdef __powerpc__ || __PPC__
        test_powerpc_double(darr1, darr2, darr1, N);
        #endif
        
        #ifdef __ARM_FEATURE_SVE
        test_arm_sve_style(arr1, arr2, N);
        #endif
    }
    
    printf("Final accumulator: %lld\n", global_acc);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(darr1);
    free(darr2);
    
    return 0;
}

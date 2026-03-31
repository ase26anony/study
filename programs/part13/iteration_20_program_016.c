/* Test program for GCC modulo scheduling register move coverage */
/* Compile with: -O3 -fdump-rtl-sms -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* For PowerPC: add -mtune=powerpc -mcpu=power8 */
/* For ARM SVE: add -march=armv8-a+sve -ftree-vectorize */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 1024
#define ITERS 10000

/* Volatile to prevent optimization */
volatile long global_sum = 0;

/* Test 1: Multiple integer recurrence chains with high register pressure */
void test_multi_recurrence_int(int *a, int *b, int *c, int *d, int n) {
    int x1 = a[0], x2 = b[0], x3 = c[0], x4 = d[0];
    int y1 = a[1], y2 = b[1], y3 = c[1], y4 = d[1];
    
    /* Multiple independent dependency chains with different distances */
    for (int i = 2; i < n; i++) {
        /* Chain 1: distance 1 recurrence */
        x1 = x1 * 3 + a[i];
        a[i] = x1;
        
        /* Chain 2: distance 1 recurrence with different operation */
        x2 = (x2 << 1) ^ b[i];
        b[i] = x2;
        
        /* Chain 3: distance 2 recurrence */
        x3 = x3 + y3 * 5;
        y3 = c[i];
        c[i] = x3;
        
        /* Chain 4: complex recurrence with multiple operations */
        x4 = (x4 & 0xFF) * 7 + (x4 >> 8) + d[i];
        d[i] = x4;
        
        /* Additional operations to increase register pressure */
        a[i-1] += b[i] & c[i];
        b[i-1] ^= d[i] | a[i];
    }
    
    /* Prevent dead code elimination */
    global_sum += x1 + x2 + x3 + x4;
}

/* Test 2: Floating-point accumulation with mixed operations */
void test_float_recurrence(double *a, double *b, double *c, int n) {
    double sum1 = a[0], sum2 = b[0], sum3 = c[0];
    double prod1 = 1.0, prod2 = 1.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP dependency chains */
        sum1 = sum1 * 1.01 + a[i];
        sum2 = sum2 * 0.99 + b[i];
        sum3 = sum3 + a[i] * b[i] - c[i];
        
        /* Independent chains with different latencies */
        prod1 = prod1 * (1.0 + a[i] * 0.001);
        prod2 = prod2 / (1.0 + b[i] * 0.001);
        
        /* Cross-chain dependencies */
        a[i] = sum1 + prod1;
        b[i] = sum2 - prod2;
        c[i] = sum3 * (prod1 + prod2);
    }
    
    global_sum += (long)(sum1 + sum2 + sum3 + prod1 + prod2);
}

/* Test 3: Pointer-chasing with strided access */
void test_pointer_chasing(int *data, int n, int stride) {
    int *ptr1 = data;
    int *ptr2 = data + stride;
    int *ptr3 = data + 2 * stride;
    int *end = data + n;
    
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int tmp1 = 0, tmp2 = 0, tmp3 = 0;
    
    while (ptr3 < end) {
        /* Multiple pointer chains with carried dependencies */
        acc1 = acc1 * 2 + *ptr1;
        tmp1 = acc1 & 0xFF;
        *ptr1 = tmp1;
        
        acc2 = (acc2 >> 1) ^ *ptr2;
        tmp2 = acc2 | 0x80;
        *ptr2 = tmp2;
        
        acc3 = acc3 + *ptr3 - tmp1;
        tmp3 = acc3 * 3;
        *ptr3 = tmp3;
        
        /* Update pointers with different strides */
        ptr1 += 1;
        ptr2 += 2;
        ptr3 += 3;
        
        /* Additional operations for register pressure */
        acc1 ^= tmp2;
        acc2 += tmp3;
        acc3 &= tmp1;
    }
    
    global_sum += acc1 + acc2 + acc3;
}

/* Test 4: Nested loops with innermost modulo-scheduled loop */
void test_nested_loops(int *a, int *b, int *c, int n, int m) {
    for (int j = 0; j < m; j++) {
        int prev1 = a[j];
        int prev2 = b[j];
        int prev3 = c[j];
        
        /* Innermost loop with carried dependencies */
        for (int i = 1; i < n; i++) {
            int idx = i * m + j;
            
            /* Three independent recurrence chains */
            int val1 = prev1 * 3 + a[idx] + 7;
            int val2 = prev2 * 5 - b[idx] ^ 0xAA;
            int val3 = (prev3 << 2) | c[idx] & 0x55;
            
            /* Update with cross dependencies */
            a[idx] = val1 + val2;
            b[idx] = val2 - val3;
            c[idx] = val3 ^ val1;
            
            /* Carry values to next iteration */
            prev1 = val1;
            prev2 = val2;
            prev3 = val3;
            
            /* Additional operations to prevent optimization */
            a[idx-1] += (val1 & 0xF) << 4;
            b[idx-1] ^= (val2 >> 4) & 0xF;
        }
        
        global_sum += prev1 + prev2 + prev3;
    }
}

/* Test 5: Mixed types and operations for architecture-specific scheduling */
void test_mixed_operations(short *sdata, int *idata, long *ldata, int n) {
    short s_acc = sdata[0];
    int i_acc = idata[0];
    long l_acc = ldata[0];
    
    for (int i = 1; i < n; i++) {
        /* Type mixing operations */
        s_acc = (s_acc * 3 + sdata[i]) & 0x7FFF;
        i_acc = i_acc ^ (i_acc << 3) + idata[i];
        l_acc = l_acc + (l_acc >> 2) * ldata[i];
        
        /* Cross-type dependencies */
        sdata[i] = s_acc + (i_acc & 0xFFFF);
        idata[i] = i_acc ^ (l_acc & 0xFFFFFFFF);
        ldata[i] = l_acc + s_acc;
        
        /* Additional pressure operations */
        s_acc += (i_acc >> 16) & 0xFF;
        i_acc ^= (l_acc >> 32) & 0xFF;
        l_acc += (s_acc * i_acc);
    }
    
    global_sum += s_acc + i_acc + (l_acc & 0xFFFFFFFF);
}

/* PowerPC-specific test with potential for FP register moves */
#ifdef __powerpc__
void test_powerpc_fp(double *a, double *b, int n) {
    double fp1 = a[0], fp2 = b[0];
    double fp3 = 1.0, fp4 = 2.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple FP operations that use many registers */
        fp1 = fp1 * 1.5 + a[i];
        fp2 = fp2 / 1.5 - b[i];
        fp3 = fp3 * fp1 + fp2;
        fp4 = fp4 / fp1 - fp2;
        
        /* Cross dependencies */
        a[i] = fp1 + fp3;
        b[i] = fp2 - fp4;
        
        /* More operations for pressure */
        fp1 = __builtin_fma(fp1, fp3, fp4);
        fp2 = __builtin_fma(fp2, fp4, fp3);
    }
    
    global_sum += (long)(fp1 + fp2 + fp3 + fp4);
}
#endif

/* Main driver with hot loops */
int main() {
    /* Allocate and initialize data */
    int *idata1 = malloc(SIZE * sizeof(int));
    int *idata2 = malloc(SIZE * sizeof(int));
    int *idata3 = malloc(SIZE * sizeof(int));
    int *idata4 = malloc(SIZE * sizeof(int));
    double *fdata1 = malloc(SIZE * sizeof(double));
    double *fdata2 = malloc(SIZE * sizeof(double));
    double *fdata3 = malloc(SIZE * sizeof(double));
    short *sdata = malloc(SIZE * sizeof(short));
    long *ldata = malloc(SIZE * sizeof(long));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        idata1[i] = rand() % 100;
        idata2[i] = rand() % 100;
        idata3[i] = rand() % 100;
        idata4[i] = rand() % 100;
        fdata1[i] = (double)(rand() % 100) / 10.0;
        fdata2[i] = (double)(rand() % 100) / 10.0;
        fdata3[i] = (double)(rand() % 100) / 10.0;
        sdata[i] = rand() % 1000;
        ldata[i] = rand() % 1000;
    }
    
    /* Hot loop running tests many times */
    for (int iter = 0; iter < ITERS; iter++) {
        test_multi_recurrence_int(idata1, idata2, idata3, idata4, SIZE);
        test_float_recurrence(fdata1, fdata2, fdata3, SIZE);
        test_pointer_chasing(idata1, SIZE, 4);
        test_nested_loops(idata1, idata2, idata3, 64, 16);
        test_mixed_operations(sdata, idata1, ldata, SIZE);
        
        #ifdef __powerpc__
        test_powerpc_fp(fdata1, fdata2, SIZE);
        #endif
        
        /* Modify data slightly each iteration */
        idata1[iter % SIZE] ^= iter;
        fdata1[iter % SIZE] += 0.1;
    }
    
    /* Output result to prevent optimization */
    printf("Final sum: %ld\n", global_sum);
    
    /* Cleanup */
    free(idata1);
    free(idata2);
    free(idata3);
    free(idata4);
    free(fdata1);
    free(fdata2);
    free(fdata3);
    free(sdata);
    free(ldata);
    
    return 0;
}

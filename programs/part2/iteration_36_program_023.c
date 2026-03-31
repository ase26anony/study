/* sel-sched-coverage.c
 * Designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-coverage.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force memory dependencies and prevent optimization */
#define NOINLINE __attribute__((noinline))
#define VOLATILE_DEP(x) asm volatile("" : "+g" (x))

/* Complex loop with multiple dependencies and operations */
static inline NOINLINE void compute_loop(int *restrict arr1, int *arr2, 
                                         float *restrict farr, double *darr,
                                         int n, int seed) {
    volatile int vol_counter = seed;  /* Prevent optimization */
    int temp1 = 0, temp2 = 0;
    float ftemp = 1.0f;
    double dtemp = 2.0;
    
    /* Hot loop with carried dependencies and multiple basic blocks */
    for (int i = 0; i < n; i++) {
        /* Multiple independent operations */
        temp1 = arr1[i] * 3 + temp1;           /* Integer multiplication with carry */
        temp2 = arr2[i % 16] ^ temp2;          /* XOR with array access */
        ftemp = farr[i] * 0.5f + ftemp * 1.1f; /* FP multiply-add */
        dtemp = dtemp / 1.01 + darr[i % 8];    /* FP division with array access */
        
        /* Memory store with potential aliasing */
        arr1[i] = temp1 + vol_counter;
        arr2[i % 16] = temp2 ^ i;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            ftemp = ftemp * 2.0f - 1.0f;
            dtemp = dtemp + 3.14159;
            vol_counter++;  /* Volatile dependency */
        } else if (i % 13 == 0) {
            ftemp = ftemp / 1.5f;
            dtemp = dtemp - 2.71828;
        }
        
        /* Additional arithmetic to increase instruction mix */
        if (i % 3 == 0) {
            temp1 = (temp1 << 2) | (temp1 >> 30);  /* Rotation */
            temp2 = temp2 * 7 - 11;
        }
        
        /* Force dependency chain */
        VOLATILE_DEP(vol_counter);
        ftemp += (float)vol_counter * 0.001f;
    }
    
    /* Store final results */
    arr1[0] = temp1;
    arr2[0] = temp2;
    farr[0] = ftemp;
    darr[0] = dtemp;
}

/* Secondary computation with different patterns */
static inline NOINLINE void compute_loop2(double *restrict d1, double *d2,
                                          int *irest, float *frest, int m) {
    double acc1 = d1[0], acc2 = d2[0];
    int iacc = irest[0];
    float facc = frest[0];
    
    for (int j = 0; j < m; j++) {
        /* Mixed precision computations */
        acc1 = acc1 * 1.1 + (double)j * 0.01;
        acc2 = acc2 / 1.05 - (double)(j % 5) * 0.1;
        
        /* Integer computations with branching */
        iacc = (iacc * 3 + j) & 0xFFFF;
        facc = facc + (float)iacc * 0.001f;
        
        /* Complex conditional */
        if (j % 11 == 0) {
            acc1 = acc1 - acc2;
            iacc = iacc ^ 0xAAAA;
        } else if (j % 17 == 0) {
            acc2 = acc2 * 2.0;
            facc = facc * 1.5f;
        }
        
        /* Memory operations */
        d1[j % 4] = acc1;
        d2[j % 4] = acc2;
        irest[j % 4] = iacc;
        frest[j % 4] = facc;
    }
}

int main(void) {
    const int N = 1024;
    const int ITERS = 1000;
    
    /* Allocate and initialize arrays with different patterns */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(16 * sizeof(int));
    float *farr = (float*)malloc(N * sizeof(float));
    double *darr = (double*)malloc(8 * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 13 + 7) & 0xFF;
        farr[i] = (float)i * 0.123f + 1.0f;
    }
    for (int i = 0; i < 16; i++) {
        arr2[i] = (i * 17 + 3) & 0xFF;
    }
    for (int i = 0; i < 8; i++) {
        darr[i] = (double)i * 0.456 + 2.0;
    }
    
    /* Additional arrays for second computation */
    double *d1 = (double*)malloc(4 * sizeof(double));
    double *d2 = (double*)malloc(4 * sizeof(double));
    int *irest = (int*)malloc(4 * sizeof(int));
    float *frest = (float*)malloc(4 * sizeof(float));
    
    for (int i = 0; i < 4; i++) {
        d1[i] = (double)i * 0.789;
        d2[i] = (double)i * 0.321;
        irest[i] = i * 11;
        frest[i] = (float)i * 0.654f;
    }
    
    /* Main computation sequence - this should trigger selective scheduling */
    int checksum = 0;
    for (int iter = 0; iter < ITERS; iter++) {
        /* Call the hot loop multiple times with different seeds */
        compute_loop(arr1, arr2, farr, darr, N, iter);
        compute_loop2(d1, d2, irest, frest, 512);
        
        /* Update checksum to prevent dead code elimination */
        checksum ^= arr1[iter % N] ^ arr2[iter % 16];
        checksum ^= (int)farr[iter % N] ^ (int)darr[iter % 8];
        checksum ^= (int)d1[iter % 4] ^ (int)d2[iter % 4];
        checksum ^= irest[iter % 4] ^ (int)frest[iter % 4];
    }
    
    /* Final reduction to ensure all computations are used */
    int final_result = checksum;
    for (int i = 0; i < N; i++) {
        final_result += arr1[i] - arr2[i % 16];
    }
    for (int i = 0; i < 4; i++) {
        final_result += (int)d1[i] + (int)d2[i] + irest[i];
    }
    
    printf("Result: %d\n", final_result & 0xFF);  /* Print to prevent optimization */
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    free(d1);
    free(d2);
    free(irest);
    free(frest);
    
    return 0;
}

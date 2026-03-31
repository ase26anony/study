/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation function with loop-carried dependencies */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d, float *result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;  /* Register for anti/output dependencies */
    
    /* Outer loop with parameterized bound */
    for (i = 1; i < n; ++i) {
        /* 1. FLOW DEPENDENCY (Register): Reduction pattern */
        sum += a[i] * 1.5f;
        
        /* 2. FLOW DEPENDENCY (Memory): Array copy with one-element shift */
        b[i] = a[i] + a[i-1];  /* Loop-carried on a[] */
        
        /* 3. ANTI and OUTPUT DEPENDENCIES (Register): Swap-like operation */
        tmp_reg = c[i];
        c[i] = d[i];
        d[i] = tmp_reg;
        
        /* 4. CONDITION DEPENDENCY: If statement with computed condition */
        float threshold = sum * 0.01f;
        if (b[i] > threshold) {
            /* Memory dependency through conditional update */
            b[i] = conditional_update(b[i], threshold);
        }
        
        /* 5. Nested loop with varying bound - prevents unrolling */
        #pragma GCC ivdep  /* Assert no loop-carried memory deps for inner loop */
        for (j = i; j < n && j < i + 3; ++j) {
            /* Complex addressing: non-affine access pattern */
            int idx = i + (j % 2);  /* Could be 0 or 1 offset */
            if (idx < n) {
                /* Memory flow dependency with non-linear index */
                d[idx] = d[idx] * 0.9f + c[j] * 0.1f;
            }
        }
        
        /* 6. OUTPUT DEPENDENCY (Memory): Multiple writes to same array */
        a[i] = a[i] * 0.8f;  /* Write after read from line 2 */
    }
    
    /* 7. Additional independent arithmetic chains for parallelism potential */
    float chain_x = 0.0f, chain_y = 0.0f, chain_z;
    for (i = 0; i < n; ++i) {
        chain_x = a[i] * 2.0f;      /* Independent chain 1 */
        chain_y = b[i] + 1.0f;      /* Independent chain 2 */
        chain_z = chain_x + chain_y; /* Convergence point */
        d[i] += chain_z * 0.5f;     /* Memory output dependency */
    }
    
    *result = sum;
}

/* Second function with different dependency pattern */
void compute2(int m, int n, float *restrict arr1, float *restrict arr2) {
    int i, k;
    float acc1 = 0.0f, acc2 = 0.0f;
    
    /* Loop with multiple accumulators and pointer arithmetic */
    for (i = 0; i < n; ++i) {
        float *ptr1 = arr1 + i;
        float *ptr2 = arr2 + (i % m);
        
        /* True/flow dependencies on accumulators */
        acc1 = acc1 + *ptr1 * 0.3f;
        acc2 = acc2 + *ptr2 * 0.7f;
        
        /* Anti-dependency: read then write */
        float temp = *ptr1;
        *ptr1 = acc1;
        
        /* Output dependency: write after write */
        *ptr2 = temp;
        *ptr2 = *ptr2 * acc2;  /* Second write to same location */
        
        /* Complex condition with register dependencies */
        if (acc1 > acc2) {
            for (k = 0; k < 2; ++k) {
                /* Small inner loop with induction variable */
                arr1[(i + k) % n] += (acc1 - acc2) * k;
            }
        }
    }
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    float *a, *b, *c, *d, *arr1, *arr2;
    float result1, result2;
    int i;
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(size1 * sizeof(float));
    b = (float*)malloc(size1 * sizeof(float));
    c = (float*)malloc(size1 * sizeof(float));
    d = (float*)malloc(size1 * sizeof(float));
    arr1 = (float*)malloc(size1 * sizeof(float));
    arr2 = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with sequential and semi-random values */
    for (i = 0; i < size1; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i % 100) * 0.2f;
        c[i] = (float)(i * 3) * 0.05f;
        d[i] = (float)(i * 7) * 0.03f;
        arr1[i] = (float)(i % 50) * 0.15f;
    }
    for (i = 0; i < size2; ++i) {
        arr2[i] = (float)(i % 75) * 0.25f;
    }
    
    /* Call compute functions multiple times with different sizes
     * to increase chance of DDG construction */
    compute(size1, a, b, c, d, &result1);
    compute(size1/2, a, b, c, d, &result1);
    compute(size1/4, a, b, c, d, &result1);
    
    compute2(size2, size1, arr1, arr2);
    compute2(size2/2, size1/2, arr1, arr2);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = result1;
    for (i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (i = 0; i < size2; ++i) {
        checksum += arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}

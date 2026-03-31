/* Target: ddg.cc lines 749-757 - DDG edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Helper with always_inline to force loop body expansion */
static inline __attribute__((always_inline)) 
void process_inner(int i, float *restrict a, float *restrict b, 
                   float *restrict x, float *restrict y, 
                   float *restrict sum, float scalar) {
    /* Register flow dependency on sum */
    *sum += a[i] * scalar;
    
    /* Memory flow dependency with one-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1];
    } else {
        b[i] = a[i];
    }
    
    /* Anti and output dependencies through swap */
    float tmp = x[i];
    x[i] = y[i];
    y[i] = tmp;
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y, 
             float *restrict result) {
    float sum = 0.0f;
    float scalar = 2.5f;
    
    /* Outer loop with parameter bound */
    for (int i = 0; i < n; ++i) {
        /* Mixed index calculation for memory edge complexity */
        int idx1 = i;
        int idx2 = i + (i % 3);  /* Non-affine index */
        
        /* Memory dependency with non-linear access */
        float val1 = a[idx1];
        float val2 = (idx2 < n) ? a[idx2] : 0.0f;
        
        /* Register dependency chain */
        float temp_reg = val1 * scalar;
        sum += temp_reg;
        
        /* Conditional creating control dependency */
        if (temp_reg > 100.0f) {
            /* Memory output dependency */
            b[i] = val2;
        } else {
            /* Memory flow dependency with offset */
            if (i > 0) {
                b[i] = a[i] + a[i-1];
            } else {
                b[i] = a[i];
            }
        }
        
        /* Multiple independent chains for parallelism potential */
        float chain1 = x[i] * 1.5f;
        float chain2 = y[i] + 2.0f;
        float chain3 = chain1 + chain2;
        
        /* Anti-dependency through register reuse */
        float reuse_reg = chain3;
        x[i] = reuse_reg;
        reuse_reg = chain1 - chain2;  /* Reusing register */
        y[i] = reuse_reg;
        
        /* Call inline helper for additional complexity */
        process_inner(i, a, b, x, y, &sum, scalar);
        
        /* Nested loop with dependent bound */
        for (int j = i; j < n && j < i + 4; ++j) {
            /* Cross-iteration memory dependency */
            if (j > 0) {
                x[j] += x[j-1] * 0.1f;
            }
        }
    }
    
    *result = sum;
}

/* Alternate computation with different pattern */
void compute2(int n, float *restrict a, float *restrict b) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Assert no loop-carried dependencies (may trigger DDG to verify) */
        float t = a[i];
        a[i] = b[i];
        b[i] = t;
        
        /* Reduction with multiple accumulators */
        acc1 += a[i];
        acc2 += b[i];
        
        /* Complex addressing */
        int idx = (i * 7) % n;
        if (idx > 0) {
            a[idx] = a[idx] + a[idx-1];
        }
    }
    
    /* Use results to prevent elimination */
    a[0] = acc1 + acc2;
}

int main(void) {
    const int max_size = 1000;
    float *a = (float*)malloc(max_size * sizeof(float));
    float *b = (float*)malloc(max_size * sizeof(float));
    float *x = (float*)malloc(max_size * sizeof(float));
    float *y = (float*)malloc(max_size * sizeof(float));
    float result;
    
    /* Initialize with pattern */
    for (int i = 0; i < max_size; ++i) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)(i % 100);
        x[i] = (float)(i * 2);
        y[i] = (float)(i * 3);
    }
    
    /* Call with different sizes to trigger various optimizations */
    for (int size = 100; size <= max_size; size += 300) {
        compute(size, a, b, x, y, &result);
        compute2(size, a, b);
        
        /* Checksum to prevent dead code elimination */
        float checksum = 0.0f;
        for (int i = 0; i < size; ++i) {
            checksum += a[i] + b[i] + x[i] + y[i];
        }
        printf("Size %d: result = %f, checksum = %f\n", 
               size, result, checksum);
    }
    
    free(a);
    free(b);
    free(x);
    free(y);
    
    return 0;
}

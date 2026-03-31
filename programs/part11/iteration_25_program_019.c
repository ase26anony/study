/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1000

/* Helper function with inline hint to keep loop in context */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* acc, float* tmp) {
    /* Memory flow dependency: uses previous iteration's a[prev] */
    float val = a[i] + a[prev];
    
    /* Register flow dependency: accumulator chain */
    *acc = *acc + val * 1.5f;
    
    /* Memory anti-dependency: read b[i] before writing it */
    float old_b = b[i];
    b[i] = val * old_b;  /* Output dependency on b[i] */
    
    /* Conditional creating control dependencies */
    if (*acc > 100.0f) {
        *acc = *acc * 0.9f;  /* Register output dependency */
    }
    
    /* Swap operation with register anti/output dependencies */
    *tmp = c[i];
    c[i] = d[i];
    d[i] = *tmp;
}

/* Main computation with loop-carried dependencies */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float acc = 0.0f;
    float tmp_reg;
    
    /* Loop with cross-iteration dependencies */
    #pragma GCC ivdep
    for (int i = 1; i < n; ++i) {
        int prev = i - 1;
        
        /* Complex addressing to prevent simple analysis */
        int idx_a = i + (prev % 3);
        int idx_b = i - (i % 2);
        
        /* Ensure bounds */
        if (idx_a >= n) idx_a = n - 1;
        if (idx_b < 0) idx_b = 0;
        
        /* Nested loop-like pattern to increase complexity */
        for (int j = i; j < n && j < i + 2; ++j) {
            /* Mixed memory accesses with loop-carried dependency */
            a[idx_a] = a[idx_a] * 0.99f + b[idx_b];
            
            /* Register dependency chain */
            float local_acc = acc;
            local_acc += a[idx_a] * 0.1f;
            
            /* Conditional control flow */
            if (local_acc > 50.0f) {
                b[idx_b] = local_acc * 0.8f;
            }
        }
        
        /* Process elements with various dependencies */
        process_element(a, b, c, d, i, prev, &acc, &tmp_reg);
        
        /* Additional output dependency on accumulator */
        acc = acc + 1.0f;
    }
    
    /* Final reduction to prevent elimination */
    a[0] = acc;
}

/* Secondary computation with different pattern */
void __attribute__((noinline))
compute2(int m, float* restrict x, float* restrict y) {
    float sum1 = 0.0f, sum2 = 0.0f;
    
    /* Loop with multiple independent chains */
    for (int i = 0; i < m; ++i) {
        /* Independent computation chains that converge */
        float chain1 = x[i] * 2.0f;
        float chain2 = y[i] + 3.0f;
        float chain3 = chain1 * 1.5f;
        float chain4 = chain2 - 1.0f;
        
        /* Converge chains with flow dependencies */
        sum1 = sum1 + chain3;
        sum2 = sum2 + chain4;
        
        /* Cross-iteration memory dependency */
        if (i > 0) {
            x[i] = x[i] + y[i-1];
        }
        
        /* Register anti-dependency via swap */
        float temp = x[i];
        x[i] = y[i];
        y[i] = temp;
        
        /* Conditional creating control flow */
        if (sum1 > sum2) {
            sum1 = sum1 * 0.95f;
        } else {
            sum2 = sum2 * 0.95f;
        }
    }
    
    /* Prevent dead code elimination */
    x[0] = sum1 + sum2;
}

int main() {
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(SIZE * sizeof(float));
    float* b = (float*)malloc(SIZE * sizeof(float));
    float* c = (float*)malloc(SIZE * sizeof(float));
    float* d = (float*)malloc(SIZE * sizeof(float));
    float* x = (float*)malloc(SIZE * sizeof(float));
    float* y = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(SIZE - i) * 0.2f;
        c[i] = (float)(i % 10) * 1.5f;
        d[i] = (float)(i % 5) * 2.0f;
        x[i] = (float)i * 0.3f;
        y[i] = (float)i * 0.4f;
    }
    
    /* Call compute with different sizes to trigger various optimizations */
    for (int iter = 0; iter < 3; ++iter) {
        int size = SIZE - iter * 100;
        
        compute(size, a, b, c, d);
        compute2(size, x, y);
        
        /* Mix up data for next iteration */
        for (int i = 1; i < size; ++i) {
            a[i] = b[i-1] * 0.9f;
            x[i] = y[i] * 1.1f;
        }
    }
    
    /* Compute checksum to prevent elimination */
    float checksum = 0.0f;
    for (int i = 0; i < SIZE; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(x); free(y);
    
    return 0;
}

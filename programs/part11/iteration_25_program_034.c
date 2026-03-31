/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* acc, float* tmp) {
    /* Memory flow dependency: b[i] depends on a[i] and a[i-1] */
    b[i] = a[i] + a[prev];
    
    /* Register flow dependency: accumulator chain */
    *acc = *acc + b[i] * 1.5f;
    
    /* Anti-dependency: swap using temporary */
    *tmp = c[i];
    c[i] = d[i];
    d[i] = *tmp;
    
    /* Output dependency: conditional update creates different edge types */
    if (*acc > 100.0f) {
        /* Condition edge from acc comparison to this store */
        a[i] = *acc * 0.5f;
        *acc = *acc - 100.0f;  /* Register output dependency */
    }
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float acc = 0.0f;
    float tmp;
    
    /* Outer loop with parameterized bound */
    for (int i = 0; i < n; ++i) {
        int prev = (i > 0) ? i - 1 : n - 1;  /* Circular dependency */
        
        /* Complex addressing to prevent simple analysis */
        int idx1 = i + (i % 3);
        int idx2 = i + (i % 2);
        
        /* Memory anti-dependency with non-affine index */
        float t = c[idx1 % n];
        c[idx1 % n] = d[idx2 % n];
        d[idx2 % n] = t;
        
        /* Call inline function to increase instruction mix */
        process_element(a, b, c, d, i, prev, &acc, &tmp);
        
        /* Nested loop with dependent bound - creates more complex DDG */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Cross-iteration memory dependency */
            b[j] = b[j] + a[i] * 0.3f;
            
            /* Register dependency chain */
            acc = acc - d[j % n] * 0.1f;
        }
        
        /* Another independent operation to provide scheduling freedom */
        float scale = (i % 2) ? 1.2f : 0.8f;
        a[i] = a[i] * scale;
    }
}

/* Alternative loop structure that might trigger different scheduler paths */
void __attribute__((noinline))
compute2(int m, int n, float* restrict x, float* restrict y) {
    /* Two-dimensional loop with mixed dependencies */
    for (int i = 0; i < m; ++i) {
        float row_acc = 0.0f;
        
        #pragma GCC ivdep
        for (int j = 0; j < n; ++j) {
            /* Assert no loop-carried dependencies (but scheduler will check) */
            int idx = i * n + j;
            
            /* Flow dependency within iteration only */
            float val = x[idx] * 2.0f + y[idx];
            row_acc += val;
            
            /* Anti-dependency within iteration */
            float old = y[idx];
            y[idx] = val;
            x[idx] = old;
        }
        
        /* Cross-iteration register dependency */
        y[i * n] += row_acc;
    }
}

int main() {
    const int size = 1000;
    const int size2 = 100;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(size * sizeof(float));
    float* b = (float*)malloc(size * sizeof(float));
    float* c = (float*)malloc(size * sizeof(float));
    float* d = (float*)malloc(size * sizeof(float));
    float* x = (float*)malloc(size2 * size2 * sizeof(float));
    float* y = (float*)malloc(size2 * size2 * sizeof(float));
    
    /* Initialize with pattern (not random to keep reproducible) */
    for (int i = 0; i < size; ++i) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = i * 0.3f;
        d[i] = i * 0.4f;
    }
    
    for (int i = 0; i < size2 * size2; ++i) {
        x[i] = (i % 10) * 0.1f;
        y[i] = (i % 5) * 0.2f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(size, a, b, c, d);
    compute(size/2, a, b, c, d);
    compute(size/4, a, b, c, d);
    
    compute2(size2, size2, x, y);
    compute2(size2/2, size2, x, y);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int i = 0; i < size2 * size2; ++i) {
        checksum += x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}

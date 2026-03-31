/* Test program to cover DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_iteration(float *restrict a, float *restrict b, float *restrict c,
                  float *restrict d, int i, float *acc, float *tmp_var) {
    /* Register flow dependency: accumulator chain */
    *acc += a[i] * 1.5f;
    
    /* Memory flow dependency with one-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1] * 0.7f;
    } else {
        b[i] = a[i];
    }
    
    /* Anti-dependency and output dependency via swap */
    *tmp_var = c[i];
    c[i] = d[i];
    d[i] = *tmp_var;
    
    /* Complex memory access pattern for non-affine analysis */
    int idx = i + (i % 3) - 1;
    if (idx >= 0 && idx < 1000) {
        a[i] += b[idx] * 0.3f;
    }
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict d) {
    float acc = 0.0f;
    float tmp = 0.0f;
    
    /* Outer loop with parameterized bound */
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Register dependencies */
        float local_acc = acc;
        
        /* Conditional creating control dependency */
        if (i % 2 == 0) {
            /* Memory dependency chain */
            a[i] = b[i] * 2.0f - c[i];
        } else {
            /* Alternative path with different dependencies */
            a[i] = c[i] * 0.5f + d[i];
        }
        
        /* Call to inlined function with mixed dependencies */
        process_iteration(a, b, c, d, i, &acc, &tmp);
        
        /* Nested loop with dependent bound - creates complex DDG */
        for (int j = i; j < n && j < i + 4; ++j) {
            /* Cross-iteration memory dependency */
            d[j] += a[i] * 0.1f;
            
            /* Register output dependency */
            tmp = a[i] + b[j];
            c[j] = tmp * 0.8f;
        }
        
        /* Reduction with loop-carried dependency */
        acc = local_acc + a[i];
        
        /* Another anti-dependency */
        float old_val = b[i];
        b[i] = acc * 0.25f;
        c[i] = old_val + d[i];
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Secondary function with different loop structure */
void __attribute__((noinline))
compute2(int m, int n, float *restrict x, float *restrict y) {
    /* Double nested loops for more complex DDG */
    for (int i = 0; i < m; ++i) {
        float sum = 0.0f;
        #pragma GCC ivdep
        for (int j = 0; j < n; ++j) {
            /* Strided access pattern */
            int idx = i * n + j;
            
            /* Multiple independent chains that can be parallelized */
            float t1 = x[idx] * 1.1f;
            float t2 = y[idx] + 2.2f;
            float t3 = t1 - t2;
            
            /* Loop-carried dependency */
            sum += t3;
            
            /* Conditional store with control dependency */
            if (sum > 0) {
                y[idx] = t3 * 0.9f;
            }
            
            /* Output dependency */
            x[idx] = sum * 0.1f;
        }
        
        /* Cross-iteration dependency through array */
        if (i > 0) {
            x[i * n] += y[(i-1) * n];
        }
    }
}

int main(void) {
    const int size = 1000;
    const int size2 = 100;
    
    /* Allocate and initialize arrays */
    float *a = (float*)malloc(size * sizeof(float));
    float *b = (float*)malloc(size * sizeof(float));
    float *c = (float*)malloc(size * sizeof(float));
    float *d = (float*)malloc(size * sizeof(float));
    float *x = (float*)malloc(size2 * size2 * sizeof(float));
    float *y = (float*)malloc(size2 * size2 * sizeof(float));
    
    /* Initialize with pattern (not all zeros) */
    for (int i = 0; i < size; ++i) {
        a[i] = (float)(i % 10) * 0.5f;
        b[i] = (float)(i % 7) * 0.3f;
        c[i] = (float)(i % 5) * 0.7f;
        d[i] = (float)(i % 11) * 0.2f;
    }
    
    for (int i = 0; i < size2 * size2; ++i) {
        x[i] = (float)(i % 13) * 0.4f;
        y[i] = (float)(i % 17) * 0.6f;
    }
    
    /* Call compute functions multiple times with different sizes
       to increase chance of DDG construction */
    compute(size, a, b, c, d);
    compute(size / 2, a, b, c, d);
    compute(size * 3 / 4, a, b, c, d);
    
    compute2(size2, size2, x, y);
    compute2(size2 / 2, size2, x, y);
    
    /* Compute checksum to prevent dead code elimination */
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

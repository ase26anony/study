/* Test program to trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* acc, float* tmp) {
    /* Memory flow dependency: b[i] depends on a[i] and a[i-1] */
    b[i] = a[i] + a[prev];
    
    /* Register flow dependency: accumulator chain */
    *acc += b[i] * 1.5f;
    
    /* Anti-dependency: swap using temporary */
    *tmp = c[i];
    c[i] = d[i];
    d[i] = *tmp;
    
    /* Output dependency: conditional update creates different edge types */
    if (*acc > 100.0f) {
        /* Condition edge from acc comparison to this store */
        a[i] = *acc * 0.5f;
        *acc = *acc * 0.1f;  /* Register output dependency */
    }
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float acc = 0.0f;
    float tmp;
    
    /* Outer loop with parameter bound */
    for (int i = 0; i < n; ++i) {
        int prev = (i > 0) ? i - 1 : n - 1;  /* Circular dependency */
        
        /* Call inlined helper to increase instruction mix */
        process_element(a, b, c, d, i, prev, &acc, &tmp);
        
        /* Additional independent operations for parallel potential */
        float x = a[i] * 2.0f;
        float y = c[i] + d[i];
        b[i] += x * y;  /* Memory flow dependency on b[i] */
        
        /* Nested loop with dependent bound - creates complex addressing */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Non-affine memory access pattern */
            int idx = i + (j % 2);
            if (idx < n) {
                /* Memory anti-dependency: read a[idx] then modify it */
                float val = a[idx];
                a[idx] = val * 0.8f + b[j] * 0.2f;
            }
        }
    }
}

/* Alternative computation with pragma to influence scheduling */
#pragma GCC ivdep
void __attribute__((noinline))
compute_ivdep(int n, float* restrict a, float* restrict b) {
    /* Loop with asserted independence - compiler may still build DDG to verify */
    float acc = 0.0f;
    for (int i = 1; i < n; ++i) {
        /* Clear flow dependency chain */
        b[i] = a[i] * 3.0f + a[i-1] * 2.0f;
        acc += b[i];
        
        /* Register dependencies with multiple uses */
        float t1 = acc * 0.5f;
        float t2 = t1 * 0.3f;
        a[i] = t1 + t2;  /* Memory output dependency on a[i] */
    }
}

int main() {
    const int sizes[] = {500, 1000, 1500};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    float checksum = 0.0f;
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate with alignment to help vectorization analysis */
        float* a = (float*)aligned_alloc(16, n * sizeof(float));
        float* b = (float*)aligned_alloc(16, n * sizeof(float));
        float* c = (float*)aligned_alloc(16, n * sizeof(float));
        float* d = (float*)aligned_alloc(16, n * sizeof(float));
        
        /* Initialize with pattern (not random for reproducibility) */
        for (int i = 0; i < n; ++i) {
            a[i] = (float)i * 0.1f;
            b[i] = (float)(n - i) * 0.2f;
            c[i] = (float)(i % 10) * 1.5f;
            d[i] = (float)(i % 5) * 2.0f;
        }
        
        /* Call both computation functions */
        compute(n, a, b, c, d);
        compute_ivdep(n, a, b);
        
        /* Calculate checksum to prevent dead code elimination */
        for (int i = 0; i < n; ++i) {
            checksum += a[i] + b[i] + c[i] + d[i];
        }
        
        free(a);
        free(b);
        free(c);
        free(d);
    }
    
    printf("Checksum: %f\n", checksum);
    return 0;
}

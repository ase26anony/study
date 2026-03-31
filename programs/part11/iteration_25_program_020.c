/* Test program to trigger DDG edge creation in GCC's instruction scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* acc, float* tmp) {
    /* Memory flow dependency: uses previous iteration's a[i-1] */
    b[i] = a[i] + a[prev];
    
    /* Register flow dependency: accumulator chain */
    *acc += c[i] * 1.5f;
    
    /* Anti and output dependencies: swap with temporary */
    *tmp = d[i];
    d[i] = c[i];
    c[i] = *tmp;
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
        
        /* Conditional creating control dependency */
        if (acc > 100.0f) {
            acc *= 0.99f;  /* Register output dependency */
        }
        
        /* Process element with mixed dependencies */
        process_element(a, b, c, d, i, prev, &acc, &tmp);
        
        /* Nested inner loop with varying bound - creates complex DDG */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Memory anti-dependency: read then write same location */
            float val = b[j];
            b[j] = val * 0.5f + a[j % n];
            
            /* Register flow in inner loop */
            acc += val * 0.1f;
        }
        
        /* Non-affine array access - harder to analyze */
        int idx = i + (i % 2);
        if (idx < n) {
            c[idx] = b[i] * 2.0f;  /* Memory flow to different index */
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Alternate computation with different pattern */
void __attribute__((noinline))
compute2(int m, int n, float* restrict x, float* restrict y) {
    float sum = 0.0f;
    
    #pragma GCC ivdep
    for (int i = 1; i < m; ++i) {
        /* Loop-carried flow dependency */
        y[i] = y[i-1] * 1.1f + x[i];
        
        /* Independent computation chain */
        float t1 = x[i] * 2.0f;
        float t2 = t1 + 1.0f;
        float t3 = t2 * 0.5f;
        
        /* Conditional with register dependency */
        if (t3 > sum) {
            sum = t3;
        }
        
        /* Output dependency on scalar */
        t1 = t3 * 0.8f;
        sum += t1;
    }
    
    x[0] = sum;
}

int main() {
    const int N = 1000;
    const int M = 500;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(N * sizeof(float));
    float* b = (float*)malloc(N * sizeof(float));
    float* c = (float*)malloc(N * sizeof(float));
    float* d = (float*)malloc(N * sizeof(float));
    float* x = (float*)malloc(N * sizeof(float));
    float* y = (float*)malloc(N * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i % 10) * 0.2f;
        c[i] = (float)(i % 5) * 0.3f;
        d[i] = (float)(i % 7) * 0.4f;
        x[i] = (float)i * 0.05f;
        y[i] = (float)(i % 3) * 0.15f;
    }
    
    /* Call compute functions multiple times with different sizes */
    compute(N, a, b, c, d);
    compute(N/2, a, b, c, d);
    compute(N*3/4, a, b, c, d);
    
    compute2(M, N, x, y);
    compute2(M/2, N, x, y);
    
    /* Calculate checksum to prevent optimization */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(x); free(y);
    
    return 0;
}

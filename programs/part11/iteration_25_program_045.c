/* Test program to trigger DDG edge creation in GCC's scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                 float* restrict d, int i, int prev, float* acc, float* tmp) {
    /* Memory flow dependency: b depends on previous a element */
    b[i] = a[i] + a[prev];
    
    /* Register flow dependency: accumulator chain */
    *acc = *acc + a[i] * 1.5f;
    
    /* Anti and output dependencies: swap operation */
    *tmp = c[i];
    c[i] = d[i];
    d[i] = *tmp;
    
    /* Condition edge: depends on computed value */
    if (*acc > 100.0f) {
        *acc = *acc * 0.9f;
    }
}

/* Main computation with loop-carried dependencies */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float acc = 0.0f;
    float tmp;
    
    /* Try to hint no loop-carried memory dependencies */
    #pragma GCC ivdep
    for (int i = 1; i < n; ++i) {
        /* Complex addressing to create memory dependencies */
        int idx1 = i;
        int idx2 = i - 1;
        int idx3 = i + (i % 3);  /* Non-affine index */
        
        /* Multiple independent chains with dependencies */
        float t1 = a[idx1] * 2.0f;
        float t2 = a[idx2] * 3.0f;
        float t3 = t1 + t2;      /* Flow dependency on t1, t2 */
        
        /* Cross-iteration dependency on b */
        b[i] = b[i-1] + t3;
        
        /* Process with helper function */
        process_element(a, b, c, d, i, i-1, &acc, &tmp);
        
        /* Additional arithmetic chain */
        c[i] = c[i] * 0.5f + d[i] * 0.5f;
        
        /* Nested loop-like pattern to increase complexity */
        for (int j = i; j < n && j < i + 3; ++j) {
            d[j] = d[j] + a[i] * 0.1f;  /* Flow dependency on a[i] */
        }
    }
}

/* Another computation with different pattern */
void __attribute__((noinline))
compute2(int m, int n, float* restrict x, float* restrict y) {
    float sum = 0.0f;
    
    /* Outer loop with parameterized bounds */
    for (int i = 0; i < m; ++i) {
        float local_acc = 0.0f;
        
        /* Inner loop with dependency on outer index */
        for (int j = i; j < n; ++j) {
            /* Flow dependency on local_acc */
            local_acc += x[j] * y[i];
            
            /* Anti-dependency: read then write */
            float old = x[j];
            x[j] = y[i] * 2.0f;
            y[i] = old + 1.0f;
            
            /* Output dependency on local_acc */
            if (local_acc > 50.0f) {
                local_acc = local_acc / 2.0f;
            }
        }
        
        /* Cross-iteration flow dependency */
        sum += local_acc;
    }
    
    /* Prevent dead code elimination */
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
        y[i] = (float)(N - i) * 0.06f;
    }
    
    /* Call computations with different sizes to trigger various DDG constructions */
    compute(N, a, b, c, d);
    compute2(M, N, x, y);
    compute(N/2, a, b, c, d);
    compute2(M/2, N/2, x, y);
    
    /* Calculate checksum to prevent elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(x);
    free(y);
    
    return 0;
}

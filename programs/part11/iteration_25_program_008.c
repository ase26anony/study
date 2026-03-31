/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves -o test_ddg test_ddg.c */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_element(float* restrict a, float* restrict b, float* restrict c, 
                float* restrict d, int i, float* sum, float* tmp_swap) {
    /* Register flow dependency: sum accumulates across iterations */
    *sum += a[i] * 1.5f;
    
    /* Memory flow dependency with one-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1] * 0.7f;
    } else {
        b[i] = a[i];
    }
    
    /* Anti and output dependencies through swap operation */
    *tmp_swap = c[i];
    c[i] = d[i];
    d[i] = *tmp_swap;
    
    /* Complex memory addressing for non-affine access */
    int idx = i + (i % 3) - 1;
    if (idx >= 0 && idx < 1000) {
        a[idx] = a[idx] * 0.99f;  /* Output dependency on a[] */
    }
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float* restrict a, float* restrict b, 
        float* restrict c, float* restrict d) {
    float sum = 0.0f;
    float tmp_swap;
    
    /* Outer loop with parameterized bound */
    for (int i = 0; i < n; ++i) {
        /* Register dependency carried across iterations */
        float local_acc = sum * 0.1f;
        
        /* Inner loop with dependent bound - creates scheduling complexity */
        for (int j = i; j < n && j < i + 5; ++j) {
            /* Memory anti-dependency: read then write */
            float temp = a[j];
            a[j] = b[j] + local_acc;
            local_acc += temp * 0.3f;
        }
        
        /* Process element with mixed dependencies */
        process_element(a, b, c, d, i, &sum, &tmp_swap);
        
        /* Condition edge: if statement depends on computed values */
        if (sum > 100.0f) {
            sum = sum * 0.9f;  /* Register output dependency */
        }
        
        /* Another memory flow dependency with stride */
        if (i >= 2) {
            d[i] = c[i-2] * 1.1f + c[i-1] * 0.9f;
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    a[0] += sum;
}

/* Secondary function with different access pattern */
void __attribute__((noinline))
compute2(int m, float* restrict x, float* restrict y) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    #pragma GCC ivdep
    for (int i = 0; i < m; ++i) {
        /* Independent chains that can be parallelized */
        float t1 = x[i] * 1.2f;
        float t2 = y[i] * 0.8f;
        float t3 = t1 + acc1;
        float t4 = t2 + acc2;
        
        /* Cross-chain dependency */
        x[i] = t3 + t4 * 0.5f;
        y[i] = t4 + t3 * 0.5f;
        
        /* Loop-carried register dependencies */
        acc1 = t3 * 0.7f;
        acc2 = t4 * 0.3f;
        
        /* Non-linear indexing */
        int idx = (i * 7) % m;
        if (idx != i) {
            y[idx] = x[i] * 0.1f;  /* Memory flow to different location */
        }
    }
}

int main() {
    const int size = 1000;
    float* a = (float*)malloc(size * sizeof(float));
    float* b = (float*)malloc(size * sizeof(float));
    float* c = (float*)malloc(size * sizeof(float));
    float* d = (float*)malloc(size * sizeof(float));
    float* x = (float*)malloc(size * sizeof(float));
    float* y = (float*)malloc(size * sizeof(float));
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < size; ++i) {
        a[i] = (i % 37) * 0.1f;
        b[i] = (i % 41) * 0.2f;
        c[i] = (i % 43) * 0.3f;
        d[i] = (i % 47) * 0.4f;
        x[i] = (i % 53) * 0.5f;
        y[i] = (i % 59) * 0.6f;
    }
    
    /* Call compute with different sizes to trigger various DDG constructions */
    compute(size, a, b, c, d);
    compute(size/2, a, b, c, d);
    compute(size*3/4, a, b, c, d);
    
    compute2(size, x, y);
    compute2(size/3, x, y);
    
    /* Compute checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i] + x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(a); free(b); free(c); free(d); free(x); free(y);
    return 0;
}

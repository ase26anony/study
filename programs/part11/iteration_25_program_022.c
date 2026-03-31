/* Target: ddg.cc lines 749-757 - create_ddg_edge() field assignments */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_inner(int i, float *restrict a, float *restrict b, 
                   float *restrict x, float *restrict y, 
                   float *sum, float *tmp_reg) {
    /* Register flow dependency - sum accumulates across iterations */
    *sum += a[i] * 1.5f;
    
    /* Memory flow dependency with one-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1];  /* True/flow dependency on a[i-1] */
    } else {
        b[i] = a[i];
    }
    
    /* Anti and output dependencies through swap with temporary */
    *tmp_reg = x[i];      /* Read x[i] */
    x[i] = y[i];          /* Write x[i] - output dep with previous read */
    y[i] = *tmp_reg;      /* Write y[i] - anti dep with previous read of x[i] */
    
    /* Condition dependency */
    if (*sum > 100.0f) {  /* Condition depends on sum computed above */
        x[i] *= 0.5f;     /* Creates condition edge */
    }
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y) {
    float sum = 0.0f;
    float tmp_reg;
    
    /* Outer loop with non-constant bound */
    for (int i = 0; i < n; ++i) {
        /* Mixed address calculation to create memory dependencies */
        int idx = i + (i % 3);  /* Non-affine index */
        if (idx < n) {
            /* Multiple independent chains for potential parallelism */
            float t1 = a[idx] * 2.0f;
            float t2 = b[i] + 1.0f;
            float t3 = t1 + t2;  /* Register dependency chain */
            
            /* Store result creating memory dependency */
            a[idx] = t3;
        }
        
        /* Process with inner logic */
        process_inner(i, a, b, x, y, &sum, &tmp_reg);
        
        /* Nested loop with dependent bounds */
        for (int j = i; j < n && j < i + 5; ++j) {
            /* Cross-iteration memory dependency */
            x[j] += y[j-1 > 0 ? j-1 : 0] * 0.3f;
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] += sum;
}

/* Alternate computation with different pattern */
void compute2(int n, float *restrict a, float *restrict b) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Assert no loop-carried dependencies (but compiler must verify) */
        float val = a[i];
        
        /* Multiple accumulators for register pressure */
        acc1 += val * 1.1f;
        acc2 += val * 2.2f;
        
        /* Memory flow with stride */
        b[i] = acc1 + acc2;
        
        /* Conditional update creating control dependency */
        if (acc1 > acc2) {
            a[i] = b[i] * 0.5f;
        }
    }
    
    /* Cross-iteration through reduction variable */
    a[0] = acc1 + acc2;
}

int main() {
    const int sizes[] = {100, 200, 500};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    float total_checksum = 0.0f;
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate with alignment hint */
        float *a = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *b = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *x = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        float *y = (float*)__builtin_assume_aligned(malloc(n * sizeof(float)), 16);
        
        /* Initialize with pattern */
        for (int i = 0; i < n; ++i) {
            a[i] = (float)(i % 10);
            b[i] = (float)(i % 7);
            x[i] = (float)(i % 5);
            y[i] = (float)(i % 3);
        }
        
        /* Call compute functions multiple times */
        compute(n, a, b, x, y);
        compute2(n, a, b);
        
        /* Calculate checksum to prevent elimination */
        for (int i = 0; i < n && i < 10; ++i) {
            total_checksum += a[i] + b[i] + x[i] + y[i];
        }
        
        free(a); free(b); free(x); free(y);
    }
    
    printf("Checksum: %f\n", total_checksum);
    return 0;
}

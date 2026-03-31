/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* Alternative: -O3 -funroll-loops -fno-peel-loops */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_inner(int i, float *restrict a, float *restrict b, 
                   float *restrict c, float *restrict d, 
                   float *restrict sum, float *restrict tmp_swap) {
    /* Multiple independent operations to create parallel opportunities */
    float t1 = a[i] * 3.14f;      /* Memory read, register operation */
    float t2 = b[i] + 2.71f;      /* Another independent chain */
    
    /* Flow dependency on previous iteration's t3 via *sum */
    float t3 = t1 + t2 + *sum;    /* Register flow edge */
    
    /* Memory flow edge with one-element lag (cross-iteration) */
    if (i > 0) {
        d[i] = c[i] + c[i-1];     /* True dependency on c[i-1] */
    } else {
        d[i] = c[i];
    }
    
    /* Anti-dependency and output dependency through swap */
    float old_val = d[i];          /* Read d[i] */
    d[i] = *tmp_swap;              /* Write d[i] - output dependency */
    *tmp_swap = old_val;           /* Write tmp_swap - anti-dependency */
    
    /* Condition edge - depends on computed value */
    if (t3 > 100.0f) {
        a[i] = t3 * 0.5f;         /* Memory write with condition */
    }
    
    /* Update accumulator for next iteration */
    *sum = t3 * 0.9f;             /* Register flow to next iteration */
}

/* Main computation function with loop nest */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d) {
    float sum = 0.0f;
    float tmp_swap = 0.0f;
    
    /* Outer loop with parameter bound */
    for (int i = 0; i < n; ++i) {
        /* Non-linear index calculation to complicate analysis */
        int idx = i + (i % 3) - 1;
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        /* Mixed memory accesses with non-affine pattern */
        float val = a[idx] + b[i % 2 ? idx : i];
        
        /* Inner loop with dependent bound - creates complex DDG */
        #pragma GCC ivdep  /* Assert no loop-carried memory deps for inner */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Multiple dependency types in inner loop */
            c[j] += val * (float)j;  /* Flow in c[j] across inner iterations */
            
            /* Anti-dependency: read then write same location */
            float temp = d[j];
            d[j] = c[j] * 0.5f;
            c[j] = temp;  /* Output dependency on c[j] */
        }
        
        /* Call inline function with core operations */
        process_inner(i, a, b, c, d, &sum, &tmp_swap);
        
        /* Additional reduction with cross-iteration dependency */
        if (i > 0) {
            b[i] += a[i-1] * 0.3f;  /* Flow from a[i-1] */
        }
    }
}

/* Secondary function with different loop pattern */
void compute2(int m, int n, float *restrict x, float *restrict y) {
    /* Nested loops with parameter bounds */
    for (int i = 0; i < m; ++i) {
        float acc = x[i];
        
        /* Inner reduction loop */
        for (int j = 0; j < n; ++j) {
            /* Multiple dependencies in reduction */
            acc += y[j] * (float)(i + j);
            
            /* Cross-iteration anti-dependency */
            y[j] = y[j] + 1.0f;  /* Read y[j], then write - anti-dependency */
            
            /* Condition based on computed value */
            if (acc > (float)(i * 100)) {
                x[i] = acc * 0.25f;  /* Memory write with condition edge */
            }
        }
        
        /* Output dependency on x[i] */
        x[i] = acc;  /* Overwrites conditional write above */
    }
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    
    /* Allocate with dynamic size to prevent compile-time optimization */
    float *a = (float*)malloc(size1 * sizeof(float));
    float *b = (float*)malloc(size1 * sizeof(float));
    float *c = (float*)malloc(size1 * sizeof(float));
    float *d = (float*)malloc(size1 * sizeof(float));
    float *x = (float*)malloc(size2 * sizeof(float));
    float *y = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with non-constant pattern */
    for (int i = 0; i < size1; ++i) {
        a[i] = (float)(i % 10) * 1.5f;
        b[i] = (float)(i % 7) * 2.0f;
        c[i] = (float)(i % 5) * 3.0f;
        d[i] = (float)(i % 3) * 4.0f;
    }
    
    for (int i = 0; i < size2; ++i) {
        x[i] = (float)(i % 8) * 1.2f;
        y[i] = (float)(i % 6) * 1.8f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d);
    compute(size1 - 100, a + 100, b + 100, c + 100, d + 100);
    compute2(size2 / 2, size2 / 4, x, y);
    compute2(size2 / 3, size2 / 2, x + 100, y + 100);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (int i = 0; i < size2; ++i) {
        checksum += x[i] + y[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(x);
    free(y);
    
    return 0;
}

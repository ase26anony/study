/* Target: ddg.cc lines 749-757 in create_ddg_edge() */
/* Compile with: -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves */
/* Alternative: -O3 -funroll-loops -fno-peel-loops */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation with loop-carried dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;  /* Register for anti/output dependencies */
    
    /* Initialize with first element dependency */
    if (n > 0) {
        d[0] = a[0] + b[0];
    }
    
    /* 
     * Complex loop nest designed to trigger DDG edge creation:
     * 1. Reduction (register flow dependency)
     * 2. Array transform with lag (memory flow dependency)  
     * 3. Swap operation (register anti/output dependencies)
     * 4. Conditional updates (condition dependencies)
     * 5. Nested loop with varying bounds
     */
    
    #pragma GCC ivdep  /* Assert no loop-carried memory deps (forces analysis) */
    for (i = 1; i < n; ++i) {
        /* 1. REGISTER FLOW DEPENDENCY: reduction pattern */
        sum += a[i] * 0.75f;  /* e->data_type = reg, e->type = flow */
        
        /* 2. MEMORY FLOW DEPENDENCY: transform with one-element lag */
        b[i] = a[i] + a[i-1];  /* Cross-iteration memory dependency */
        
        /* 3. REGISTER ANTI/OUTPUT DEPENDENCIES: swap pattern */
        tmp_reg = c[i];        /* Anti-dependency on c[i] */
        c[i] = d[i-1];         /* Output dependency on c[i] */
        d[i-1] = tmp_reg;      /* Flow dependency on tmp_reg */
        
        /* 4. CONDITION DEPENDENCY: guarded computation */
        float threshold = sum * 0.01f;
        if (b[i] > threshold) {  /* Condition depends on computed values */
            /* 5. NESTED LOOP with variable bounds - prevents unrolling */
            for (j = i; j < n && j < i + 3; ++j) {
                /* Complex addressing creates memory dependencies */
                int idx = j + (i % 2);  /* Non-affine index */
                if (idx < n) {
                    a[idx] = conditional_update(a[idx], threshold);
                }
            }
        }
        
        /* Additional output dependency chain */
        d[i] = d[i-1] * 1.1f + b[i];  /* Another flow dependency */
    }
    
    /* Final reduction with mixed operations */
    for (i = n/2; i < n; ++i) {
        sum += c[i] - d[i];
    }
    
    *result = sum;
}

/* Secondary function with different dependency pattern */
void transform_array(int n, float* restrict x, float* restrict y) {
    float acc = x[0];
    
    #pragma GCC ivdep
    for (int i = 1; i < n; ++i) {
        /* Multiple independent chains with dependencies */
        float t1 = x[i] * 1.5f;
        float t2 = y[i-1] + 2.0f;
        float t3 = t1 - t2;
        
        /* Cross-iteration dependency on acc */
        acc = acc + t3;
        y[i] = acc;
        
        /* Pointer arithmetic for complex memory analysis */
        float* ptr = &x[i];
        *(ptr + (i%4 - 2)) = t3;  /* May create memory dependencies */
    }
}

int main() {
    const int max_size = 1000;
    float* a = (float*)malloc(max_size * sizeof(float));
    float* b = (float*)malloc(max_size * sizeof(float));
    float* c = (float*)malloc(max_size * sizeof(float));
    float* d = (float*)malloc(max_size * sizeof(float));
    
    /* Initialize with pattern to avoid simple analysis */
    for (int i = 0; i < max_size; ++i) {
        a[i] = (i % 10) * 1.5f;
        b[i] = (i % 7) * 0.8f;
        c[i] = (i % 5) * 2.1f;
        d[i] = (i % 3) * 1.2f;
    }
    
    float result1, result2, result3;
    
    /* Call with different sizes to trigger various DDG constructions */
    compute(500, a, b, c, d, &result1);
    compute(750, a, b, c, d, &result2);
    
    /* Transform arrays to create different dependency patterns */
    transform_array(300, a, b);
    transform_array(600, c, d);
    
    compute(900, a, b, c, d, &result3);
    
    /* Prevent dead code elimination */
    float checksum = result1 + result2 + result3 + a[0] + b[0] + c[0] + d[0];
    printf("Checksum: %f\n", checksum);
    
    /* Additional loop with different characteristics */
    for (int iter = 0; iter < 10; ++iter) {
        float temp = 0.0f;
        for (int i = 0; i < 100; ++i) {
            /* Mix of dependencies in small loop */
            temp = temp * 0.9f + a[i] * b[i];
            c[i] = c[i] + temp;
            if (i % 3 == 0) {
                d[i] = d[i] * 0.5f;
            }
        }
        checksum += temp;
    }
    
    printf("Final checksum: %f\n", checksum);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}

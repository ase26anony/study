/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg_coverage
 * Or with: gcc -O3 -funroll-loops -fno-peel-loops test_ddg_coverage.c -o test_ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1000

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float compute_element(float a, float b, float c, int cond) {
    /* Mix of operations to create register dependencies */
    float t1 = a * 1.5f;
    float t2 = b + 2.3f;
    float t3 = t1 - t2;
    return cond ? t3 * c : t3 / c;
}

/* Main computation function with loop carrying various dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* restrict e) {
    int i, j;
    float acc = 0.0f;          /* Register accumulator - flow dependency */
    float tmp_reg = 0.0f;      /* Temporary register - anti/output dependencies */
    
    /* Outer loop with parameterized bound */
    for (i = 1; i < n; ++i) {
        /* 1. FLOW DEPENDENCY (Register): Reduction pattern */
        acc += a[i] * 1.7f;
        
        /* 2. FLOW DEPENDENCY (Memory): Array copy with one-element shift */
        b[i] = a[i] + a[i-1];  /* Uses a[i-1] from previous iteration */
        
        /* 3. ANTI and OUTPUT DEPENDENCIES: Swap-like operation */
        tmp_reg = c[i];        /* ANTI: read before write in next stmt */
        c[i] = d[i];           /* OUTPUT: write to c[i] */
        d[i] = tmp_reg;        /* FLOW: use of tmp_reg */
        
        /* 4. CONDITION DEPENDENCY: Conditional update based on computed value */
        float cond_val = compute_element(a[i], b[i-1], (float)i, i % 3);
        if (cond_val > 0.5f) {
            e[i] = e[i-1] * 0.9f;  /* Memory flow dependency */
        } else {
            e[i] = e[i-1] * 1.1f;  /* Alternative path */
        }
        
        /* 5. NESTED LOOP with varying bound - increases complexity */
        #pragma GCC ivdep  /* Assert no loop-carried dependencies (may trigger DDG verification) */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Memory dependencies with non-affine access pattern */
            int idx = j + (i % 2);  /* Non-affine index calculation */
            if (idx < n) {
                a[idx] = a[idx] * 0.99f + b[j] * 0.01f;  /* Mixed memory dependencies */
            }
        }
        
        /* 6. OUTPUT DEPENDENCY on register */
        tmp_reg = acc * 0.5f;  /* Reuse of tmp_reg creates output dependency */
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = acc;
}

/* Secondary function with different loop structure */
void compute2(int n, float* restrict x, float* restrict y) {
    float sum1 = 0.0f, sum2 = 0.0f;
    int i;
    
    /* Loop with multiple independent chains and dependencies */
    for (i = 1; i < n; ++i) {
        /* Independent computation chains that converge */
        float chain1 = x[i] * 2.0f;
        float chain2 = y[i] + 3.0f;
        float chain3 = x[i-1] * y[i-1];  /* Memory flow dependency */
        
        /* Convergence point with register dependencies */
        sum1 = sum1 + chain1 + chain3;
        sum2 = sum2 + chain2 - chain3;
        
        /* Cross-assignment creating anti-dependencies */
        float old_x = x[i];     /* ANTI: read x[i] */
        x[i] = y[i] * sum1;     /* OUTPUT: write x[i] */
        y[i] = old_x * sum2;    /* FLOW: use old_x */
        
        /* Pointer arithmetic creating complex memory dependencies */
        float* ptr = &x[i];
        if (i % 4 == 0) {
            *(ptr + (i % 3)) = sum1;  /* Non-constant offset */
        }
    }
    
    /* Store results */
    x[0] = sum1 + sum2;
}

int main() {
    /* Initialize with different sizes to trigger various compilation paths */
    int sizes[] = {100, 500, 1000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    srand(time(NULL));
    
    for (int s = 0; s < num_sizes; ++s) {
        int n = sizes[s];
        
        /* Allocate and initialize arrays */
        float* a = (float*)malloc(n * sizeof(float));
        float* b = (float*)malloc(n * sizeof(float));
        float* c = (float*)malloc(n * sizeof(float));
        float* d = (float*)malloc(n * sizeof(float));
        float* e = (float*)malloc(n * sizeof(float));
        float* x = (float*)malloc(n * sizeof(float));
        float* y = (float*)malloc(n * sizeof(float));
        
        if (!a || !b || !c || !d || !e || !x || !y) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        
        /* Initialize with pattern (mix of random and sequential) */
        for (int i = 0; i < n; ++i) {
            a[i] = (float)(i % 100) * 0.1f;
            b[i] = (float)(rand() % 100) * 0.01f;
            c[i] = (float)i * 0.05f;
            d[i] = (float)(i * 2) * 0.025f;
            e[i] = (float)(i % 50) * 0.02f;
            x[i] = (float)(rand() % 100) * 0.015f;
            y[i] = (float)(i % 75) * 0.03f;
        }
        
        /* Perform computations - multiple calls to increase coverage chance */
        compute(n, a, b, c, d, e);
        compute2(n, x, y);
        
        /* Calculate checksum to prevent dead code elimination */
        float checksum = 0.0f;
        for (int i = 0; i < n && i < 10; ++i) {
            checksum += a[i] + b[i] + c[i] + d[i] + e[i] + x[i] + y[i];
        }
        
        printf("Size %d: checksum = %f\n", n, checksum);
        
        /* Free memory */
        free(a); free(b); free(c); free(d); free(e);
        free(x); free(y);
    }
    
    return 0;
}

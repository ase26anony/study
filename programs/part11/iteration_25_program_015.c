/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation function with loop-carried dependencies */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d, float* result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;
    
    /* Initialize with some values to create dependencies */
    b[0] = a[0] * 2.0f;
    c[0] = a[0] + 1.0f;
    
    /* 
     * Complex loop nest with multiple dependency types:
     * 1. Flow/true dependencies (RAW)
     * 2. Anti-dependencies (WAR)  
     * 3. Output dependencies (WAW)
     * 4. Memory and register dependencies
     */
    
    /* Outer loop - non-constant bound creates scheduling challenge */
    for (i = 1; i < n; ++i) {
        /* FLOW dependency on b[i-1] from previous iteration */
        b[i] = a[i] + b[i-1] * 0.7f;
        
        /* REGISTER flow dependency on sum (accumulator) */
        sum += a[i] * 1.5f;
        
        /* ANTI and OUTPUT dependencies through tmp_reg */
        tmp_reg = c[i];          /* Read c[i] */
        c[i] = d[i] * 0.3f;      /* Write c[i] - output dependency with previous read */
        d[i] = tmp_reg + 0.5f;   /* Write d[i] - anti-dependency with previous read of c[i] */
        
        /* Conditional creating control dependencies */
        #pragma GCC ivdep
        for (j = i; j < n && j < i + 3; ++j) {
            /* Memory dependency with non-affine index */
            int idx = i + (j % 2);
            if (idx < n) {
                /* Mixed memory/register dependencies */
                float temp = a[idx] * b[j];
                a[idx] = conditional_update(temp, sum);
                
                /* Another flow dependency chain */
                if (j > 0) {
                    d[j] = d[j-1] + temp * 0.1f;
                }
            }
        }
        
        /* Output dependency on scalar variable */
        tmp_reg = sum * 0.2f;
        
        /* Another anti-dependency pattern */
        float old_val = b[i];
        b[i] = tmp_reg + old_val * 0.8f;
        
        /* Complex addressing to create memory dependencies */
        if (i % 4 == 0) {
            int offset = (i / 2) % 3;
            c[i - offset] = a[i] * b[i] + d[i];
        }
    }
    
    /* Final reduction with flow dependency */
    for (i = 0; i < n; ++i) {
        sum += c[i] * 0.25f;
    }
    
    *result = sum;
}

/* Another function with different loop structure */
void compute2(int m, int n, float* restrict x, float* restrict y, float* res) {
    float acc1 = 0.0f, acc2 = 0.0f;
    int i, k;
    
    /* Nested loops with dependencies */
    for (i = 0; i < m; ++i) {
        /* Initialize with dependency on previous iteration */
        if (i > 0) {
            x[i] = y[i-1] * 1.1f;
        }
        
        /* Inner loop with carried dependency */
        for (k = 0; k < n; ++k) {
            /* Flow dependency on acc1 across inner iterations */
            acc1 += x[i] * k;
            
            /* Anti-dependency pattern */
            float temp = y[k];
            y[k] = acc1 * 0.3f;
            acc2 += temp;
            
            /* Conditional with register dependency */
            if (acc1 > acc2) {
                x[i] = x[i] * 0.9f + acc2;
            }
        }
        
        /* Cross-iteration dependency */
        y[i] = acc1 - acc2;
    }
    
    *res = acc1 + acc2;
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    float *a, *b, *c, *d, *x, *y;
    float result1, result2;
    int i;
    
    /* Seed RNG for reproducibility */
    srand(42);
    
    /* Allocate and initialize arrays */
    a = (float*)malloc(size1 * sizeof(float));
    b = (float*)malloc(size1 * sizeof(float));
    c = (float*)malloc(size1 * sizeof(float));
    d = (float*)malloc(size1 * sizeof(float));
    x = (float*)malloc(size2 * sizeof(float));
    y = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with pattern (not just random to create predictable dependencies) */
    for (i = 0; i < size1; ++i) {
        a[i] = (float)i * 0.1f;
        b[i] = (float)(i % 10) * 0.2f;
        c[i] = (float)(i % 5) * 0.3f;
        d[i] = (float)(i % 7) * 0.4f;
    }
    
    for (i = 0; i < size2; ++i) {
        x[i] = (float)i * 0.05f;
        y[i] = (float)(i % 3) * 0.15f;
    }
    
    /* Call compute functions multiple times with different sizes
     * to increase chance of DDG construction */
    compute(size1, a, b, c, d, &result1);
    compute(size1 / 2, a, b, c, d, &result1);
    
    compute2(size2 / 4, 10, x, y, &result2);
    compute2(size2 / 2, 5, x, y, &result2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f, %f\n", result1, result2);
    printf("Checksum: %f\n", result1 + result2);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}

/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to increase DDG construction scope */
static inline __attribute__((always_inline)) 
float conditional_update(float val, float threshold) {
    return (val > threshold) ? val * 0.5f : val * 2.0f;
}

/* Main computation function with loop-carried dependencies */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict c, float *restrict d, float *result) {
    int i, j;
    float sum = 0.0f;
    float tmp_reg;  /* Register for anti/output dependencies */
    
    /* Reduction with flow dependency on sum (register flow edge) */
    for (i = 0; i < n; ++i) {
        sum += a[i] * 1.5f;
    }
    
    /* Array transformation with one-element lag (memory flow edge) */
    /* Use pragma to influence dependency analysis */
    #pragma GCC ivdep
    for (i = 1; i < n; ++i) {
        b[i] = a[i] + a[i-1] * 0.7f;  /* Flow dependency on a[i-1] */
    }
    b[0] = a[0];  /* Handle boundary */
    
    /* Nested loop with dependent bounds for complexity */
    for (i = 0; i < n; ++i) {
        /* Inner loop bound depends on outer index */
        for (j = i; j < n && j < i + 5; ++j) {
            /* Mixed memory accesses with non-affine index */
            int idx = j + (i % 2);  /* Non-affine pattern */
            if (idx < n) {
                c[idx] = b[j] * c[i];  /* Multiple memory dependencies */
            }
        }
    }
    
    /* Swap operations creating anti and output dependencies */
    for (i = 0; i < n - 1; i += 2) {
        /* Register anti-dependency via tmp_reg */
        tmp_reg = d[i];
        d[i] = d[i + 1];      /* Output dependency on d[i] */
        d[i + 1] = tmp_reg;   /* True dependency on tmp_reg */
        
        /* Conditional update creating condition edges */
        if (tmp_reg > 0.0f) {  /* Condition depends on computed value */
            d[i] = conditional_update(d[i], 0.5f);
        }
    }
    
    /* Complex loop with all dependency types */
    float acc1 = 0.0f, acc2 = 0.0f;
    for (i = 0; i < n; ++i) {
        /* Independent arithmetic chains for potential parallelism */
        float x = a[i] * 2.0f;      /* Memory read */
        float y = b[i] + 1.0f;      /* Another memory read */
        float z = x + y;            /* Register dependency on x,y */
        
        /* Conditional with data-dependent branch */
        if (z > acc1) {            /* Condition edge */
            acc1 = z;               /* Register output dependency */
            c[i] = z * 0.3f;        /* Memory write */
        } else {
            acc2 += z;              /* Register flow dependency */
            /* Non-linear access pattern */
            int alt_idx = (i * 7) % n;
            if (alt_idx >= 0 && alt_idx < n) {
                d[alt_idx] = z * 0.7f;  /* Memory write with complex index */
            }
        }
        
        /* Cross-iteration dependency via accumulator */
        sum = sum * 0.99f + z * 0.01f;  /* Strong loop-carried dependency */
    }
    
    *result = sum + acc1 + acc2;
}

/* Second computation with different pattern */
void compute2(int m, int n, float *restrict arr1, float *restrict arr2) {
    int i, j;
    
    /* 2D loop nest for more complex DDG */
    for (i = 0; i < m; ++i) {
        float row_sum = 0.0f;
        for (j = 0; j < n; ++j) {
            int idx = i * n + j;
            row_sum += arr1[idx] * arr2[j];
            
            /* Pointer arithmetic creating memory dependencies */
            float *ptr = &arr1[idx];
            *ptr = *ptr * 0.8f + row_sum * 0.2f;
        }
        
        /* Loop-carried dependency through row_sum */
        arr2[i % n] = row_sum / n;
    }
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    const int m = 50, n = 20;
    
    /* Allocate and initialize arrays */
    float *a = (float*)malloc(size1 * sizeof(float));
    float *b = (float*)malloc(size1 * sizeof(float));
    float *c = (float*)malloc(size1 * sizeof(float));
    float *d = (float*)malloc(size1 * sizeof(float));
    float *arr1 = (float*)malloc(m * n * sizeof(float));
    float *arr2 = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < size1; ++i) {
        a[i] = (i % 37) * 0.1f;
        b[i] = (i % 23) * 0.2f;
        c[i] = (i % 47) * 0.3f;
        d[i] = (i % 31) * 0.4f - 0.5f;  /* Some negative values */
    }
    
    for (int i = 0; i < m * n; ++i) {
        arr1[i] = (i % 17) * 0.05f;
    }
    for (int i = 0; i < n; ++i) {
        arr2[i] = (i % 13) * 0.07f;
    }
    
    float result1, result2;
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d, &result1);
    compute(size2, a, b, c, d, &result2);
    compute2(m, n, arr1, arr2);
    
    /* Use results to prevent dead code elimination */
    float checksum = result1 + result2;
    for (int i = 0; i < n; ++i) {
        checksum += arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}

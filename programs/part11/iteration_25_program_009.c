/* Target: ddg.cc lines 749-757 - create_ddg_edge() field assignments */
#include <stdio.h>
#include <stdlib.h>

/* Helper with always_inline to ensure loop body is visible */
static inline __attribute__((always_inline)) 
void process_inner(int i, float *restrict a, float *restrict b, 
                   float *restrict x, float *restrict y, 
                   float *sum, float *tmp_var) {
    /* Register flow dependency - reduction */
    *sum += a[i] * 1.5f;
    
    /* Memory flow dependency with one-element lag */
    if (i > 0) {
        b[i] = a[i] + a[i-1] * 0.7f;
    } else {
        b[i] = a[i];
    }
    
    /* Anti and output dependencies via swap */
    float local_tmp = x[i];
    x[i] = y[i];
    y[i] = local_tmp;
    
    /* Register output dependency */
    *tmp_var = local_tmp * 2.0f;
}

/* Main computation with loop-carried dependencies */
void compute(int n, float *restrict a, float *restrict b, 
             float *restrict x, float *restrict y) {
    float sum = 0.0f;
    float tmp_var = 0.0f;
    
    /* Outer loop with parameter bound */
    #pragma GCC ivdep
    for (int i = 0; i < n; ++i) {
        /* Mixed address calculation - not purely affine */
        int idx = i + (i % 3);
        if (idx < n) {
            /* Memory dependency with non-linear index */
            float val = a[idx] * b[i % n];
            
            /* Condition edge - depends on computed value */
            if (val > 0.5f) {
                x[i] = val * 0.8f;
            } else {
                x[i] = val * 1.2f;
            }
        }
        
        /* Process inner operations */
        process_inner(i, a, b, x, y, &sum, &tmp_var);
        
        /* Nested loop with dependent bound - creates complex DDG */
        for (int j = i; j < n && j < i + 5; ++j) {
            /* Cross-iteration memory dependency */
            y[j] += x[i] * 0.3f;
            
            /* Register anti-dependency */
            float old = tmp_var;
            tmp_var = y[j] * 1.1f;
            x[i] += old;  /* Use old value */
        }
    }
    
    /* Use results to prevent elimination */
    a[0] = sum + tmp_var;
}

/* Secondary computation with different pattern */
void compute2(int m, int n, float *restrict arr1, float *restrict arr2) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    /* Loop with multiple independent chains */
    for (int i = 1; i < m; ++i) {
        /* Chain 1: Flow dependencies */
        float t1 = arr1[i] * 1.7f;
        float t2 = arr2[i-1] + 2.3f;
        float t3 = t1 + t2;
        
        /* Chain 2: Independent but with output dep */
        float u1 = arr2[i] * 0.9f;
        arr1[i] = u1 + t3;  /* Merges both chains */
        
        /* Accumulators with register flow deps */
        acc1 = acc1 + t3;
        acc2 = acc2 - u1;
        
        /* Memory anti-dependency */
        float temp = arr1[i-1];
        arr1[i-1] = arr2[i] * 1.4f;
        arr2[i] = temp + 0.5f;
    }
    
    /* Prevent dead code elimination */
    arr1[0] = acc1;
    arr2[0] = acc2;
}

int main(void) {
    const int size1 = 1000;
    const int size2 = 500;
    
    /* Allocate and initialize arrays */
    float *a = (float*)malloc(size1 * sizeof(float));
    float *b = (float*)malloc(size1 * sizeof(float));
    float *x = (float*)malloc(size1 * sizeof(float));
    float *y = (float*)malloc(size1 * sizeof(float));
    float *arr1 = (float*)malloc(size2 * sizeof(float));
    float *arr2 = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < size1; ++i) {
        a[i] = (i % 7) * 0.1f;
        b[i] = (i % 5) * 0.2f;
        x[i] = (i % 3) * 0.3f;
        y[i] = (i % 11) * 0.4f;
    }
    
    for (int i = 0; i < size2; ++i) {
        arr1[i] = (i % 13) * 0.05f;
        arr2[i] = (i % 17) * 0.06f;
    }
    
    /* Call compute multiple times with different sizes
       to increase chance of DDG construction */
    compute(size1, a, b, x, y);
    compute(size1/2, a, b, x, y);
    compute(size1/4, a, b, x, y);
    
    compute2(size2, size2, arr1, arr2);
    compute2(size2/2, size2, arr1, arr2);
    compute2(size2, size2/2, arr1, arr2);
    
    /* Calculate checksum to prevent elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + x[i] + y[i];
    }
    for (int i = 0; i < size2; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(x); free(y);
    free(arr1); free(arr2);
    
    return 0;
}

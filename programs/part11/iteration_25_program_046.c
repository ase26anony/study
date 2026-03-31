/* Target: ddg.cc lines 749-757 - create_ddg_edge() field assignments */
#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible */
static void __attribute__((always_inline)) 
process_iteration(float *restrict a, float *restrict b, float *restrict c, 
                   float *restrict d, int i, float *sum, float *tmp_reg) {
    /* Memory flow dependency: b[i] depends on a[i] and a[i-1] */
    b[i] = a[i] + a[i-1];
    
    /* Register flow dependency: sum accumulation */
    *sum += a[i] * 1.5f;
    
    /* Anti-dependency: read x[i] before writing it */
    *tmp_reg = c[i];
    c[i] = d[i] * 2.0f;
    d[i] = *tmp_reg;  /* Output dependency on d[i] */
    
    /* Condition edge: dependency between computation and branch */
    if (*sum > 100.0f) {
        c[i] *= 0.5f;  /* Creates condition edge */
    }
}

/* Main computation function with loop nest */
void __attribute__((noinline))
compute(int n, float *restrict a, float *restrict b, 
        float *restrict c, float *restrict d) {
    float sum = 0.0f;
    float tmp_reg = 0.0f;
    
    /* Initialize first element to avoid out-of-bounds */
    if (n > 0) {
        b[0] = a[0];
    }
    
    /* Try to hint no loop-carried dependencies for memory */
    #pragma GCC ivdep
    for (int i = 1; i < n; ++i) {
        /* Complex addressing to create memory dependencies */
        int idx1 = i + (i % 3);
        int idx2 = i - (i % 2);
        
        /* Memory anti-dependency with non-affine index */
        float temp = a[idx1 % n];
        a[idx1 % n] = b[idx2 % n] * 1.1f;
        b[idx2 % n] = temp;
        
        /* Call inlined function to create register dependencies */
        process_iteration(a, b, c, d, i, &sum, &tmp_reg);
        
        /* Nested loop with varying bound - creates complex dependency pattern */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Output dependency on c[j] */
            c[j] = c[j] + a[i] * b[j];
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = sum;
}

/* Secondary function with different loop structure */
void __attribute__((noinline))
compute2(int m, int n, float *restrict arr1, float *restrict arr2) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    /* Double nested loop with cross-iteration dependencies */
    for (int i = 0; i < m; ++i) {
        /* Reduction with register flow dependency */
        acc1 += arr1[i];
        
        for (int j = 0; j < n; ++j) {
            /* Memory flow dependency with 2D access pattern */
            int idx = (i * n + j) % (m * n);
            arr2[idx] = arr1[i] + arr2[(idx + 1) % (m * n)];
            
            /* Register anti-dependency */
            float old = arr2[idx];
            arr2[idx] = acc1 * 0.3f + arr2[idx];
            acc2 += old;
        }
        
        /* Conditional with data dependency */
        if (acc1 > acc2) {
            arr1[i] = acc2;
            acc2 = 0.0f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use non-constant sizes to prevent full unrolling */
    int size1 = (argc > 1) ? atoi(argv[1]) : 1000;
    int size2 = (argc > 2) ? atoi(argv[2]) : 500;
    
    if (size1 < 10) size1 = 1000;
    if (size2 < 10) size2 = 500;
    
    /* Allocate with dynamic sizes */
    float *a = (float*)malloc(size1 * sizeof(float));
    float *b = (float*)malloc(size1 * sizeof(float));
    float *c = (float*)malloc(size1 * sizeof(float));
    float *d = (float*)malloc(size1 * sizeof(float));
    float *arr1 = (float*)malloc(size1 * size2 * sizeof(float));
    float *arr2 = (float*)malloc(size1 * size2 * sizeof(float));
    
    /* Initialize with simple pattern */
    for (int i = 0; i < size1; ++i) {
        a[i] = i * 0.1f;
        b[i] = i * 0.2f;
        c[i] = i * 0.3f;
        d[i] = i * 0.4f;
    }
    
    for (int i = 0; i < size1 * size2; ++i) {
        arr1[i] = i * 0.01f;
        arr2[i] = i * 0.02f;
    }
    
    /* Call compute multiple times with different sizes */
    compute(size1, a, b, c, d);
    compute(size1 / 2, a, b, c, d);
    compute(size1 / 4, a, b, c, d);
    
    compute2(size1 / 10, size2 / 10, arr1, arr2);
    compute2(size1 / 20, size2 / 20, arr1, arr2);
    
    /* Calculate checksum to prevent elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    
    for (int i = 0; i < size1 * size2; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(arr1); free(arr2);
    
    return 0;
}

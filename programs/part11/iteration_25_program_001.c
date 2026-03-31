/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>

/* Helper function with always_inline to ensure loop body is visible to scheduler */
static inline __attribute__((always_inline)) 
void process_element(float* restrict a, float* restrict b, float* restrict c,
                     float* restrict d, int i, int prev, float* sum, 
                     float* tmp_reg, int* cond_flag) {
    /* Multiple independent arithmetic chains for potential parallelism */
    float x = a[i] * 3.14f;
    float y = b[i] + 2.71f;
    
    /* Flow dependency on previous iteration's value */
    *sum = *sum + x + y;
    
    /* Memory flow edge with one-element lag */
    if (i > 0) {
        c[i] = a[i] + a[prev];  /* Flow dependency on a[prev] from previous iteration */
    } else {
        c[i] = a[i];
    }
    
    /* Anti-dependency and output dependency through swap-like operation */
    *tmp_reg = d[i];
    d[i] = d[i] * 0.5f;  /* Output dependency on d[i] */
    float old_val = *tmp_reg;  /* Anti-dependency through tmp_reg */
    
    /* Condition edge - depends on computed values */
    *cond_flag = (x > y) ? 1 : 0;
    if (*cond_flag) {
        b[i] = b[i] * 0.9f;  /* Conditional update creates control dependency */
    }
}

/* Main computation function with loop nest */
void compute(int n, float* restrict a, float* restrict b, 
             float* restrict c, float* restrict d) {
    float sum = 0.0f;
    float tmp_reg = 0.0f;
    int cond_flag = 0;
    
    /* Outer loop with non-constant bound */
    #pragma GCC ivdep  /* Assert no loop-carried memory dependencies (for testing) */
    for (int i = 0; i < n; ++i) {
        int prev = (i > 0) ? i - 1 : 0;
        
        /* Process element with mixed dependencies */
        process_element(a, b, c, d, i, prev, &sum, &tmp_reg, &cond_flag);
        
        /* Additional operations to create more edges */
        /* Register flow dependency chain */
        float chain1 = sum * 0.1f;
        float chain2 = chain1 + tmp_reg;
        tmp_reg = chain2 * 0.5f;  /* Output dependency on tmp_reg */
        
        /* Memory anti-dependency: read after write */
        float read_after_write = b[i];  /* Anti-dependency on b[i] written in process_element */
        
        /* Nested loop-like pattern with dependency on outer index */
        for (int j = i; j < n && j < i + 3; ++j) {
            /* Complex index to create memory dependencies */
            int idx = i + (j % 2);
            if (idx < n) {
                a[idx] = a[idx] + read_after_write * 0.01f;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    a[0] = sum;
}

/* Second function with different loop structure */
void compute2(int m, int n, float* restrict arr1, float* restrict arr2) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    /* Nested loops with cross-iteration dependencies */
    for (int i = 1; i < m; ++i) {
        /* Flow dependency on acc1 from previous iteration */
        acc1 = acc1 + arr1[i];
        
        for (int j = 0; j < n; ++j) {
            /* Multiple dependencies in inner loop */
            float tmp = arr2[j];
            
            /* Flow dependency within inner loop */
            acc2 = acc2 + tmp;
            
            /* Anti-dependency */
            arr2[j] = arr1[i] * acc2;
            
            /* Output dependency */
            tmp = acc1 * 0.5f;
            
            /* Condition edge */
            if (acc2 > acc1) {
                arr1[i] = arr1[i] + tmp;
            }
        }
        
        /* Cross-iteration memory dependency */
        arr1[i] = arr1[i] + arr1[i-1];
    }
    
    /* Store results */
    arr1[0] = acc1 + acc2;
}

int main() {
    const int size1 = 1000;
    const int size2 = 500;
    
    /* Allocate and initialize arrays */
    float* a = (float*)malloc(size1 * sizeof(float));
    float* b = (float*)malloc(size1 * sizeof(float));
    float* c = (float*)malloc(size1 * sizeof(float));
    float* d = (float*)malloc(size1 * sizeof(float));
    float* arr1 = (float*)malloc(size2 * sizeof(float));
    float* arr2 = (float*)malloc(size2 * sizeof(float));
    
    /* Initialize with pattern (not all zeros) */
    for (int i = 0; i < size1; ++i) {
        a[i] = (float)(i % 100) * 0.1f;
        b[i] = (float)((i + 1) % 100) * 0.2f;
        c[i] = 0.0f;
        d[i] = (float)((i * 3) % 100) * 0.3f;
    }
    
    for (int i = 0; i < size2; ++i) {
        arr1[i] = (float)(i % 50) * 0.05f;
        arr2[i] = (float)((i * 2) % 50) * 0.1f;
    }
    
    /* Call compute functions multiple times with different sizes
     * to increase chance of DDG construction */
    compute(size1, a, b, c, d);
    compute(size1 / 2, a, b, c, d);
    compute(size1 / 4, a, b, c, d);
    
    compute2(size2 / 2, size2 / 4, arr1, arr2);
    compute2(size2 / 3, size2 / 5, arr1, arr2);
    
    /* Calculate checksum to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < size1; ++i) {
        checksum += a[i] + b[i] + c[i] + d[i];
    }
    for (int i = 0; i < size2; ++i) {
        checksum += arr1[i] + arr2[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(arr1);
    free(arr2);
    
    return 0;
}
